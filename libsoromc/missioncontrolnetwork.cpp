/*
 * Copyright 2016 The University of Oklahoma.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "missioncontrolnetwork.h"

#define MSG_NEGOTIATE 0
#define MSG_BROKER_STATE 1
#define MSG_REQUEST_ROLE 2
#define MSG_DENY_ROLE 3
#define MSG_ACCEPT_ROLE 4
#define MSG_REQUEST_CONNECTION 5

#define LOG_TAG "MissionControlNetwork"

#define NAME_LENGTH 32

namespace Soro {
namespace MissionControl {

MissionControlNetwork::MissionControlNetwork(QObject *parent) : QObject(parent) {
    _name = generateName();
}

MissionControlNetwork::~MissionControlNetwork() {
    clearConnections();
}

bool MissionControlNetwork::init(QString *error) {
    if (_broadcastSocket) {
        LOG_W(LOG_TAG, "init() called more than once");
        return true;
    }
    _broadcastSocket = new QUdpSocket(this);
    connect(_broadcastSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));

    if (!startNegotiation()) {
        if (error) *error = "Could not start mission control network negotiation. This is most likely because you are already running a mission control process on this computer.\n\nIf you are sure no other mission control is running, this could indicate a network issue or port conflict.";
        return false;
    }
    return true;
}

void MissionControlNetwork::clearConnections() {
    // Release all channel objects (not the broadcast socket)
    if (_clientChannel) {
        disconnect(_clientChannel, 0, this, 0);
        delete _clientChannel;
        _clientChannel = NULL;
    }
    foreach (Connection *connection, _brokerConnections) {
        disconnect(connection->channel, 0, this, 0);
        delete connection->channel;
    }
    _brokerConnections.clear();
}

bool MissionControlNetwork::startNegotiation() {
    LOG_I(LOG_TAG, "Beginning negotiation");

    KILL_TIMER(_broadcastStateTimerId);
    KILL_TIMER(_broadcastIntentTimerId);
    KILL_TIMER(_requestConnectionTimerId);
    KILL_TIMER(_requestRoleTimerId);

    clearConnections();
    // Set this to true until there is evidence this shouldn't be the broker
    _isBroker = true;
    _connected = false;
    // Role is claimed after negoation for boker, must be reset
    _role = SpectatorRole;
    // Re-bind the broadcast socket
    _broadcastSocket->abort();
    disconnect(_broadcastSocket, SIGNAL(readyRead()), this, 0);
    connect(_broadcastSocket, SIGNAL(readyRead()),
            this, SLOT(negotiation_broadcastSocketReadyRead()));
    if (!_broadcastSocket->bind(NETWORK_MC_BROADCAST_PORT)) {
        return false;
    }

    START_TIMER(_broadcastIntentTimerId, 500);
    QTimer::singleShot(3000, this, SLOT(endNegotiation()));

    return true;
}

void MissionControlNetwork::endNegotiation() {
    LOG_I(LOG_TAG, "Ending negoation, we " + QString(_isBroker ? " are the broker" : " are not the broker"));
    KILL_TIMER(_broadcastIntentTimerId);
    disconnect(_broadcastSocket, SIGNAL(readyRead()), this, 0);
    if (_isBroker) {
        START_TIMER(_broadcastStateTimerId, 500);
        _connected = true;
        connect(_broadcastSocket, SIGNAL(readyRead()),
                this, SLOT(broker_broadcastSocketReadyRead()));
        emit connected(true);
    }
    else {
        if (_clientChannel) {
            LOG_W(LOG_TAG, "Client channel was not null post-negotiation");
            clearConnections();
        }
        _clientChannel = Channel::createServer(this, NETWORK_MC_BROADCAST_PORT, _name, Channel::TcpProtocol);
        connect(_clientChannel, SIGNAL(messageReceived(const char*,Channel::MessageSize)),
                this, SLOT(client_channelMessageReceived(const char*,Channel::MessageSize)));
        connect(_clientChannel, SIGNAL(stateChanged(Channel::State)),
                this, SLOT(client_channelStateChanged(Channel::State)));
        connect(_broadcastSocket, SIGNAL(readyRead()),
                this, SLOT(client_broadcastSocketReadyRead()));
        _clientChannel->open();
        START_TIMER(_requestConnectionTimerId, 100);
        QTimer::singleShot(3000, this, SLOT(ensureConnection()));
    }
}

void MissionControlNetwork::socketError(QAbstractSocket::SocketError err) {
    Q_UNUSED(err);
    LOG_E(LOG_TAG, "Socket error: " + _broadcastSocket->errorString());
    startNegotiation();
}

void MissionControlNetwork::client_channelStateChanged(Channel::State state) {
    if ((state != Channel::ConnectedState) && _clientChannel->wasConnected()) {
        LOG_E(LOG_TAG, "Lost connection to broker");
        startNegotiation();
        emit disconnected();
    }
    else if (state == Channel::ConnectedState) {
        LOG_I(LOG_TAG, "Connected to broker at " + _clientChannel->getPeerAddress().toString());
        KILL_TIMER(_requestConnectionTimerId);
        _connected = true;
        emit connected(false);
    }
}

void MissionControlNetwork::ensureConnection() {
    if (_clientChannel
            && (_clientChannel->getState() != Channel::ConnectedState)
            && !_clientChannel->wasConnected()) {
        // Never connected to the broker, possible it was shut down right
        // after negotiation ended
        LOG_E(LOG_TAG, "Unable to connect to broker in time");
        startNegotiation();
    }
}

void MissionControlNetwork::requestRole(Role role) {
    if (!_connected) {
        LOG_W(LOG_TAG, "Got role request before connected to network");
        emit roleDenied(role);
    }
    else if (_isBroker) {
        if (role == SpectatorRole) {
            _role = role;
            emit roleGranted(role);
        }
        else {
            // Check if a client has already claimed this role
            foreach (Connection *connection, _brokerConnections) {
                if (connection->role == role) {
                    // There's already a client with this role
                    emit roleDenied(role);
                    return;
                }
            }
            // No other client with this role
            _role = role;
            emit roleGranted(role);
        }
    }
    else {
        _pendingRole = role;
        START_TIMER(_requestRoleTimerId, 500);
    }
}

void MissionControlNetwork::sendSharedMessage(const char *message, Channel::MessageSize size) {
    if (_clientChannel) {
        _clientChannel->sendMessage(message, size);
    }
    else {
        foreach (Connection *connection, _brokerConnections) {
            if (connection->channel) {
                connection->channel->sendMessage(message, size);
            }
        }
    }
}

void MissionControlNetwork::timerEvent(QTimerEvent *e) {
    if (e->timerId() == _broadcastIntentTimerId) {
        LOG_I(LOG_TAG, "Sending MSG_NEGOTIATE");
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream << (quint8)MSG_NEGOTIATE;
        stream << _name;

        _broadcastSocket->writeDatagram(message, QHostAddress::Broadcast, NETWORK_MC_BROADCAST_PORT);
    }
    else if (e->timerId() == _broadcastStateTimerId) {
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream << (quint8)MSG_BROKER_STATE;
        stream << _name;
        stream << (quint16)_brokerConnections.size() + 1;

        _broadcastSocket->writeDatagram(message, QHostAddress::Broadcast, NETWORK_MC_BROADCAST_PORT);
        emit statisticsUpdate(_brokerConnections.size() + 1);
    }
    else if (e->timerId() == _requestConnectionTimerId) {
        LOG_I(LOG_TAG, "Sending MSG_REQUEST_CONNECTION");
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream << (quint8)MSG_REQUEST_CONNECTION;
        stream << _name;

        _broadcastSocket->writeDatagram(message, QHostAddress::Broadcast, NETWORK_MC_BROADCAST_PORT);
    }
    else if (e->timerId() == _requestRoleTimerId) {
        LOG_I(LOG_TAG, "Sending MSG_REQUEST_ROLE");
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream << (quint8)MSG_REQUEST_ROLE;
        stream << _name;
        stream << reinterpret_cast<quint32&>(_pendingRole);

        _broadcastSocket->writeDatagram(message, QHostAddress::Broadcast, NETWORK_MC_BROADCAST_PORT);
    }
}

void MissionControlNetwork::negotiation_broadcastSocketReadyRead() {
    while (_broadcastSocket->hasPendingDatagrams()) {
        SocketAddress address;
        int len =_broadcastSocket->readDatagram(_buffer, 100, &address.host, &address.port);
        if (len < 0) continue;
        QDataStream stream(QByteArray::fromRawData(_buffer, len));

        quint8 header;
        QString requestName;
        stream >> header;
        stream >> requestName;

        if (_name.compare(requestName) == 0) return;

        switch (header) {
        case MSG_NEGOTIATE:
            LOG_I(LOG_TAG, "Receiving negotiation message from " + requestName);
            _isBroker = _name.compare(requestName) > 0;
            _brokerAddress = address;
            break;
        case MSG_BROKER_STATE:
            LOG_I(LOG_TAG, "Receiving broker status message");
            _isBroker = false;
            _brokerAddress = address;
            break;
        }
    }
}

void MissionControlNetwork::acceptClientRole(SocketAddress address, Role role, QString name) {
    LOG_I(LOG_TAG, "Accepting client role request");
    QByteArray response;
    QDataStream responseStream(&response, QIODevice::WriteOnly);
    responseStream << (quint8)MSG_ACCEPT_ROLE;
    responseStream << _name;
    responseStream << reinterpret_cast<quint32&>(role);
    _broadcastSocket->writeDatagram(response, address.host, address.port);
    // update client's role information
    foreach (Connection *connection, _brokerConnections) {
        if (connection->channel->getName().compare(name) == 0) {
            connection->role = role;
            return;
        }
    }
    // Didn't find the client in the list
    LOG_E(LOG_TAG, "Could not find client with name " + name + " in connection list for role change");
}

void MissionControlNetwork::denyClientRole(SocketAddress address, Role role) {
    LOG_I(LOG_TAG, "Denying client role request");
    QByteArray response;
    QDataStream responseStream(&response, QIODevice::WriteOnly);
    responseStream << (quint8)MSG_DENY_ROLE;
    responseStream << _name;
    responseStream << reinterpret_cast<quint32&>(role);
    _broadcastSocket->writeDatagram(response, address.host, address.port);
}

void MissionControlNetwork::broker_broadcastSocketReadyRead() {
    while (_broadcastSocket->hasPendingDatagrams()) {
        SocketAddress address;
        int len =_broadcastSocket->readDatagram(_buffer, 100, &address.host, &address.port);
        if (len < 0) continue;
        QDataStream stream(QByteArray::fromRawData(_buffer, len));

        quint8 header;
        QString requestName;
        stream >> header;
        stream >> requestName;

        if (_name.compare(requestName) == 0) return;

        switch (header) {
        case MSG_BROKER_STATE: {
            int compare = _name.compare(requestName);
            if (compare > 0) {
                LOG_E(LOG_TAG, "Detected another broker on the network, surrendering control to it");
                // There appears to be another broker on the network that should take precedence over us,
                // start renegotiating
                startNegotiation();
                emit disconnected();
            }
            break;
        }
        case MSG_REQUEST_ROLE:
            // Client is requesting to fill a role on the network
            Role requestRole;
            stream >> reinterpret_cast<quint32&>(requestRole);
            if (requestRole != SpectatorRole) {
                if (_role == requestRole) {
                    // Request denied
                    denyClientRole(address, requestRole);
                    return;
                }
                foreach (Connection *connection, _brokerConnections) {
                    if (connection->channel->getName().compare(requestName) == 0) continue;
                    if (connection->role == requestRole) {
                        // Request denied
                        denyClientRole(address, requestRole);
                        return;
                    }
                }
            }
            // Request accepted
            acceptClientRole(address, requestRole, requestName);
            break;
        case MSG_REQUEST_CONNECTION:
            // Ensure the client is not already in the list
            foreach (Connection *connection, _brokerConnections) {
                if (connection->channel->getName().compare(requestName) == 0) {
                    return;
                }
            }
            // Create a channel for the new client
            Connection *connection = new Connection;
            connection->channel = Channel::createClient(this, address, requestName, Channel::TcpProtocol);
            connect(connection->channel, &Channel::stateChanged, [=](Channel::State state) {
                broker_clientChannelStateChanged(connection->channel, state);
            });
            connect(connection->channel, &Channel::messageReceived, [=](const char* message, Channel::MessageSize size) {
                broker_clientChannelMessageReceived(connection->channel, message, size);
            });
            connection->channel->open();
            _brokerConnections.append(connection);
            break;
        }
    }
}

void MissionControlNetwork::client_broadcastSocketReadyRead() {
    while (_broadcastSocket->hasPendingDatagrams()) {
        SocketAddress address;
        int len =_broadcastSocket->readDatagram(_buffer, 100, &address.host, &address.port);
        if (len < 0) continue;
        QDataStream stream(QByteArray::fromRawData(_buffer, len));

        quint8 header;
        QString requestName;
        Role responseRole;
        stream >> header;
        stream >> requestName;

        if (_name.compare(requestName) == 0) return;

        switch (header) {
        case MSG_ACCEPT_ROLE:
            stream >> reinterpret_cast<quint32&>(responseRole);
            if ((_pendingRole != _role) && (responseRole == _pendingRole)) {
                KILL_TIMER(_requestRoleTimerId);
                // Role request accepted
                LOG_I(LOG_TAG, "Role request accepted");
                _role = _pendingRole;
                emit roleGranted(_role);
            }
            break;
        case MSG_DENY_ROLE:
            stream >> reinterpret_cast<quint32&>(responseRole);
            if ((_pendingRole != _role) && (responseRole == _pendingRole)) {
                KILL_TIMER(_requestRoleTimerId);
                // Role request denied
                LOG_I(LOG_TAG, "Role request denied");
                emit roleDenied(_pendingRole);
                _pendingRole = _role;
            }
            break;
        case MSG_BROKER_STATE:
            quint16 networkSize;
            stream >> networkSize;
            emit statisticsUpdate(networkSize);
            break;
        }
    }
}

void MissionControlNetwork::broker_clientChannelStateChanged(Channel *channel, Channel::State state) {
    if (state == Channel::ConnectedState) {
        emit newClientConnected(channel);
    }
    else if(channel->wasConnected()) {
        for (int i = 0; i < _brokerConnections.length(); ++i) {
            if (_brokerConnections[i]->channel == channel) {
                LOG_I(LOG_TAG, "Removing inactive client " + channel->getName() + " from list");
                disconnect(channel, 0, this, 0);
                delete _brokerConnections[i];
                _brokerConnections.removeAt(i);
                break;
            }
        }
    }
}

void MissionControlNetwork::broker_clientChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    foreach (Connection *connection, _brokerConnections) {
        if (connection->channel && (connection->channel != channel)) {
            connection->channel->sendMessage(message, size);
        }
    }
    emit sharedMessageReceived(message, size);
}

void MissionControlNetwork::client_channelMessageReceived(const char* message, Channel::MessageSize size) {
    emit sharedMessageReceived(message, size);
}

QString MissionControlNetwork::generateName() {
    const QString chars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_-+={}[];:`~<>,./?|");

    qsrand(QTime::currentTime().msec());
    QString randomString;
    for(int i = 0; i < NAME_LENGTH; ++i) {
        randomString.append(chars.at(qrand() % chars.length()));
    }
    return randomString;
}

bool MissionControlNetwork::isBroker() const {
    return _isBroker & _connected;
}

Role MissionControlNetwork::getRole() const {
    return _role;
}

QHostAddress MissionControlNetwork::getBrokerAddress() const {
    if (_connected && _clientChannel) {
        return _clientChannel->getPeerAddress().host;
    }
    return QHostAddress::Null;
}

}
}


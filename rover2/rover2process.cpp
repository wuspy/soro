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

#include "rover2process.h"

#define LOG_TAG "Rover2"

namespace Soro {
namespace Rover {

Rover2Process::Rover2Process(QObject *parent) : QObject(parent) {
    // Must initialize once the event loop has started.
    // This can be accomplished using a single shot timer.
    QTimer::singleShot(1, this, SLOT(init()));
}

void Rover2Process::init() {
    LOG_I(LOG_TAG, "*****************Loading Configuration*******************");

    QString error;
    if (!_config.load(&error)) {
        LOG_E(LOG_TAG, error);
        QCoreApplication::exit(1);
        return;
    }

    LOG_I(LOG_TAG, "*****************Initializing Video system*******************");


    _videoServers = new VideoServerArray(this);
    _videoServers->populate(_config.getBlacklistedCameras(),
                            NETWORK_ALL_CAMERA_PORT_1 + _config.getComputer1CameraCount(),
                            _config.getComputer1CameraCount());

    connect(_videoServers, &VideoServerArray::videoServerError, this, &Rover2Process::mediaServerError);

    if (_videoServers->serverCount() > _config.getComputer2CameraCount()) {
        LOG_E(LOG_TAG, "The configuration specifies less cameras than this, the last ones will be removed");
        while (_videoServers->serverCount() > _config.getComputer2CameraCount()) {
            _videoServers->remove(_videoServers->serverCount() - 1);
        }
    }
    else if (_videoServers->serverCount() < _config.getComputer2CameraCount()) {
        LOG_E(LOG_TAG, "The configuration specifies more cameras than this, check cable connections");
    }

    LOG_I(LOG_TAG, "*************Initializing core networking*****************");

    _masterComputerBroadcastSocket = new QUdpSocket(this);
    connect(_masterComputerBroadcastSocket, &QUdpSocket::readyRead, this, &Rover2Process::masterComputerBroadcastSocketReadyRead);

    beginBroadcast();
}

void Rover2Process::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if ((e->timerId() == _broadcastTimerId) && _masterComputerBroadcastSocket) {
        // broadcast to the master computer
        LOG_I(LOG_TAG, "Sending broadcast on subnet");
        _masterComputerBroadcastSocket->writeDatagram(SECONDARY_COMPUTER_BROADCAST_STRING, strlen(SECONDARY_COMPUTER_BROADCAST_STRING) + 1,
                                                      QHostAddress::Broadcast, NETWORK_ROVER_COMPUTER2_PORT);
    }
}

void Rover2Process::beginBroadcast() {
    LOG_I(LOG_TAG, "Beginning UDP broadcast on port " + QString::number(NETWORK_ROVER_COMPUTER2_PORT));
    if (_masterComputerBroadcastSocket) {
        _masterComputerBroadcastSocket->abort();
        _masterComputerBroadcastSocket->bind(QHostAddress::Any, NETWORK_ROVER_COMPUTER2_PORT);
        _masterComputerBroadcastSocket->open(QIODevice::ReadWrite);
    }
    START_TIMER(_broadcastTimerId, 1000);
}

void Rover2Process::masterComputerBroadcastSocketReadyRead() {
    char buffer[100];
    SocketAddress peer;
    while (_masterComputerBroadcastSocket->hasPendingDatagrams()) {
        int len = _masterComputerBroadcastSocket->readDatagram(&buffer[0], 100, &peer.host, &peer.port);
        if ((strncmp(MASTER_COMPUTER_BROADCAST_STRING, buffer, len) == 0)) {
            // master computer has responded
            LOG_I(LOG_TAG, "Got response from master computer on broadcast socket");

            // initialize the tcp channel
            if (!_masterComputerChannel) {
                _masterComputerChannel = Channel::createClient(this, peer, CHANNEL_NAME_SECONDARY_COMPUTER,
                                          Channel::TcpProtocol, QHostAddress::Any);

                if (_masterComputerChannel->getState() == Channel::ErrorState) {
                    LOG_E(LOG_TAG, "The master computer channel experienced a fatal error during initialization");
                    exit(1); return;
                }

                connect(_masterComputerChannel, &Channel::messageReceived, this, &Rover2Process::masterChannelMessageReceived);
                connect(_masterComputerChannel, &Channel::stateChanged, this, &Rover2Process::masterChannelStateChanged);

                _masterComputerChannel->open();
            }

            // stop broadcasting
            KILL_TIMER(_broadcastTimerId);
        }
    }
}

void Rover2Process::masterChannelStateChanged(Channel::State state) {
    if ((state != Channel::ConnectedState) && _masterComputerChannel->wasConnected()) {
        // lost connection to master computer, start broadcasting again
        LOG_E(LOG_TAG, "Lost connection to master computer");
        _masterComputerChannel->close();
        delete _masterComputerChannel;
        _masterComputerChannel = nullptr;

        START_TIMER(_broadcastTimerId, 1000);
    }
}

void Rover2Process::masterChannelMessageReceived(const char *message, Channel::MessageSize size) {
    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    SharedMessageType messageType;

    stream >> reinterpret_cast<quint32&>(messageType);
    switch (messageType) {
    case SharedMessage_RequestActivateCamera: {
        qint32 camera;
        QString formatString;
        VideoFormat format;
        stream >> camera;
        stream >> formatString;
        format.deserialize(formatString);
        if ((camera >= _config.getComputer1CameraCount()) && (camera < _config.getComputer1CameraCount() + _config.getComputer2CameraCount())) {
            LOG_I(LOG_TAG, "Camera " + QString::number(camera) + " is about to be activated");
            _videoServers->activate(camera, format);
        }
        else {
            LOG_E(LOG_TAG, "Received camera ID out of range (" + QString::number(camera));
        }
    }
        break;
    case SharedMessage_RequestDeactivateCamera:
        qint32 camera;
        stream >> camera;
        if ((camera >= _config.getComputer1CameraCount()) && (camera < _config.getComputer1CameraCount() + _config.getComputer2CameraCount())) {
            LOG_I(LOG_TAG, "Camera " + QString::number(camera) + " is about to be deactivated");
            _videoServers->deactivate(camera);
        }
        else {
            LOG_E(LOG_TAG, "Received camera ID out of range (" + QString::number(camera));
        }
        break;
    default:
        break;
    }
}

void Rover2Process::mediaServerError(MediaServer *server, QString message) {
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_RoverMediaServerError;
    stream.setByteOrder(QDataStream::BigEndian);

    stream << reinterpret_cast<quint32&>(messageType);
    stream << (qint32)server->getMediaId();
    stream << message;

    _masterComputerChannel->sendMessage(byteArray);
}

Rover2Process::~Rover2Process() {\

}

} // namespace Rover
} // namespace Soro

#include "rover2process.h"

#define LOG_TAG "Rover2"

namespace Soro {
namespace Rover {

Rover2Process::Rover2Process(const Configuration *config, QObject *parent) : QObject(parent) {
    _config = config;

    // Must initialize once the event loop has started.
    // This can be accomplished using a single shot timer.
    QTimer::singleShot(1, this, SLOT(init()));
}

void Rover2Process::init() {
    LOG_I(LOG_TAG, "Configuration has been loaded successfully");

    LOG_I(LOG_TAG, "*****************Initializing Video system*******************");

    _videoServers = new VideoServerArray(this);
    _videoServers->populate(_config->BlacklistedUsbCameras,
                            _config->FirstVideoPort + _config->MainComputerCameraCount,
                            _config->MainComputerCameraCount);

    if (_videoServers->serverCount() > _config->SecondaryComputerCameraCount) {
        LOG_E(LOG_TAG, "The configuration specifies less cameras than this, the last ones will be removed");
        while (_videoServers->serverCount() > _config->SecondaryComputerCameraCount) {
            _videoServers->remove(_videoServers->serverCount() - 1);
        }
    }
    else if (_videoServers->serverCount() < _config->SecondaryComputerCameraCount) {
        LOG_E(LOG_TAG, "The configuration specifies more cameras than this, check cable connections");
    }

    LOG_I(LOG_TAG, "*************Initializing core networking*****************");

    _masterComputerBroadcastSocket = new QUdpSocket(this);
    connect(_masterComputerBroadcastSocket, SIGNAL(readyRead()),
            this, SLOT(masterComputerBroadcastSocketReadyRead()));

    beginBroadcast();
}

void Rover2Process::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if ((e->timerId() == _broadcastTimerId) && _masterComputerBroadcastSocket) {
        // broadcast to the master computer
        LOG_I(LOG_TAG, "Sending broadcast on subnet");
        _masterComputerBroadcastSocket->writeDatagram(SECONDARY_COMPUTER_BROADCAST_STRING, strlen(SECONDARY_COMPUTER_BROADCAST_STRING) + 1,
                                                      QHostAddress::Broadcast, _config->SecondaryComputerPort);
    }
}

void Rover2Process::beginBroadcast() {
    LOG_I(LOG_TAG, "Beginning UDP broadcast on port " + QString::number(_config->SecondaryComputerPort));
    if (_masterComputerBroadcastSocket) {
        _masterComputerBroadcastSocket->abort();
        _masterComputerBroadcastSocket->bind(QHostAddress::Any, _config->SecondaryComputerPort);
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

                connect(_masterComputerChannel, SIGNAL(messageReceived(Channel*,const char*,Channel::MessageSize)),
                        this, SLOT(masterChannelMessageReceived(Channel*,const char*,Channel::MessageSize)));
                connect(_masterComputerChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
                        this, SLOT(masterChannelStateChanged(Channel*,Channel::State)));

                _masterComputerChannel->open();
            }

            // stop broadcasting
            KILL_TIMER(_broadcastTimerId);
        }
    }
}

void Rover2Process::masterChannelStateChanged(Channel *channel, Channel::State state) {
    if ((state != Channel::ConnectedState) && channel->wasConnected()) {
        // lost connection to master computer, start broadcasting again
        LOG_E(LOG_TAG, "Lost connection to master computer");
        _masterComputerChannel->close();
        delete _masterComputerChannel;
        _masterComputerChannel = NULL;

        START_TIMER(_broadcastTimerId, 1000);
    }
}

void Rover2Process::masterChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    SharedMessageType messageType;

    stream >> reinterpret_cast<quint32&>(messageType);
    switch (messageType) {
    case SharedMessage_RequestActivateCamera: {
        qint32 camera;
        VideoFormat format;
        stream >> camera;
        stream >> reinterpret_cast<quint32&>(format);
        if ((camera >= _config->MainComputerCameraCount) && (camera < _config->MainComputerCameraCount + _config->SecondaryComputerCameraCount)) {
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
        if ((camera >= _config->MainComputerCameraCount) && (camera < _config->MainComputerCameraCount + _config->SecondaryComputerCameraCount)) {
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

Rover2Process::~Rover2Process() {
    if (_masterComputerBroadcastSocket) delete _masterComputerBroadcastSocket;
    if (_masterComputerChannel) delete _masterComputerChannel;
}

} // namespace Rover
} // namespace Soro

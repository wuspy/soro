#include "rover2process.h"

#define LOG_TAG "Rover2"

namespace Soro {
namespace Rover {

Rover2Process::Rover2Process(QObject *parent) : QObject(parent) {
    _log = new Logger(this);
    _log->setLogfile(QCoreApplication::applicationDirPath() + "/rover2_" + QDateTime::currentDateTime().toString("M-dd_h:mm:AP") + ".log");
    _log->RouteToQtLogger = true;
    _log->MaxQtLoggerLevel = LOG_LEVEL_DEBUG;
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    //Must initialize from the event loop
    START_TIMER(_initTimerId, 1);
}

void Rover2Process::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _initTimerId) {
        KILL_TIMER(_initTimerId); //single shot

        LOG_I("**************Loading configuration from soro.ini****************");
        QString err = QString::null;
        if (!_config.load(&err)) {
            LOG_E(err);
            exit(1); return;
        }
        _config.applyLogLevel(_log);
        LOG_I("Configuration has been loaded successfully");

        LOG_I("*****************Initializing Video system*******************");

        _videoServers.populate(_config.BlacklistedUvdCameras, _config.FirstVideoPort + _config.MainComputerCameraCount);

        if (_videoServers.cameraCount() > _config.SecondaryComputerCameraCount) {
            LOG_E("The configuration specifies less cameras than this, the last ones will be removed");
            while (_videoServers.cameraCount() > _config.SecondaryComputerCameraCount) {
                _videoServers.remove(_videoServers.cameraCount() - 1);
            }
        }
        else if (_videoServers.cameraCount() < _config.SecondaryComputerCameraCount) {
            LOG_E("The configuration specifies more cameras than this, check cable connections");
        }

        LOG_I("*************Initializing core networking*****************");

        _masterComputerBroadcastSocket = new QUdpSocket(this);
        connect(_masterComputerBroadcastSocket, SIGNAL(readyRead()),
                this, SLOT(masterComputerBroadcastSocketReadyRead()));

        beginBroadcast();

        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("Initialization complete");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");

    }
    else if ((e->timerId() == _broadcastTimerId) && _masterComputerBroadcastSocket) {
        // broadcast to the master computer
        LOG_I("Sending broadcast on subnet");
        _masterComputerBroadcastSocket->writeDatagram(SECONDARY_COMPUTER_BROADCAST_STRING, strlen(SECONDARY_COMPUTER_BROADCAST_STRING) + 1,
                                                      QHostAddress::Broadcast, _config.SecondaryComputerPort);
    }
}

void Rover2Process::beginBroadcast() {
    LOG_I("Beginning UDP broadcast on port " + QString::number(_config.SecondaryComputerPort));
    if (_masterComputerBroadcastSocket) {
        _masterComputerBroadcastSocket->abort();
        _masterComputerBroadcastSocket->bind(QHostAddress::Any, _config.SecondaryComputerPort);
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
            LOG_I("Got response from master computer on broadcast socket");

            // initialize the tcp channel
            if (!_masterComputerChannel) {
                _masterComputerChannel = new Channel(this, peer, CHANNEL_NAME_SECONDARY_COMPUTER,
                                          Channel::TcpProtocol, QHostAddress::Any, _log);

                if (_masterComputerChannel->getState() == Channel::ErrorState) {
                    LOG_E("The master computer channel experienced a fatal error during initialization");
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
        LOG_E("Lost connection to master computer");
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
        StreamFormat format;
        stream >> camera;
        stream >> format;
        LOG_I("Camera " + QString::number(camera) + " is about to be activated");
        _videoServers.activate(camera, format);
    }
        break;
    case SharedMessage_RequestDeactivateCamera:
        qint32 camera;
        stream >> camera;
        LOG_I("Camera " + QString::number(camera) + " is about to be deactivated");
        _videoServers.deactivate(camera);
        break;
    default:
        break;
    }
}

Rover2Process::~Rover2Process() {
    if (_log) delete _log;
    if (_masterComputerBroadcastSocket) delete _masterComputerBroadcastSocket;
    if (_masterComputerChannel) delete _masterComputerChannel;
}

} // namespace Rover
} // namespace Soro

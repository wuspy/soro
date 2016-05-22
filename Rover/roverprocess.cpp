#include "roverprocess.h"

#define LOG_TAG "Rover"

namespace Soro {
namespace Rover {

RoverProcess::RoverProcess(QObject *parent) : QObject(parent) {
    _log = new Logger(this);
    _log->setLogfile(QCoreApplication::applicationDirPath() + "/rover_" + QDateTime::currentDateTime().toString("M-dd_h:mm:AP") + ".log");
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

void RoverProcess::timerEvent(QTimerEvent *e) {
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

        LOG_I("*************Initializing core networking*****************");

        _armChannel = new Channel(this, _config.ArmChannelPort, CHANNEL_NAME_ARM,
                                  Channel::UdpProtocol, QHostAddress::Any, _log);
        _driveChannel = new Channel(this, _config.DriveChannelPort, CHANNEL_NAME_DRIVE,
                                  Channel::UdpProtocol, QHostAddress::Any, _log);
        _gimbalChannel = new Channel(this, _config.GimbalChannelPort, CHANNEL_NAME_GIMBAL,
                                  Channel::UdpProtocol, QHostAddress::Any, _log);
        _sharedChannel = new Channel(this, _config.SharedChannelPort, CHANNEL_NAME_SHARED,
                                  Channel::TcpProtocol, QHostAddress::Any, _log);
        _secondaryComputerChannel = new Channel(this, _config.SecondaryComputerPort, CHANNEL_NAME_SECONDARY_COMPUTER,
                                        Channel::TcpProtocol, QHostAddress::Any, _log);

        if (_armChannel->getState() == Channel::ErrorState) {
            LOG_E("The arm channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_driveChannel->getState() == Channel::ErrorState) {
            LOG_E("The drive channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_gimbalChannel->getState() == Channel::ErrorState) {
            LOG_E("The gimbal channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_sharedChannel->getState() == Channel::ErrorState) {
            LOG_E("The shared channel experienced a fatal error during initialization");
            exit(1); return;
        }

        _armChannel->open();
        _driveChannel->open();
        _gimbalChannel->open();
        _sharedChannel->open();
        _secondaryComputerChannel->open();

        // create the udp broadcast socket that will listen for the secondary computer
        _secondaryComputerBroadcastSocket = new QUdpSocket(this);

        // observers for network channel connectivity changes
        connect(_sharedChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
                this, SLOT(sharedChannelStateChanged(Channel*,Channel::State)));
        connect(_secondaryComputerChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
                this, SLOT(secondaryComputerStateChanged(Channel*,Channel::State)));

        connect(_secondaryComputerBroadcastSocket, SIGNAL(readyRead()),
                this, SLOT(secondaryComputerBroadcastSocketReadyRead()));
        connect(_secondaryComputerBroadcastSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(secondaryComputerBroadcastSocketError(QAbstractSocket::SocketError)));


        beginSecondaryComputerListening();

        LOG_I("All network channels initialized successfully");

        LOG_I("*****************Initializing MBED systems*******************");

        // create mbed channels
        _armControllerMbed = new MbedChannel(SocketAddress(QHostAddress::Any, _config.ArmMbedPort), MBED_ID_ARM, this, _log);
        _driveGimbalControllerMbed = new MbedChannel(SocketAddress(QHostAddress::Any, _config.DriveCameraMbedPort), MBED_ID_DRIVE_CAMERA, this, _log);

        // observers for mbed connectivity changes
        connect(_armControllerMbed, SIGNAL(stateChanged(MbedChannel*,MbedChannel::State)),
                this, SLOT(mbedChannelStateChanged(MbedChannel*,MbedChannel::State)));
        connect(_driveGimbalControllerMbed, SIGNAL(stateChanged(MbedChannel*,MbedChannel::State)),
                this, SLOT(mbedChannelStateChanged(MbedChannel*,MbedChannel::State)));

        // observers for network channels message received
        connect(_armChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
                 this, SLOT(armChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));
        connect(_driveChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
                 this, SLOT(driveChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));
        connect(_gimbalChannel, SIGNAL(messageReceived(Channel*,const char*,Channel::MessageSize)),
                this, SLOT(gimbalChannelMessageReceived(Channel*,const char*,Channel::MessageSize)));
        connect(_sharedChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
                 this, SLOT(sharedChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));

        LOG_I("*****************Initializing GPS system*******************");

        _gpsServer = new GpsServer(this, SocketAddress(QHostAddress::Any, 1911), _log);
        connect(_gpsServer, SIGNAL(gpsUpdate(NmeaMessage)),
                this, SLOT(gpsUpdate(NmeaMessage)));

        LOG_I("*****************Initializing Video system*******************");

        _videoServers = new VideoServerArray(_log, this);
        _videoServers->populate(_config.BlacklistedUvdCameras, _config.FirstVideoPort, 0);

        if (_videoServers->cameraCount() > _config.MainComputerCameraCount) {
            LOG_E("The configuration specifies less cameras than this, the last ones will be removed");
            while (_videoServers->cameraCount() > _config.MainComputerCameraCount) {
                _videoServers->remove(_videoServers->cameraCount() - 1);
            }
        }
        else if (_videoServers->cameraCount() < _config.MainComputerCameraCount) {
            LOG_E("The configuration specifies more cameras than this, check cable connections");
        }

        LOG_I("*****************Initializing Audio system*******************");

        _audioServer = new AudioServer(69, SocketAddress(QHostAddress::Any, _config.AudioStreamPort), _log, this);

        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("Initialization complete");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
    }
}

void RoverProcess::beginSecondaryComputerListening() {
    if (_secondaryComputerBroadcastSocket) {
        _secondaryComputerBroadcastSocket->abort();
    }
    _secondaryComputerBroadcastSocket->bind(QHostAddress::Any, _config.SecondaryComputerPort);
    _secondaryComputerBroadcastSocket->open(QIODevice::ReadWrite);
}

void RoverProcess::secondaryComputerBroadcastSocketError(QAbstractSocket::SocketError err) {
    LOG_E("Error on secondary computer broadcast socket: " + _secondaryComputerBroadcastSocket->errorString());
    QTimer::singleShot(500, this, SLOT(beginSecondaryComputerListening()));
}

void RoverProcess::secondaryComputerStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel); Q_UNUSED(state);
    sendSystemStatusMessage();
}

void RoverProcess::sharedChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    if (state == Channel::ConnectedState) {
        // send all status information since we just connected
        QTimer::singleShot(1000, this, SLOT(sendSystemStatusMessage()));
    }
}

void RoverProcess::mbedChannelStateChanged(MbedChannel *channel, MbedChannel::State state) {
    Q_UNUSED(channel); Q_UNUSED(state);
    sendSystemStatusMessage();
}

void RoverProcess::sendSystemStatusMessage() {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    SharedMessageType messageType = SharedMessage_RoverStatusUpdate;
    bool armState = _armControllerMbed->getState() == MbedChannel::ConnectedState;
    bool driveGimbalState = _driveGimbalControllerMbed->getState() == MbedChannel::ConnectedState;
    bool secondaryComputerState = _secondaryComputerChannel->getState() == Channel::ConnectedState;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << armState;
    stream << driveGimbalState;
    stream << secondaryComputerState;
    _sharedChannel->sendMessage(message);
}

// observers for network channels message received

void RoverProcess::armChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    char header = message[0];
    MbedMessageType messageType;
    reinterpret_cast<quint32&>(messageType) = (quint32)reinterpret_cast<unsigned char&>(header);
    switch (messageType) {
    case MbedMessage_ArmGamepad:
    case MbedMessage_ArmMaster:
        _armControllerMbed->sendMessage(message, (int)size);
        break;
    default:
        LOG_E("Received invalid message from mission control on arm control channel");
        break;
    }
}

void RoverProcess::driveChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    char header = message[0];
    MbedMessageType messageType;
    reinterpret_cast<quint32&>(messageType) = (quint32)reinterpret_cast<unsigned char&>(header);
    switch (messageType) {
    case MbedMessage_Drive:
        _driveGimbalControllerMbed->sendMessage(message, (int)size);
        break;
    default:
        LOG_E("Received invalid message from mission control on drive control channel");
        break;
    }
}

void RoverProcess::gimbalChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    char header = message[0];
    MbedMessageType messageType;
    reinterpret_cast<quint32&>(messageType) = (quint32)reinterpret_cast<unsigned char&>(header);
    switch (messageType) {
    case MbedMessage_Gimbal:
        _driveGimbalControllerMbed->sendMessage(message, (int)size);
        break;
    default:
        LOG_E("Received invalid message from mission control on gimbal control channel");
        break;
    }
}

void RoverProcess::sharedChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
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
        if (camera >= _config.MainComputerCameraCount) {
            // this is the second odroid's camera
            QByteArray byteArray2;
            QDataStream stream2(&byteArray2, QIODevice::WriteOnly);
            stream2 << reinterpret_cast<quint32&>(messageType);
            stream2 << camera;
            stream2 << format;
            _secondaryComputerChannel->sendMessage(byteArray2);
        }
        else {
            _videoServers->activate(camera, format);
        }
    }
        break;
    case SharedMessage_RequestDeactivateCamera:
        qint32 camera;
        stream >> camera;
        if (camera >= _config.MainComputerCameraCount) {
            // this is the second odroid's camera
            QByteArray byteArray2;
            QDataStream stream2(&byteArray2, QIODevice::WriteOnly);
            stream2 << reinterpret_cast<quint32&>(messageType);
            stream2 << camera;
            _secondaryComputerChannel->sendMessage(byteArray2);
        }
        else {
            _videoServers->deactivate(camera);
        }
        break;
    case SharedMessage_RequestActivateAudioStream:
        AudioFormat format;
        stream >> reinterpret_cast<quint32&>(format);
        _audioServer->start("hw:1", format);
        break;
    case SharedMessage_RequestDeactivateAudioStream:
        _audioServer->stop();
        break;
    default:
        LOG_W("Got unknown shared channel message");
        break;
    }
}

void RoverProcess::secondaryComputerBroadcastSocketReadyRead() {
    char buffer[100];
    SocketAddress peer;
    while (_secondaryComputerBroadcastSocket->hasPendingDatagrams()) {
        int len = _secondaryComputerBroadcastSocket->readDatagram(&buffer[0], 100, &peer.host, &peer.port);
        if (strncmp(SECONDARY_COMPUTER_BROADCAST_STRING, buffer, len) == 0) {
            // secondary computer is broadcasting, respond
            LOG_I("Getting broadcast from secondary computer");
            _secondaryComputerBroadcastSocket->writeDatagram(MASTER_COMPUTER_BROADCAST_STRING, strlen(MASTER_COMPUTER_BROADCAST_STRING) + 1, peer.host, peer.port);
        }
    }
}

void RoverProcess::videoServerError(int cameraId, QString message) {
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_RoverVideoServerError;
    stream.setByteOrder(QDataStream::BigEndian);

    stream << reinterpret_cast<quint32&>(messageType);
    stream << (qint32)cameraId;
    stream << message;

    _sharedChannel->sendMessage(byteArray);
}

void RoverProcess::gpsUpdate(NmeaMessage message) {
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_RoverGpsUpdate;
    stream.setByteOrder(QDataStream::BigEndian);

    stream << reinterpret_cast<quint32&>(messageType);
    stream << message;

    _sharedChannel->sendMessage(byteArray);
}

RoverProcess::~RoverProcess() {
    if (_armChannel) {
        disconnect(_armChannel, 0, 0, 0);
        delete _armChannel;
    }
    if (_driveChannel) {
        disconnect(_driveChannel, 0, 0, 0);
        delete _driveChannel;
    }
    if (_sharedChannel) {
        disconnect(_sharedChannel, 0, 0, 0);
        delete _sharedChannel;
    }
    if (_secondaryComputerChannel) {
        disconnect(_secondaryComputerChannel, 0, 0, 0);
        delete _secondaryComputerChannel;
    }
    if (_armControllerMbed) {
        disconnect(_armControllerMbed, 0, 0, 0);
        delete _armControllerMbed;
    }
    if (_driveGimbalControllerMbed) {
        disconnect(_driveGimbalControllerMbed, 0, 0, 0);
        delete _driveGimbalControllerMbed;
    }
    if (_videoServers) {
        disconnect(_videoServers, 0, 0, 0);
        delete _videoServers;
    }
    if (_gpsServer) {
        disconnect(_gpsServer, 0, 0, 0);
        delete _gpsServer;
    }

    if (_log) delete _log;
}

} // namespace Rover
} // namespace Soro

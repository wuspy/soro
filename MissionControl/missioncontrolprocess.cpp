#include "missioncontrolprocess.h"

#define LOG_TAG "Mission Control"

//config
#define MASTER_ARM_INI_PATH QCoreApplication::applicationDirPath() + "/config/master_arm.ini"
//https://github.com/gabomdq/SDL_GameControllerDB
#define SDL_MAP_FILE_PATH QCoreApplication::applicationDirPath() + "/config/gamecontrollerdb.txt"

#define CONTROL_SEND_INTERVAL 50

namespace Soro {
namespace MissionControl {

MissionControlProcess::MissionControlProcess(QString name, bool masterSubnetNode, Role role, QObject *parent) : QObject(parent) {
    _isMaster = masterSubnetNode;
    _role = role;
    _name = name;

    _log = new Logger(this);
    _log->setLogfile(QCoreApplication::applicationDirPath() + "/mission_control" + QDateTime::currentDateTime().toString("M-dd_h:mm_AP") + ".log");
    _log->RouteToQtLogger = true;
    _log->MaxQtLoggerLevel = LOG_LEVEL_ERROR;

    ui = new SoroMainWindow();
    ui->onNameChanged(_name);
    ui->onRoleChanged(_role);
    ui->onMasterChanged(_isMaster);
    ui->show();

    _freeCameraWidgets.append(ui->getTopCameraWidget());
    _freeCameraWidgets.append(ui->getBottomCameraWidget());
    _freeCameraWidgets.append(ui->getFullscreenCameraWidget());

    connect(ui, SIGNAL(chatMessageEntered(QString)),
            this, SLOT(chatMessageEntered(QString)));
    connect(ui, SIGNAL(cycleVideosClockwise()),
            this, SLOT(cycleVideosClockwise()));
    connect(ui, SIGNAL(cycleVideosCounterclockwise()),
            this, SLOT(cycleVideosCounterClockwise()));
    connect(ui, SIGNAL(cameraFormatChanged(int,VideoFormat)),
            this, SLOT(cameraFormatSelected(int,VideoFormat)));
    connect(ui, SIGNAL(cameraNameEdited(int,QString)),
            this, SLOT(cameraNameEdited(int,QString)));
    connect(ui, SIGNAL(audioStreamFormatChanged(AudioFormat)),
            this, SLOT(audioStreamFormatSelected(AudioFormat)));
    connect(ui, SIGNAL(audioStreamMuteChanged(bool)),
            this, SLOT(audioStreamMuteSelected(bool)));

    QTimer::singleShot(1, this, SLOT(init()));
}

void MissionControlProcess::init() {
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");
    LOG_I("-----------------isMaster--------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    /***************************************
     * This code handles the initialization and reading the configuration file
     * This has to be run after the event loop has been started
     */
    //parse soro.ini configuration
    QString err = QString::null;
    if (!_config.load(&err)) {
        LOG_E(err);
        ui->onFatalError(err);
        return;
    }
    _config.applyLogLevel(_log);
    LOG_I("Configuration has been loaded successfully");

    LOG_I("***************Initializing Rover connections*********************");

    switch (_role) {
    case ArmOperatorRole:
        arm_loadMasterArmConfig();
        _masterArmChannel = new MbedChannel(SocketAddress(QHostAddress::Any, _config.MasterArmPort), MBED_ID_MASTER_ARM, _log);
        connect(_masterArmChannel, SIGNAL(messageReceived(const char*,int)),
                this, SLOT(arm_masterArmMessageReceived(const char*,int)));
        connect(_masterArmChannel, SIGNAL(stateChanged(MbedChannel*,MbedChannel::State)),
                this, SLOT(arm_masterArmStateChanged(MbedChannel*,MbedChannel::State)));
        _controlChannel = new Channel(this, SocketAddress(_config.ServerAddress, _config.ArmChannelPort), CHANNEL_NAME_ARM,
                Channel::UdpProtocol, QHostAddress::Any, _log);
        ui->arm_onMasterArmStateChanged(MbedChannel::ConnectingState);
        break;
    case DriverRole:
        initSDL();
        _controlChannel = new Channel(this, SocketAddress(_config.ServerAddress, _config.DriveChannelPort), CHANNEL_NAME_DRIVE,
                Channel::UdpProtocol, QHostAddress::Any, _log);
        break;
    case CameraOperatorRole:
        initSDL();
        _controlChannel = new Channel(this, SocketAddress(_config.ServerAddress, _config.GimbalChannelPort), CHANNEL_NAME_GIMBAL,
                Channel::UdpProtocol, QHostAddress::Any, _log);
        break;
    case SpectatorRole:
        //no control connections to create since spectators don't control anything
        break;
    }

    // start statistic timers
    if (_isMaster) {
        START_TIMER(_bitrateUpdateTimerId, 1000);
    }
    if (_role != SpectatorRole) { // spectator has no UDP connection to monitor
        START_TIMER(_droppedPacketTimerId, 5000);
        START_TIMER(_rttStatTimerId, 1000);
    }

    LOG_I("****************Initializing Mission Control network connections*******************");

    _broadcastSocket = new QUdpSocket(this);
    if (_isMaster) {
        LOG_I("Setting up as master subnet node");
        // create the main shared channel to connect to the rover
        _sharedChannel = new Channel(this, SocketAddress(_config.ServerAddress, _config.SharedChannelPort), CHANNEL_NAME_SHARED,
                Channel::TcpProtocol, QHostAddress::Any, _log);
        _sharedChannel->open();
        connect(_sharedChannel, SIGNAL(messageReceived(Channel*,const char*,Channel::MessageSize)),
                this, SLOT(master_roverSharedChannelMessageReceived(Channel*,const char*,Channel::MessageSize)));
        connect(_sharedChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
                this, SLOT(master_roverSharedChannelStateChanged(Channel*,Channel::State)));
        // create the udp broadcast receive port to listen to other mission control nodes trying to connect
        if (!_broadcastSocket->bind(QHostAddress::Any, _config.McBroadcastPort)) {
            ui->onFatalError("Unable to bind subnet broadcast port on port " + QString::number(_config.McBroadcastPort) +
                            ". You may be trying to run two master mission controls on the same computer, don\'t do that.");
            return;
        }
        if (!_broadcastSocket->open(QIODevice::ReadWrite)) {
            ui->onFatalError("Unable to open subnet broadcast port on port " + QString::number(_config.McBroadcastPort));
            return;
        }
        connect(_broadcastSocket, SIGNAL(readyRead()),
                this, SLOT(master_broadcastSocketReadyRead()));

        ui->onMccChannelStateChanged(Channel::ConnectedState);
    }
    else {
        LOG_I("Setting up as slave subnet node");
        // create a tcp channel on a random port to act as a server, and
        // create a udp socket on the same port to send out broadcasts with the
        // server's information so the master node can connect
        _sharedChannel = new Channel(this, 0, _name,
                Channel::TcpProtocol, QHostAddress::Any, _log);
        _sharedChannel->open();
        connect(_sharedChannel, SIGNAL(stateChanged(Channel*, Channel::State)),
                this, SLOT(slave_masterSharedChannelStateChanged(Channel*, Channel::State)));
        connect(_sharedChannel, SIGNAL(messageReceived(Channel*, const char*,Channel::MessageSize)),
                this, SLOT(slave_masterSharedChannelMessageReceived(Channel*, const char*,Channel::MessageSize)));
        if (!_broadcastSocket->bind(QHostAddress::Any, _sharedChannel->getHostAddress().port)) {
            ui->onFatalError("Unable to bind subnet broadcast port on " + QString::number(_sharedChannel->getHostAddress().port));
            return;
        }
        if (!_broadcastSocket->open(QIODevice::ReadWrite)) {
            ui->onFatalError("Unable to open subnet broadcast port on " + QString::number(_sharedChannel->getHostAddress().port));
            return;
        }
        // broadcast our metadata every 500 ms
        START_TIMER(_broadcastSharedChannelInfoTimerId, 500);
        // 3 second timeout to connect to the master node
        START_TIMER(_masterResponseWatchdogTimerId, 3000);

        ui->onMccChannelStateChanged(Channel::ConnectingState);
    }
    // also connect the shared channel's state changed event to this signal
    // so the UI can be updated. This signal is also broadcast when the rover
    // becomes disconnected from the master mission control, and they inform us
    // of that event since we are connected to the rover through them.
    if (_controlChannel != NULL) {
        connect(_controlChannel, SIGNAL(stateChanged(Channel*, Channel::State)),
                this, SLOT(controlChannelStateChanged(Channel*, Channel::State)));
        _controlChannel->open();
        if (_controlChannel->getState() == Channel::ErrorState) {
            ui->onFatalError("The control channel experienced a fatal error. This is most likely due to a configuration problem.");
            return;
        }

        ui->onControlChannelStateChanged(Channel::ConnectingState);
    }

    if (_sharedChannel->getState() == Channel::ErrorState) {
        ui->onFatalError("The shared data channel experienced a fatal error. This is most likely due to a configuration problem.");
        return;
    }

    ui->onSharedChannelStateChanged(Channel::ConnectingState);

    LOG_I("***************Initializing Video system******************");

    if (_isMaster) {
        LOG_I("Creating video clients for rover");
        for (int i = 0; i < _config.MainComputerCameraCount + _config.SecondaryComputerCameraCount; i++) {
            VideoClient *client = new VideoClient(i, SocketAddress(_config.ServerAddress, _config.FirstVideoPort + i), QHostAddress::Any, _log, this);

            connect(client, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
                    this, SLOT(videoClientStateChanged(MediaClient*,MediaClient::State)));

            // add localhost bounce to the video stream so the in-app player can display it from a udpsrc
            client->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, client->getServerAddress().port));
            _videoClients.append(client);
        }
    }
    for (int i = 0; i < _config.MainComputerCameraCount + _config.SecondaryComputerCameraCount; i++) {
        _videoFormats.append(VideoFormat_Null);
        _cameraNames.append("Camera " + QString::number(i + 1));
    }

    LOG_I("***************Initializing Audio system******************");

    if (_isMaster) {
        _audioClient = new AudioClient(69, SocketAddress(_config.ServerAddress, _config.AudioStreamPort), QHostAddress::Any, _log, this);
        // forward audio stream through localhost
        _audioClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, _config.AudioStreamPort));
        connect(_audioClient, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
                this, SLOT(audioClientStateChanged(MediaClient*,MediaClient::State)));
    }

    _audioPlayer = new AudioPlayer(this);
    _audioFormat = AudioFormat_Null;

    // update UI

    ui->onArmSubsystemStateChanged(UnknownSubsystemState);
    ui->onDriveCameraSubsystemStateChanged(UnknownSubsystemState);
    ui->onSecondaryComputerStateChanged(UnknownSubsystemState);
}

void MissionControlProcess::videoClientStateChanged(MediaClient *client, MediaClient::State state) {
    VideoClient *videoClient = reinterpret_cast<VideoClient*>(client);

    handleCameraStateChange(videoClient->getMediaId(), state, videoClient->getVideoFormat(), videoClient->getErrorString());

    // rebroadcast to other mission controls
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_CameraChanged;
    VideoFormat format = videoClient->getVideoFormat();

    stream << reinterpret_cast<quint32&>(messageType);
    stream << (qint32)client->getMediaId();
    stream << reinterpret_cast<quint32&>(state);
    stream << reinterpret_cast<quint32&>(format);
    stream << client->getErrorString();

    broadcastSharedMessage(message.constData(), message.size(), false);
}

void MissionControlProcess::audioClientStateChanged(MediaClient *client, MediaClient::State state) {
    AudioClient *audioClient = reinterpret_cast<AudioClient*>(client);

    handleAudioStateChanged(state, audioClient->getAudioFormat(), audioClient->getErrorString());

    // rebroadcast to other mission controls
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_AudioStreamChanged;
    AudioFormat format = audioClient->getAudioFormat();

    stream << reinterpret_cast<quint32&>(messageType);
    stream << reinterpret_cast<quint32&>(state);
    stream << reinterpret_cast<quint32&>(format);
    stream << client->getErrorString();

    broadcastSharedMessage(message.constData(), message.size(), false);
}

void MissionControlProcess::handleCameraStateChange(int cameraID, VideoClient::State state, VideoFormat format, QString errorString) {
    _videoFormats[cameraID] = format;
    switch (state) {
    case VideoClient::ConnectingState:
        if (_assignedCameraWidgets.contains(cameraID)) {
            if (_assignedCameraWidgets.contains(cameraID)) {
                if (errorString.isEmpty()) {
                    endStreamOnWidget(_assignedCameraWidgets.value(cameraID), "Trying to connect to the rover...");
                }
                else {
                    endStreamOnWidget(_assignedCameraWidgets.value(cameraID), "The rover experienced an error streaming this camera: " + errorString);
                }    void handleBitrateUpdate(int bpsRoverDown, int bpsRoverUp);
            }
        }
        break;
    case VideoClient::ConnectedState:
        if (_assignedCameraWidgets.contains(cameraID)) {
            if (errorString.isEmpty()) {
                endStreamOnWidget(_assignedCameraWidgets.value(cameraID), "This camera isn't being streamed right now.");
            }
            else {
                endStreamOnWidget(_assignedCameraWidgets.value(cameraID), "The rover experienced an error streaming this camera: " + errorString);
            }
        }
        break;
    case VideoClient::StreamingState:
        if (_assignedCameraWidgets.contains(cameraID)) {
            playStreamOnWidget(cameraID, _assignedCameraWidgets.value(cameraID), format);
        }
        else if (_freeCameraWidgets.size() > 0) {
            playStreamOnWidget(cameraID, _freeCameraWidgets.at(0), format);
        }
        break;
    }
}

void MissionControlProcess::playAudio() {
    if (_audioFormat != AudioFormat_Null) {
        if (_isMaster) {
            _audioPlayer->play(SocketAddress(QHostAddress::LocalHost, _config.AudioStreamPort), _audioFormat);
        }
        else {
            _audioPlayer->play(SocketAddress(QHostAddress::Any, _config.AudioStreamPort), _audioFormat);
        }
    }
}

void MissionControlProcess::handleAudioStateChanged(AudioClient::State state, AudioFormat encoding, QString errorString) {
    _audioFormat = encoding;
    ui->onAudioFormatChanged(encoding);
    switch (state) {
    case AudioClient::StreamingState:
        if (!ui->isMuteAudioSelected()) {
            playAudio();
        }
        break;
    default:
        _audioPlayer->stop();
        break;
    }
}

void MissionControlProcess::handleRoverSharedChannelStateChanged(Channel::State state) {
    switch (state) {
    case Channel::ConnectedState:
        _roverSharedChannelConnected = true;
        break;
    default:
        _roverSharedChannelConnected = false;
        // also update subsystem states
        ui->onArmSubsystemStateChanged(UnknownSubsystemState);
        ui->onDriveCameraSubsystemStateChanged(UnknownSubsystemState);
        ui->onSecondaryComputerStateChanged(UnknownSubsystemState);
        break;
    }
    ui->onSharedChannelStateChanged(state);
}

void MissionControlProcess::endStreamOnWidget(CameraWidget *widget, QString reason) {
    int oldCamera = _assignedCameraWidgets.key(widget, -1);
    if (oldCamera != -1) {
        _assignedCameraWidgets.remove(oldCamera);
        _videoFormats.replace(oldCamera, VideoFormat_Null);
        ui->onCameraFormatChanged(oldCamera, VideoFormat_Null);
    }
    if (_freeCameraWidgets.indexOf(widget) < 0) {
        _freeCameraWidgets.insert(0, widget);
    }
    widget->stop(reason);
    widget->setCameraName("No Video");
}

void MissionControlProcess::playStreamOnWidget(int cameraID, CameraWidget *widget, VideoFormat format) {
    if (_assignedCameraWidgets.contains(cameraID)) {
        CameraWidget *oldWidget = _assignedCameraWidgets.value(cameraID, NULL);
        if (oldWidget != widget) {
            oldWidget->stop();
            _assignedCameraWidgets.remove(cameraID);
            _freeCameraWidgets.append(oldWidget);
        }
    }
    _freeCameraWidgets.removeAll(widget);
    _assignedCameraWidgets.insert(cameraID, widget);
    _videoFormats.replace(cameraID, format);
    ui->onCameraFormatChanged(cameraID, format);
    if (_isMaster) {
        _assignedCameraWidgets.value(cameraID)->play(SocketAddress(QHostAddress::LocalHost, _config.FirstVideoPort + cameraID), format);
    }
    else {
        _assignedCameraWidgets.value(cameraID)->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + cameraID), format);
    }
    _assignedCameraWidgets.value(cameraID)->setCameraName(_cameraNames.at(cameraID));
}

void MissionControlProcess::controlChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    ui->onControlChannelStateChanged(state);
}

void MissionControlProcess::master_broadcastSocketReadyRead() {
    SocketAddress peer;
    Role peerRole;
    QString peerName;
    while (_broadcastSocket->hasPendingDatagrams()) {
        int len = _broadcastSocket->readDatagram(_buffer, sizeof(_buffer), &peer.host, &peer.port);
        // found a new mission control trying to connect
        QByteArray byteArray(_buffer, len);
        QDataStream stream(byteArray);
        stream >> reinterpret_cast<quint32&>(peerRole);
        stream >> peerName;
        // ensure they don't conflict with an existing mission control
        bool conflict = false;
        for (int i = 0; i < _slaveMissionControlChannels.size(); i++) {
            //check if they are already added
            if (_slaveMissionControlChannels[i]->getProvidedServerAddress() == peer) {
                // address conflict
                conflict = true;
                break;
            }
            if (_slaveMissionControlChannels[i]->getName().compare(peerName) == 0) {
                // name conflict
                conflict = true;
                break;
            }
            if ((peerRole != SpectatorRole) && (_slaveMissionControlRoles[i] == peerRole)) {
                // role conflict_videoServerArray
                conflict = true;
                break;
            }
        }
        if (!conflict) {
            // not already added, create a channel for them
            LOG_I("Creating new channel for node " + peer.toString());
            Channel *channel = new Channel(this, peer, peerName,
                               Channel::TcpProtocol, _broadcastSocket->localAddress(), _log);
            channel->open();
            _slaveMissionControlChannels.append(channel);
            _slaveMissionControlRoles.append(peerRole);
            connect(channel, SIGNAL(messageReceived(Channel*,const char*,Channel::MessageSize)),
                    this, SLOT(master_slaveSharedChannelMessageReceived(Channel*,const char*,Channel::MessageSize)));
            connect(channel, SIGNAL(stateChanged(Channel*,Channel::State)),
                    this, SLOT(master_slaveSharedChannelStateChanged(Channel*,Channel::State)));

            // forward video streams to their address as well (using the same port that the server uses
            // for consistency)
            foreach (VideoClient *client, _videoClients) {
                client->addForwardingAddress(SocketAddress(peer.host, client->getServerAddress().port));
            }
            // and audio stream
            _audioClient->addForwardingAddress(SocketAddress(peer.host, _config.AudioStreamPort));
        }
    }
}

void MissionControlProcess::broadcastSharedMessage(const char *message, int size, bool includeRover, Channel *exclude) {
    if (!_isMaster) {
        _sharedChannel->sendMessage(message, size);
        return;
    }
    if (includeRover) {
        _sharedChannel->sendMessage(message, size);
    }
    foreach (Channel *c, _slaveMissionControlChannels) {
        if (c != exclude) {
            c->sendMessage(message, size);
        }
    }
}

void MissionControlProcess::handleSharedChannelMessage(const char *message, Channel::MessageSize size) {
    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    SharedMessageType messageType;

    LOG_E("Getting shared channel message");

    stream >> reinterpret_cast<quint32&>(messageType);
    switch (messageType) {
    case SharedMessage_RoverSharedChannelStateChanged: {
        Channel::State state;
        stream >> reinterpret_cast<quint32&>(state);
        handleRoverSharedChannelStateChanged(state);
    }
        break;
    case SharedMessage_MissionControlConnected: {
        QString name;
        stream >> name;
        ui->onNotification(MCCNotification, name, name + " has connected to the mission control center network.");
    }
        break;
    case SharedMessage_MissionControlDisconnected: {
        QString name;
        stream >> name;
        ui->onNotification(MCCNotification, name, name + " has disconnected from the mission control center network.");
    }
        break;
    case SharedMessage_MissionControlChat: {
        QString name;
        QString message;
        stream >> name;
        stream >> message;
        ui->onNotification(ChatNotification, name , message);
    }
        break;
    case SharedMessage_RoverStatusUpdate: {
        bool armNormal, driveCameraNormal, secondComputerNormal;

        stream >> armNormal;
        stream >> driveCameraNormal;
        stream >> secondComputerNormal;

        _lastArmSubsystemState = armNormal ? NormalSubsystemState : MalfunctionSubsystemState;
        _lastDriveGimbalSubsystemState = driveCameraNormal ? NormalSubsystemState : MalfunctionSubsystemState;
        _lastSecondaryComputerSubsystemState = secondComputerNormal ? NormalSubsystemState : MalfunctionSubsystemState;

        ui->onArmSubsystemStateChanged(_lastArmSubsystemState);
        ui->onDriveCameraSubsystemStateChanged(_lastDriveGimbalSubsystemState);
        ui->onSecondaryComputerStateChanged(_lastSecondaryComputerSubsystemState);
    }
        break;
    case SharedMessage_CameraChanged: {
        qint32 cameraID;
        VideoFormat format;
        VideoClient::State state;
        QString errorString;

        stream >> cameraID;
        stream >> reinterpret_cast<quint32&>(state);
        stream >> reinterpret_cast<quint32&>(format);
        stream >> errorString;

        handleCameraStateChange(cameraID, state, format, errorString);
    }
        break;
    case SharedMessage_AudioStreamChanged: {
        AudioFormat format;
        AudioClient::State state;
        QString errorString;

        stream >> reinterpret_cast<quint32&>(state);
        stream >> reinterpret_cast<quint32&>(format);
        stream >> errorString;

        handleAudioStateChanged(state, format, errorString);
    }
        break;
    case SharedMessage_BitrateUpdate: {
        quint64 bpsRoverDown, bpsRoverUp;
        stream >> bpsRoverDown;
        stream >> bpsRoverUp;

        ui->onBitrateUpdate(bpsRoverDown, bpsRoverUp);
    }
        break;
    case SharedMessage_CameraNameChanged: {
        qint32 cameraId;
        QString newName;
        stream >> cameraId;
        stream >> newName;

        handleCameraNameChanged(cameraId, newName);
    }
        break;
    case SharedMessage_RoverVideoServerError: {
        qint32 cameraId;
        QString error;
        stream >> cameraId;
        stream >> error;

        LOG_E("Streaming error on camera " + QString::number(cameraId) + ": " + error);
    }
        break;
    case SharedMessage_RoverGpsUpdate: {
        LatLng coords;
        stream >> coords;
        ui->onLocationUpdate(coords);
    }
    default:
        LOG_E("Got unknown message header on shared channel");
        break;
    }
}

void MissionControlProcess::master_roverSharedChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);

    handleRoverSharedChannelStateChanged(state);

    // broadcast the new state to all other mission controls
    SharedMessageType messageType = SharedMessage_RoverSharedChannelStateChanged;
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);

    stream << reinterpret_cast<quint32&>(messageType);
    stream << reinterpret_cast<quint32&>(state);

    broadcastSharedMessage(message.constData(), message.size(), false);
}

void MissionControlProcess::master_roverSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    //message from the rover, rebroadcast to all other mission controls
    handleSharedChannelMessage(message, size);
    broadcastSharedMessage(message, size, false);
}

void MissionControlProcess::master_slaveSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    // message from another mission control, resend it to the rover and all other mission controls
    handleSharedChannelMessage(message, size);
    broadcastSharedMessage(message, size, true, channel);
}

void MissionControlProcess::master_slaveSharedChannelStateChanged(Channel *channel, Channel::State state) {
    switch (state) {
    case Channel::ConnectedState: {
        LOG_I("Mission control " + channel->getName() + " has connected");

        // notify all other mission controls that a new mission control has connected
        SharedMessageType messageType = SharedMessage_MissionControlConnected;
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);

        stream << reinterpret_cast<quint32&>(messageType);
        stream << channel->getName();

        broadcastSharedMessage(message.constData(), message.size(), false, channel);

        _newSlaveMissionControls.append(channel);
        QTimer::singleShot(2000, this, SLOT(sendWelcomePackets()));
    }
        break;
    default: // assume anything other than connected is bad
        if (channel->wasConnected()) {
            LOG_W("Mission control " + channel->getName() + " appears to have disconnected");
            // nofity all mission controls that a mission control has disconnected
            SharedMessageType messageType = SharedMessage_MissionControlDisconnected;
            QByteArray message;
            QDataStream stream(&message, QIODevice::WriteOnly);

            stream << reinterpret_cast<quint32&>(messageType);
            stream << channel->getName();

            broadcastSharedMessage(message.constData(), message.size(), false, channel);

            // remove the mission control's old channel and hope they connect again via broadcasting
            if (channel->wasConnected() && (state != Channel::ConnectedState)) {
                // remove the channel and wait for them to hopefully reconnect
                disconnect(channel, 0, 0, 0);
                channel->close();
                delete channel;
                int index = _slaveMissionControlChannels.indexOf(channel);
                if (index >= 0) {
                    _slaveMissionControlChannels.removeAt(index);
                    _slaveMissionControlRoles.removeAt(index);
                }
            }
        }
        break;
    }
}

void MissionControlProcess::slave_masterSharedChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    switch (state) {
    case Channel::ConnectedState:
        LOG_I("Connected to master mission control");
        // connected to the master mission control, stop broadcasting
        KILL_TIMER(_broadcastSharedChannelInfoTimerId);
        KILL_TIMER(_masterResponseWatchdogTimerId);
        break;
    case Channel::ConnectingState:
        LOG_W("Lost connection with the master mission control");
        if (channel->wasConnected()) {
            // lost connection to the master mission control, start rebroadcasting
            // our information
            START_TIMER(_broadcastSharedChannelInfoTimerId, 500);
        }
        handleRoverSharedChannelStateChanged(Channel::ConnectingState);
        break;
    case Channel::ErrorState:
        ui->onFatalError("The shared channel experienced a fatal error");
        break;
    default:
        break;
    }
    ui->onMccChannelStateChanged(state);
}

void MissionControlProcess::slave_masterSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    handleSharedChannelMessage(message, size);
}

void MissionControlProcess::sendWelcomePackets() {
    // send the new mission controls all the information it needs to get up to date

    foreach (Channel *channel, _newSlaveMissionControls) {
        // send rover connection state

        SharedMessageType messageType = SharedMessage_RoverSharedChannelStateChanged;
        QByteArray roverConnectionMessage;
        QDataStream stream(&roverConnectionMessage, QIODevice::WriteOnly);
        Channel::State roverState = _sharedChannel->getState();
        stream << reinterpret_cast<quint32&>(messageType);
        stream << reinterpret_cast<quint32&>(roverState);

        channel->sendMessage(roverConnectionMessage);

        // send rover subsystem state

        QByteArray roverSubsystemMessage;
        QDataStream stream2(&roverSubsystemMessage, QIODevice::WriteOnly);
        messageType = SharedMessage_RoverStatusUpdate;

        stream2 << reinterpret_cast<quint32&>(messageType);
        stream2 << (_lastArmSubsystemState == NormalSubsystemState);
        stream2 << (_lastDriveGimbalSubsystemState == NormalSubsystemState);
        stream2 << (_lastArmSubsystemState == NormalSubsystemState);

        channel->sendMessage(roverSubsystemMessage);

        // send video states

        foreach(VideoClient *client, _videoClients) {
            QByteArray videoClientMessage;
            QDataStream stream3(&videoClientMessage, QIODevice::WriteOnly);
            messageType = SharedMessage_CameraChanged;
            VideoClient::State state = client->getState();
            VideoFormat format = client->getVideoFormat();

            stream3 << reinterpret_cast<quint32&>(messageType);
            stream3 << (qint32)client->getMediaId();
            stream3 << reinterpret_cast<quint32&>(state);
            stream3 << reinterpret_cast<quint32&>(format);
            stream3 << client->getErrorString();

            channel->sendMessage(videoClientMessage);
        }

        // send audio state

        QByteArray audioClientState;
        QDataStream stream4(&audioClientState, QIODevice::WriteOnly);
        messageType = SharedMessage_AudioStreamChanged;
        VideoClient::State state = _audioClient->getState();
        AudioFormat format = _audioClient->getAudioFormat();

        stream4 << reinterpret_cast<quint32&>(messageType);
        stream4 << reinterpret_cast<quint32&>(state);
        stream4 << reinterpret_cast<quint32&>(format);
        stream4 << _audioClient->getErrorString();

        channel->sendMessage(audioClientState);

        // send camera names

        for (int i = 0; i < _cameraNames.size(); i++) {
            // rebroadcast to other mission controls
            QByteArray message;
            QDataStream stream(&message, QIODevice::WriteOnly);
            SharedMessageType messageType = SharedMessage_CameraNameChanged;

            stream << reinterpret_cast<quint32&>(messageType);
            stream << (qint32)i;
            stream << _cameraNames.at(i);

            broadcastSharedMessage(message.constData(), message.size(), false);
        }
    }

    _newSlaveMissionControls.clear();
}

void MissionControlProcess::chatMessageEntered(QString message) {
    // broadcast the message to all other mission controls
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_MissionControlChat;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << _name;
    stream << message;

    broadcastSharedMessage(byteArray.constData(), byteArray.size(), false);
}

void MissionControlProcess::cameraFormatSelected(int camera, VideoFormat format) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType;

    if (format == VideoFormat_Null) {
        messageType = SharedMessage_RequestDeactivateCamera;

        stream << reinterpret_cast<quint32&>(messageType);
        stream << (qint32)camera;
    }
    else {
        messageType = SharedMessage_RequestActivateCamera;

        stream << reinterpret_cast<quint32&>(messageType);
        stream << (qint32)camera;
        stream << reinterpret_cast<quint32&>(format);
    }

    _sharedChannel->sendMessage(message);
}

void MissionControlProcess::cameraNameEdited(int camera, QString newName) {
    handleCameraNameChanged(camera, newName);

    // rebroadcast to other mission controls
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_CameraNameChanged;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << (qint32)camera;
    stream << newName;

    broadcastSharedMessage(message.constData(), message.length(), false);
}

void MissionControlProcess::audioStreamFormatSelected(AudioFormat format) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType;

    if (format == AudioFormat_Null) {
        messageType = SharedMessage_RequestDeactivateAudioStream;

        stream << reinterpret_cast<quint32&>(messageType);
    }
    else {
        messageType = SharedMessage_RequestActivateAudioStream;
        stream << reinterpret_cast<quint32&>(messageType);
        stream << reinterpret_cast<quint32&>(format);
    }

    _sharedChannel->sendMessage(message);
}

void MissionControlProcess::audioStreamMuteSelected(bool mute) {
    if (mute) {
        _audioPlayer->stop();
    }
    else if (_audioFormat) {
        playAudio();
    }
}

void MissionControlProcess::handleCameraNameChanged(int camera, QString newName) {
    _cameraNames.replace(camera, newName);
    ui->setCameraName(camera, newName);
    if (_assignedCameraWidgets.contains(camera)) {
        _assignedCameraWidgets.value(camera)->setCameraName(newName);
    }
}

void MissionControlProcess::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _controlSendTimerId) {
        /***************************************
         * This timer sends gamepad data to the rover at a regular interval
         * Not applicable for master arm control, the mbed controls the
         * packet rate in that scenario
         */
        if (_gameController) {
            SDL_GameControllerUpdate();
            if (!SDL_GameControllerGetAttached(_gameController)) {
                _gameController = NULL;
                START_TIMER(_inputSelectorTimerId, 1000);
                ui->onGamepadChanged(NULL);
                return;
            }
            switch (_role) {
            case ArmOperatorRole:
                //send the rover an arm gamepad packet
                ArmMessage::setGamepadData(_buffer,
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY),
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_RIGHTY),
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT),
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT),
                                           SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER),
                                           SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER),
                                           SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_Y),
                                           SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_X));
                _controlChannel->sendMessage(_buffer, ArmMessage::RequiredSize_Gamepad);
                break;
            case DriverRole:
                //send the rover a drive gamepad packet
                switch (_driveGamepadMode) {
                case SingleStickDrive:
                    DriveMessage::setGamepadData_DualStick(_buffer,
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY),
                                                 _driveMiddleSkidSteerFactor);
                    break;
                case DualStickDrive:
                    DriveMessage::setGamepadData_DualStick(_buffer,
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY),
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_RIGHTY),
                                                 _driveMiddleSkidSteerFactor);
                    break;
                }
                _controlChannel->sendMessage(_buffer, DriveMessage::RequiredSize);
                break;
            case CameraOperatorRole:
                //send the rover a gimbal gamepad packet
                GimbalMessage::setGamepadData(_buffer,
                                              SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                              SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_RIGHTY),
                                              SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_X),
                                              SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_Y),
                                              SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_B),
                                              SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_A));
                _controlChannel->sendMessage(_buffer, GimbalMessage::RequiredSize);
                break;
            default:
                break;
            }

            // use the gamepad to cycle the videos as well
            if (SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT) == 1) {
                if (!_ignoreGamepadVideoButtons) {
                    cycleVideosClockwise();
                    _ignoreGamepadVideoButtons = true;
                }
            }
            else if (SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) == 1) {
                if (!_ignoreGamepadVideoButtons) {
                    cycleVideosClockwise();
                    _ignoreGamepadVideoButtons = true;
                }
            }
            else {
                _ignoreGamepadVideoButtons = false;
            }
        }
    }
    else if (e->timerId() == _inputSelectorTimerId) {
        /***************************************
         * This timer querys SDL at regular intervals to look for a
         * suitable controller
         */
        SDL_GameControllerUpdate();
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                SDL_GameController *controller = SDL_GameControllerOpen(i);
                if (controller && SDL_GameControllerMapping(controller)) {
                    //this gamepad will do
                    _gameController = controller;
                    ui->onGamepadChanged(controller);
                    KILL_TIMER(_inputSelectorTimerId);
                    return;
                }
                SDL_GameControllerClose(controller);
            }
        }
    }
    else if (e->timerId() == _broadcastSharedChannelInfoTimerId) {
        /***************************************
         * (slave) This timer broadcasts our information to the entire subnet so
         * the master mission control can connect to us
         */
        LOG_I("Broadcasting shared channel information on address "
              + SocketAddress(_broadcastSocket->localAddress(), _broadcastSocket->localPort()).toString());
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream << reinterpret_cast<quint32&>(_role);
        stream << _name;
        _broadcastSocket->writeDatagram(message.constData(), message.size() + 1,
                                        QHostAddress::Broadcast, _config.McBroadcastPort);
    }
    else if (e->timerId() == _masterResponseWatchdogTimerId) {
        /****************************************
         * (slave) This timer only runs once in the event the master mission control
         * does not respond back in time. In this event, an error is shown
         * to the user
         */
        LOG_E("Master mission control did not respond in time");
        ui->onFatalError("Unable to connect to the mission control network. Please make sure that:\n\n"
                   "1. There is a master mission control running on the network, and there is no firewall issues.\n"
                   "2. The name you chose isn't already being used by an existing mission control.\n"
                   "3. The role you selected (Arm Operator, Driver, etc.) isn't already claimed by an existin mission control.");
        return;
    }
    else if (e->timerId() == _droppedPacketTimerId) {
        /****************************************
         * This timer runs every few seconds to update the
         * dropped packet statistic
         */
        ui->onDroppedPacketRateUpdate(_controlChannel->getUdpDroppedPacketsPercent());
    }
    else if (e->timerId() == _rttStatTimerId) {
        /****************************************
         * This timer runs regularly to update the
         * rtt (ping) statistic
         */
        ui->onRttUpdate(_controlChannel->getLastRtt());
    }
    else if (e->timerId() == _bitrateUpdateTimerId) {
        /*****************************************
         * (master) This timer regularly updates the total bitrate count,
         * and also broadcasts it to slave mission controls since they
         * cannot calculate video bitrate
         */
        quint64 bpsRoverDown = 0, bpsRoverUp = 0;
        foreach (VideoClient *client, _videoClients) {
            bpsRoverUp += client->getBitrate();
        }
        bpsRoverUp += _audioClient->getBitrate();
        bpsRoverUp += _sharedChannel->getBitsPerSecondDown();
        bpsRoverDown += _sharedChannel->getBitsPerSecondUp();
        if (_controlChannel != NULL) {
            bpsRoverUp += _controlChannel->getBitsPerSecondDown();
            bpsRoverDown += _controlChannel->getBitsPerSecondUp();
        }

        ui->onBitrateUpdate(bpsRoverDown, bpsRoverUp);

        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        SharedMessageType messageType = SharedMessage_BitrateUpdate;

        stream << reinterpret_cast<quint32&>(messageType);
        stream << bpsRoverDown;
        stream << bpsRoverUp;

        broadcastSharedMessage(message.constData(), message.size(), false);
    }
}

void MissionControlProcess::cycleVideosClockwise() {
    int oldTop = _assignedCameraWidgets.key(ui->getTopCameraWidget(), -1);
    int oldBottom = _assignedCameraWidgets.key(ui->getBottomCameraWidget(), -1);
    int oldFullscreen = _assignedCameraWidgets.key(ui->getFullscreenCameraWidget(), -1);
    ui->getTopCameraWidget()->stop("Switching video mode...");
    ui->getBottomCameraWidget()->stop("Switching video mode...");
    ui->getFullscreenCameraWidget()->stop("Switching video mode...");
    _assignedCameraWidgets.clear();
    if (oldFullscreen >= 0) {
        playStreamOnWidget(oldFullscreen, ui->getBottomCameraWidget(), _videoFormats.at(oldFullscreen));
    }
    else {
        endStreamOnWidget(ui->getBottomCameraWidget(), "This camera isn't being streamed right now.");
    }
    if (oldTop >= 0) {
        playStreamOnWidget(oldTop, ui->getFullscreenCameraWidget(), _videoFormats.at(oldTop));
    }
    else {
        endStreamOnWidget(ui->getFullscreenCameraWidget(), "This camera isn't being streamed right now.");
    }
    if (oldBottom >= 0) {
        playStreamOnWidget(oldBottom, ui->getTopCameraWidget(), _videoFormats.at(oldBottom));
    }
    else {
        endStreamOnWidget(ui->getTopCameraWidget(), "This camera isn't being streamed right now.");
    }
}

void MissionControlProcess::cycleVideosCounterClockwise() {
    int oldTop = _assignedCameraWidgets.key(ui->getTopCameraWidget(), -1);
    int oldBottom = _assignedCameraWidgets.key(ui->getBottomCameraWidget(), -1);
    int oldFullscreen = _assignedCameraWidgets.key(ui->getFullscreenCameraWidget(), -1);
    ui->getTopCameraWidget()->stop("Switching video mode...");
    ui->getBottomCameraWidget()->stop("Switching video mode...");
    ui->getFullscreenCameraWidget()->stop("Switching video mode...");
    _assignedCameraWidgets.clear();
    if (oldFullscreen >= 0) {
        playStreamOnWidget(oldFullscreen, ui->getTopCameraWidget(), _videoFormats.at(oldFullscreen));
    }
    else {
        endStreamOnWidget(ui->getTopCameraWidget(), "This camera isn't being streamed right now.");
    }
    if (oldTop >= 0) {
        playStreamOnWidget(oldTop, ui->getBottomCameraWidget(), _videoFormats.at(oldTop));
    }
    else {
        endStreamOnWidget(ui->getBottomCameraWidget(), "This camera isn't being streamed right now.");
    }
    if (oldBottom >= 0) {
        playStreamOnWidget(oldBottom, ui->getFullscreenCameraWidget(), _videoFormats.at(oldBottom));
    }
    else {
        endStreamOnWidget(ui->getFullscreenCameraWidget(), "This camera isn't being streamed right now.");
    }
}

/* Initializes SDL for gamepad input and loads
 * the gamepad map file.
 */
void MissionControlProcess::initSDL() {
    LOG_D("initSDL()");
    if (!_sdlInitialized) {
        LOG_I("Initializing SDL");
        if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
            ui->onFatalError("SDL failed to initialize: " + QString(SDL_GetError()));
            return;
        }
        _sdlInitialized = true;
        _gameController = NULL;
        if (SDL_GameControllerAddMappingsFromFile((SDL_MAP_FILE_PATH).toLocal8Bit().constData()) == -1) {
            ui->onFatalError("Failed to load SDL gamepad map: " + QString(SDL_GetError()));
            return;
        }
        START_TIMER(_controlSendTimerId, CONTROL_SEND_INTERVAL);
        START_TIMER(_inputSelectorTimerId, 1000);
        ui->onGamepadChanged(NULL);
    }
}

/* Facade for SDL_Quit that cleans up some things here as well
 */
void MissionControlProcess::quitSDL() {
    LOG_D("quitSDL()");
    if (_sdlInitialized) {
        SDL_Quit();
        _gameController = NULL;
        _sdlInitialized = false;
    }
}

void MissionControlProcess::arm_masterArmStateChanged(MbedChannel *channel, MbedChannel::State state) {
    Q_UNUSED(channel);
    ui->arm_onMasterArmStateChanged(state);
}

void MissionControlProcess::arm_masterArmMessageReceived(const char *message, int size) {
    LOG_D("arm_masterArmMessageReceived()");
    memcpy(_buffer, message, size);
    //translate message from master pot values to slave servo values
    ArmMessage::translateMasterArmValues(_buffer, _masterArmRanges);
    qDebug() << "yaw=" << ArmMessage::getMasterYaw(_buffer)
             << ", shldr=" << ArmMessage::getMasterShoulder(_buffer)
             << ", elbow=" << ArmMessage::getMasterElbow(_buffer)
             << ", wrist=" << ArmMessage::getMasterWrist(_buffer); /**/
    if (_controlChannel != NULL) {
        _controlChannel->sendMessage(_buffer, size);
    }
    else {
        LOG_E("Got message from master arm with null control channel");
    }
}

void MissionControlProcess::arm_loadMasterArmConfig() {
    LOG_D("loadMasterArmConfig()");
    if (_role == ArmOperatorRole) {
        QFile masterArmFile(MASTER_ARM_INI_PATH);
        if (_masterArmRanges.load(masterArmFile)) {
            LOG_I("Loaded master arm configuration");
        }
        else {
            ui->onFatalError("The master arm configuration file " + MASTER_ARM_INI_PATH + " is either missing or invalid");
        }
    }
}

QString MissionControlProcess::getName() const {
    return _name;
}

Logger* MissionControlProcess::getLogger() {
    return _log;
}

const SoroIniLoader *MissionControlProcess::getConfiguration() const {
    return &_config;
}

void MissionControlProcess::drive_setMiddleSkidSteerFactor(float factor) {
    _driveMiddleSkidSteerFactor = factor;
}

void MissionControlProcess::drive_setGamepadMode(DriveGamepadMode mode) {
    _driveGamepadMode = mode;
}

float MissionControlProcess::drive_getMiddleSkidSteerFactor() const {
    return _driveMiddleSkidSteerFactor;
}

DriveGamepadMode MissionControlProcess::drive_getGamepadMode() const {
    return _driveGamepadMode;
}

Role MissionControlProcess::getRole() const {
    return _role;
}

bool MissionControlProcess::isMasterSubnetNode() const {
    return _isMaster;
}

MissionControlProcess::~MissionControlProcess() {
    quitSDL();
    foreach (Channel *c, _slaveMissionControlChannels) {
        disconnect(c, 0, 0, 0);
        delete c;
    }
    if (_controlChannel) {
        disconnect(_controlChannel, 0, 0, 0);
        delete _controlChannel;
    }
    if (_sharedChannel) {
        disconnect(_sharedChannel, 0, 0, 0);
        delete _sharedChannel;
    }
    if (_masterArmChannel) {
        disconnect(_masterArmChannel, 0, 0, 0);
        delete _masterArmChannel;
    }

    if (_log) delete _log;
}

}
}

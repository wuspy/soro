#include "missioncontrolprocess.h"

#define LOG_TAG "Mission Control"

//config
#define MASTER_ARM_INI_PATH QCoreApplication::applicationDirPath() + "/config/master_arm.ini"
//https://github.com/gabomdq/SDL_GameControllerDB
#define SDL_MAP_FILE_PATH QCoreApplication::applicationDirPath() + "/config/gamecontrollerdb.txt"

#define CONTROL_SEND_INTERVAL 50

namespace Soro {
namespace MissionControl {

MissionControlProcess::MissionControlProcess(QString name, CameraWidget *topVideo, CameraWidget *bottomVideo, CameraWidget *fullscreenVideo,
                                             bool masterSubnetNode, MissionControlProcess::Role role, QMainWindow *presenter) : QObject(presenter) {
    _masterMissionControl = masterSubnetNode;
    _role = role;
    _topVideoWidget = topVideo;
    _bottomVideoWidget = bottomVideo;
    _fullscreenVideoWidget = fullscreenVideo;
    _name = name;
    _log = new Logger(this);
    _log->setLogfile(QCoreApplication::applicationDirPath() + "/mission_control" + QDateTime::currentDateTime().toString("M-dd_h:mm_AP") + ".log");
    _log->RouteToQtLogger = true;
    _log->MaxQtLoggerLevel = LOG_LEVEL_ERROR;
}

void MissionControlProcess::init() {
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");
    LOG_I("-------------------------------------------------------");
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
        emit fatalError(err);
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
        connect(_masterArmChannel, SIGNAL(stateChanged(MbedChannel *, MbedChannel::State)),
                this, SIGNAL(arm_masterArmStateChanged(MbedChannel *, MbedChannel::State)));
        if (_config.ServerSide == SoroIniLoader::MissionControlEndPoint) {
            _controlChannel = new Channel(this, _config.ArmChannelPort, CHANNEL_NAME_ARM,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        else {
            _controlChannel = new Channel(this, SocketAddress(_config.ServerAddress, _config.ArmChannelPort), CHANNEL_NAME_ARM,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        break;
    case DriverRole:
        initSDL();
        if (_config.ServerSide == SoroIniLoader::MissionControlEndPoint) {
            _controlChannel = new Channel(this, _config.DriveChannelPort, CHANNEL_NAME_DRIVE,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        else {
            _controlChannel = new Channel(this, SocketAddress(_config.ServerAddress, _config.DriveChannelPort), CHANNEL_NAME_DRIVE,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        break;
    case CameraOperatorRole:
        initSDL();
        if (_config.ServerSide == SoroIniLoader::MissionControlEndPoint) {
            _controlChannel = new Channel(this, _config.GimbalChannelPort, CHANNEL_NAME_GIMBAL,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        else {
            _controlChannel = new Channel(this, SocketAddress(_config.ServerAddress, _config.GimbalChannelPort), CHANNEL_NAME_GIMBAL,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        break;
    case SpectatorRole:
        //no control connections to create since spectators don't control anything
        break;
    }

    // start statistic timers
    START_TIMER(_rttStatTimerId, 1000);
    if (_role != SpectatorRole) { // spectator has no UDP connection to monitor
        START_TIMER(_droppedPacketTimerId, 5000);
    }


    LOG_I("****************Initializing Mission Control network connections*******************");

    _broadcastSocket = new QUdpSocket(this);
    if (_masterMissionControl) {
        LOG_I("Setting up as master subnet node");
        // create the main shared channel to connect to the rover
        if (_config.ServerSide == SoroIniLoader::MissionControlEndPoint) {
            _sharedChannel = new Channel(this, _config.SharedChannelPort, CHANNEL_NAME_SHARED,
                    Channel::TcpProtocol, QHostAddress::Any, _log);
        }
        else {
            _sharedChannel = new Channel(this, SocketAddress(_config.ServerAddress, _config.SharedChannelPort), CHANNEL_NAME_SHARED,
                    Channel::TcpProtocol, QHostAddress::Any, _log);
        }
        _sharedChannel->open();
        connect(_sharedChannel, SIGNAL(messageReceived(Channel*,const char*,Channel::MessageSize)),
                this, SLOT(master_roverSharedChannelMessageReceived(Channel*,const char*,Channel::MessageSize)));
        connect(_sharedChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
                this, SLOT(master_roverSharedChannelStateChanged(Channel*,Channel::State)));
        // create the udp broadcast receive port to listen to other mission control nodes trying to connect
        if (!_broadcastSocket->bind(QHostAddress::Any, _config.McBroadcastPort)) {
            emit fatalError("Unable to bind subnet broadcast port on port " + QString::number(_config.McBroadcastPort) +
                            ". You may be trying to run two master mission controls on the same computer, don\'t do that.");
            return;
        }
        if (!_broadcastSocket->open(QIODevice::ReadWrite)) {
            emit fatalError("Unable to open subnet broadcast port on port " + QString::number(_config.McBroadcastPort));
            return;
        }
        connect(_broadcastSocket, SIGNAL(readyRead()),
                this, SLOT(master_broadcastSocketReadyRead()));
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
            emit fatalError("Unable to bind subnet broadcast port on " + QString::number(_sharedChannel->getHostAddress().port));
            return;
        }
        if (!_broadcastSocket->open(QIODevice::ReadWrite)) {
            emit fatalError("Unable to open subnet broadcast port on " + QString::number(_sharedChannel->getHostAddress().port));
            return;
        }
        START_TIMER(_broadcastSharedChannelInfoTimerId, 500);
        START_TIMER(_masterResponseWatchdogTimerId, 3000); // 3 seconds should be plenty
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
            emit fatalError("The control channel experienced a fatal error. This is most likely due to a configuration problem.");
            return;
        }
    }

    if (_sharedChannel->getState() == Channel::ErrorState) {
        emit fatalError("The shared data channel experienced a fatal error. This is most likely due to a configuration problem.");
        return;
    }

    LOG_I("***************Initializing Video system******************");

    if (_masterMissionControl) {
        LOG_I("Creating video clients for rover");
        for (int i = 0; i < 5; i++) {
            VideoClient *client = new VideoClient("Camera " + QString::number(i + 1),
                                                  SocketAddress(_config.ServerAddress, _config.FirstVideoPort + i),
                                                  QHostAddress::Any, _log, this);
            connect(client, SIGNAL(stateChanged(VideoClient*,VideoClient::State)),
                    this, SLOT(videoClientStateChanged(VideoClient*,VideoClient::State)));

            _videoClients.append(client);
            _streamFormats.append(StreamFormat());
        }
    }

    //activate the rover's first 3 cameras
    _videoWidgets.insert(0, _topVideoWidget);
    _videoWidgets.insert(1, _bottomVideoWidget);
    _videoWidgets.insert(2, _fullscreenVideoWidget);

    _videoWidgets[0]->stop("Waiting for connection...");
    _videoWidgets[1]->stop("Waiting for connection...");
    _videoWidgets[2]->stop("Waiting for connection...");
}

void MissionControlProcess::videoClientStateChanged(VideoClient *client, VideoClient::State state) {
    int cameraID = _videoClients.indexOf(client);
    handleCameraStateChange(cameraID, state, client->getStreamFormat(), client->getErrorString());

    // rebroadcast to other mission controls
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_CameraChanged;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << (qint32)cameraID;
    stream << reinterpret_cast<quint32&>(state);
    stream << client->getStreamFormat();
    stream << client->getErrorString();

    broadcastSharedMessage(message.constData(), message.size(), false);
}

void MissionControlProcess::handleCameraStateChange(int cameraID, VideoClient::State state, StreamFormat format, QString errorString) {
    _streamFormats[cameraID] = format;
    switch (state) {
    case VideoClient::ConnectingState:
        if (_videoWidgets.contains(cameraID)) {
            if (errorString.isEmpty()) {
                _videoWidgets[cameraID]->stop("Trying to connect with the rover...");
            }
            else {
                _videoWidgets[cameraID]->stop("The rover experienced an error streaming this camera: " + errorString);
            }
        }
        break;
    case VideoClient::ConnectedState:
        if (_videoWidgets.contains(cameraID)) {
            if (errorString.isEmpty()) {
                _videoWidgets[cameraID]->stop("You\'re connected, but the rover isn\'t sending this stream right now.");
            }
            else {
                _videoWidgets[cameraID]->stop("The rover experienced an error streaming this camera: " + errorString);
            }
        }
        break;
    case VideoClient::StreamingState:
        if (_videoWidgets.contains(cameraID)) {
            if (_masterMissionControl) {
                _videoWidgets[cameraID]->play(_videoClients[cameraID]->element(), format.Encoding);
            }
            else {
                _videoWidgets[cameraID]->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + cameraID), format.Encoding);
            }
        }
        break;
    }
}

void MissionControlProcess::controlChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    emit connectionStateChanged(state,
                                _masterMissionControl ? Channel::ConnectedState : _sharedChannel->getState(),
                                _roverSharedChannelConnected ? Channel::ConnectedState : Channel::ConnectingState);
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
                // role conflict
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
        }
    }
}

void MissionControlProcess::broadcastSharedMessage(const char *message, int size, bool includeRover, Channel *exclude) {
    if (!_masterMissionControl) {
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
    }
        break;
    case SharedMessage_MissionControlConnected: {
        QString name;
        stream >> name;
        emit notification(MCCNotification, name, name + " has connected to the mission control center network.");
    }
        break;
    case SharedMessage_MissionControlDisconnected: {
        QString name;
        stream >> name;
        emit notification(MCCNotification, name, name + " has disconnected from the mission control center network.");
    }
        break;
    case SharedMessage_MissionControlChat: {
        QString name;
        QString message;
        stream >> name;
        stream >> message;
        emit notification(ChatNotification, name , message);
    }
        break;
    case SharedMessage_RoverDisconnected: {
        _roverSharedChannelConnected = false;
        // emit rover system state update because we no longer know the system state of the rover
        emit roverSystemStateUpdate(UnknownSubsystemState, UnknownSubsystemState, UnknownSubsystemState);
        // emit connection state changed signal
        emit connectionStateChanged(_controlChannel == NULL ? Channel::ErrorState : _controlChannel->getState(),
                                    Channel::ConnectingState, _sharedChannel->getState());
    }
        break;
    case SharedMessage_RoverStatusUpdate: {
        _roverSharedChannelConnected = true;
        RoverSubsystemState armSubsystemState, driveCameraSubsystemState;
        bool armNormal, driveCameraNormal;

        stream >> armNormal;
        stream >> driveCameraNormal;

        armSubsystemState = armNormal ? NormalSubsystemState : MalfunctionSubsystemState;
        driveCameraSubsystemState = driveCameraNormal ? NormalSubsystemState : MalfunctionSubsystemState;
        //TODO secondary computer
        emit roverSystemStateUpdate(armSubsystemState, driveCameraSubsystemState, MalfunctionSubsystemState);
    }
        break;
    case SharedMessage_CameraChanged: {
        qint32 cameraID;
        StreamFormat format;
        VideoClient::State state;
        QString errorString;

        stream >> cameraID;
        stream >> reinterpret_cast<quint32&>(state);
        stream >> format;
        stream >> errorString;

        handleCameraStateChange(cameraID, state, format, errorString);
    }
        break;
    case SharedMessage_RoverVideoServerError: {
        // find out which stream had the error
        qint32 cameraID;
        QString message;
        stream >> cameraID;
        stream >> message;

        // if the stream that experienced the error is currently playing, stop it and display
        // the error instead
        if (_videoWidgets.contains(cameraID)) {
            _videoWidgets[cameraID]->stop("The rover experienced an error streaming this camera: " + message);
            _videoWidgets.remove(cameraID);
        }
    }
        break;
    default:
        LOG_E("Got unknown message header on shared channel");
        break;
    }
}

void MissionControlProcess::master_roverSharedChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    _roverSharedChannelConnected = state == Channel::ConnectedState;

    // broadcast the new state to all other mission controls
    SharedMessageType messageType = SharedMessage_RoverSharedChannelStateChanged;
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);

    stream << reinterpret_cast<quint32&>(messageType);
    stream << reinterpret_cast<quint32&>(state);

    broadcastSharedMessage(message.constData(), message.size(), false);

    if (state == Channel::ConnectingState) {
        // emit rover system state update because we no longer know the system state of the rover
        emit roverSystemStateUpdate(UnknownSubsystemState, UnknownSubsystemState, UnknownSubsystemState);
    }
    // emit connection state changed signal
    emit connectionStateChanged(_controlChannel == NULL ? Channel::ErrorState : _controlChannel->getState(),
                                Channel::ConnectedState, _sharedChannel->getState());
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
        //connected to the master mission control, stop broadcasting
        KILL_TIMER(_broadcastSharedChannelInfoTimerId);
        KILL_TIMER(_masterResponseWatchdogTimerId);
        break;
    case Channel::ConnectingState:
        LOG_W("Lost connection with the master mission control");
        if (channel->wasConnected()) {
            // lost connection to the master mission control, start rebroadcasting
            // our information
            START_TIMER(_broadcastSharedChannelInfoTimerId, 500);
            START_TIMER(_masterResponseWatchdogTimerId, 3000);
        }
        _roverSharedChannelConnected = false;
        // emit rover system state update because we no longer know the system state of the rover
        emit roverSystemStateUpdate(UnknownSubsystemState, UnknownSubsystemState, UnknownSubsystemState);
        break;
    case Channel::ErrorState:
        emit fatalError("The shared channel experienced a fatal error");
        break;
    default:
        break;
    }
    emit connectionStateChanged(_controlChannel == NULL ? Channel::ErrorState : _controlChannel->getState(),
                                _sharedChannel->getState(),
                                _roverSharedChannelConnected ? Channel::ConnectedState : Channel::ConnectingState);
}

void MissionControlProcess::slave_masterSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    handleSharedChannelMessage(message, size);
}

void MissionControlProcess::postChatMessage(QString message) {
    // broadcast the message to all other mission controls
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_MissionControlChat;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << _name;
    stream << message;

    broadcastSharedMessage(byteArray.constData(), byteArray.size(), false);
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
                delete _gameController;
                _gameController = NULL;
                START_TIMER(_inputSelectorTimerId, 1000);
                emit gamepadChanged(NULL);
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
                                           SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_Y));
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
                                              SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY));
                _controlChannel->sendMessage(_buffer, GimbalMessage::RequiredSize);
                break;
            default:
                break;
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
                    emit gamepadChanged(controller);
                    KILL_TIMER(_inputSelectorTimerId);
                    return;
                }
                SDL_GameControllerClose(controller);
                delete controller;
            }
        }
    }
    else if (e->timerId() == _broadcastSharedChannelInfoTimerId) {
        /***************************************
         * This timer broadcasts our information to the entire subnet so
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
         * This timer only runs once in the event the master mission control
         * does not respond back in time. In this event, an error is shown
         * to the user
         */
        LOG_E("Master mission control did not respond in time");
        emit fatalError("Unable to connect to the mission control network. Please make sure that:\n\n"
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
        emit droppedPacketRateUpdate(_controlChannel->getUdpDroppedPacketsPercent());
    }
    else if (e->timerId() == _rttStatTimerId) {
        /****************************************
         * This timer runs regularly to update the
         * rtt (ping) statistic
         */
        emit rttUpdate(_controlChannel->getLastRtt());
    }
}

void MissionControlProcess::cycleVideosClockwise() {
    int oldTop = _videoWidgets.key(_topVideoWidget, -1);
    int oldBottom = _videoWidgets.key(_bottomVideoWidget, -1);
    int oldFullscreen = _videoWidgets.key(_fullscreenVideoWidget, -1);
    _topVideoWidget->stop("Switching video mode...");
    _bottomVideoWidget->stop("Switching video mode...");
    _fullscreenVideoWidget->stop("Switching video mode...");
    _videoWidgets.clear();
    if (oldFullscreen >= 0) {
        if (_masterMissionControl) {
            _bottomVideoWidget->play(_videoClients[oldFullscreen]->element(), _streamFormats[oldFullscreen].Encoding);
        }
        else {
            _bottomVideoWidget->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + oldFullscreen), _streamFormats[oldFullscreen].Encoding);
        }
        _videoWidgets.insert(oldFullscreen, _bottomVideoWidget);
    }
    else {
        _bottomVideoWidget->stop("This camera isn't being streamed right now.");
    }
    if (oldTop >= 0) {
        if (_masterMissionControl) {
            _fullscreenVideoWidget->play(_videoClients[oldTop]->element(), _streamFormats[oldTop].Encoding);
        }
        else {
            _fullscreenVideoWidget->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + oldTop), _streamFormats[oldTop].Encoding);
        }
        _videoWidgets.insert(oldTop, _fullscreenVideoWidget);
    }
    else {
        _fullscreenVideoWidget->stop("This camera isn't being streamed right now.");
    }
    if (oldBottom >= 0) {
        if (_masterMissionControl) {
            _topVideoWidget->play(_videoClients[oldBottom]->element(), _streamFormats[oldBottom].Encoding);
        }
        else {
            _topVideoWidget->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + oldBottom), _streamFormats[oldBottom].Encoding);
        }
        _videoWidgets.insert(oldBottom, _topVideoWidget);
    }
    else {
        _topVideoWidget->stop("This camera isn't being streamed right now.");
    }
}

void MissionControlProcess::cycleVideosCounterClockwise() {
    int oldTop = _videoWidgets.key(_topVideoWidget, -1);
    int oldBottom = _videoWidgets.key(_bottomVideoWidget, -1);
    int oldFullscreen = _videoWidgets.key(_fullscreenVideoWidget, -1);
    _topVideoWidget->stop("Switching video mode...");
    _bottomVideoWidget->stop("Switching video mode...");
    _fullscreenVideoWidget->stop("Switching video mode...");
    _videoWidgets.clear();
    if (oldTop >= 0) {
        if (_masterMissionControl) {
            _bottomVideoWidget->play(_videoClients[oldTop]->element(), _streamFormats[oldTop].Encoding);
        }
        else {
            _bottomVideoWidget->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + oldTop), _streamFormats[oldTop].Encoding);
        }
        _videoWidgets.insert(oldTop, _bottomVideoWidget);
    }
    else {
        _bottomVideoWidget->stop("This camera isn't being streamed right now.");
    }
    if (oldBottom >= 0) {
        if (_masterMissionControl) {
            _fullscreenVideoWidget->play(_videoClients[oldBottom]->element(), _streamFormats[oldBottom].Encoding);
        }
        else {
            _fullscreenVideoWidget->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + oldBottom), _streamFormats[oldBottom].Encoding);
        }
        _videoWidgets.insert(oldBottom, _fullscreenVideoWidget);
    }
    else {
        _fullscreenVideoWidget->stop("This camera isn't being streamed right now.");
    }
    if (oldFullscreen >= 0) {
        if (_masterMissionControl) {
            _topVideoWidget->play(_videoClients[oldFullscreen]->element(), _streamFormats[oldFullscreen].Encoding);
        }
        else {
            _topVideoWidget->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + oldFullscreen), _streamFormats[oldFullscreen].Encoding);
        }
        _videoWidgets.insert(oldFullscreen, _topVideoWidget);
    }
    else {
        _topVideoWidget->stop("This camera isn't being streamed right now.");
    }
}

void MissionControlProcess::playCamera(int cameraID, CameraWidget *widget) {
    int oldCameraID = _videoWidgets.key(widget);
    if (oldCameraID >= 0) {
        _videoWidgets.remove(oldCameraID);
    }

    if (_streamFormats[cameraID].Encoding == UnknownOrNoEncoding) {
        widget->stop("This camera isn't being streamed right now");
    }
    else {
        if (_masterMissionControl) {
            widget->play(_videoClients[cameraID]->element(), _streamFormats[cameraID].Encoding);
        }
        else {
            widget->play(SocketAddress(QHostAddress::Any, _config.FirstVideoPort + cameraID), _streamFormats[cameraID].Encoding);
        }
        widget->setCameraName("Camera " + QString::number(cameraID + 1));
        _videoWidgets.insert(cameraID, widget);
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
            emit fatalError("SDL failed to initialize: " + QString(SDL_GetError()));
            return;
        }
        _sdlInitialized = true;
        _gameController = NULL;
        if (SDL_GameControllerAddMappingsFromFile((SDL_MAP_FILE_PATH).toLocal8Bit().constData()) == -1) {
            emit fatalError("Failed to load SDL gamepad map: " + QString(SDL_GetError()));
            return;
        }
        START_TIMER(_controlSendTimerId, CONTROL_SEND_INTERVAL);
        START_TIMER(_inputSelectorTimerId, 1000);
        emit initializedSDL();
        emit gamepadChanged(NULL);
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
            emit fatalError("The master arm configuration file " + MASTER_ARM_INI_PATH + " is either missing or invalid");
        }
    }
}

QString MissionControlProcess::getName() const {
    return _name;
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

MissionControlProcess::DriveGamepadMode MissionControlProcess::drive_getGamepadMode() const {
    return _driveGamepadMode;
}

MissionControlProcess::Role MissionControlProcess::getRole() const {
    return _role;
}

bool MissionControlProcess::isMasterSubnetNode() const {
    return _masterMissionControl;
}

MissionControlProcess::~MissionControlProcess() {
    quitSDL();
    foreach (Channel *c, _slaveMissionControlChannels) {
        delete c;
    }
    if (_controlChannel != NULL) delete _controlChannel;
    if (_sharedChannel != NULL) delete _sharedChannel;
    if (_masterArmChannel != NULL) delete _masterArmChannel;
    if (_log != NULL) delete _log;
}

}
}

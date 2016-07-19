#include "missioncontrolprocess.h"

#define LOG_TAG "Mission Control"

#define MASTER_ARM_FILE "master_arm.ini"
#define CONTROL_SEND_INTERVAL 50

namespace Soro {
namespace MissionControl {

MissionControlProcess::MissionControlProcess(const Configuration *config, GamepadManager *gamepad,
                                              MissionControlNetwork *mcNetwork, ControlSystem *controlSystem, QObject *parent) : QObject(parent) {
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "Starting mission control process...");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");

    _mcNetwork = mcNetwork;
    _controlSystem = controlSystem;
    _gamepad = gamepad;
    _config = config;

    // Create UI
    _ui = new MainWindow(config, gamepad, mcNetwork, controlSystem);
    connect(_ui, SIGNAL(closed()), this, SIGNAL(windowClosed()));
    _ui->show();

    _freeCameraWidgets.append(_ui->getTopCameraWidget());
    _freeCameraWidgets.append(_ui->getBottomCameraWidget());
    _freeCameraWidgets.append(_ui->getFullscreenCameraWidget());

    connect(_ui, SIGNAL(cycleVideosClockwise()),
            this, SLOT(cycleVideosClockwise()));
    connect(_ui, SIGNAL(cycleVideosCounterclockwise()),
            this, SLOT(cycleVideosCounterClockwise()));
    connect(_ui, SIGNAL(cameraFormatChanged(int,VideoFormat)),
            this, SLOT(cameraFormatSelected(int,VideoFormat)));
    connect(_ui, SIGNAL(cameraNameEdited(int,QString)),
            this, SLOT(cameraNameEdited(int,QString)));
    connect(_ui, SIGNAL(audioStreamFormatChanged(AudioFormat)),
            this, SLOT(audioStreamFormatSelected(AudioFormat)));
    connect(_ui, SIGNAL(audioStreamMuteChanged(bool)),
            this, SLOT(audioStreamMuteSelected(bool)));

    connect(_mcNetwork, SIGNAL(newClientConnected(Channel*)),
            this, SLOT(onNewMissionControlClient(Channel*)));

    LOG_I(LOG_TAG, "****************Initializing connections*******************");

    if (_mcNetwork->isBroker()) {
        LOG_I(LOG_TAG, "Setting up as rover shared connection");
        // Create the main shared channel to connect to the rover
        _roverChannel = Channel::createClient(this, SocketAddress(_config->ServerAddress, _config->SharedChannelPort), CHANNEL_NAME_SHARED,
                Channel::TcpProtocol, QHostAddress::Any);
        _roverChannel->open();
        connect(_roverChannel, SIGNAL(messageReceived(Channel*,const char*,Channel::MessageSize)),
                this, SLOT(roverSharedChannelMessageReceived(Channel*,const char*,Channel::MessageSize)));
        connect(_roverChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
                this, SLOT(roverSharedChannelStateChanged(Channel*,Channel::State)));
    }

    if (_controlSystem) {
        _controlSystem->enable();
    }

    LOG_I(LOG_TAG, "***************Initializing Video system******************");

    if (_mcNetwork->isBroker()) {
        LOG_I(LOG_TAG, "Creating video clients for rover");
        for (int i = 0; i < _config->MainComputerCameraCount + _config->SecondaryComputerCameraCount; i++) {
            VideoClient *client = new VideoClient(i, SocketAddress(_config->ServerAddress, _config->FirstVideoPort + i), QHostAddress::Any, this);

            connect(client, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
                    this, SLOT(videoClientStateChanged(MediaClient*,MediaClient::State)));

            // add localhost bounce to the video stream so the in-app player can display it from a udpsrc
            client->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, client->getServerAddress().port));
            _videoClients.append(client);
        }
    }
    for (int i = 0; i < _config->MainComputerCameraCount + _config->SecondaryComputerCameraCount; i++) {
        _videoFormats.append(VideoFormat_Null);
        _cameraNames.append("Camera " + QString::number(i + 1));
    }

    LOG_I(LOG_TAG, "***************Initializing Audio system******************");

    if (_mcNetwork->isBroker()) {
        _audioClient = new AudioClient(69, SocketAddress(_config->ServerAddress, _config->AudioStreamPort), QHostAddress::Any, this);
        // forward audio stream through localhost
        _audioClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, _config->AudioStreamPort));
        connect(_audioClient, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
                this, SLOT(audioClientStateChanged(MediaClient*,MediaClient::State)));
    }

    _audioPlayer = new AudioPlayer(this);
    _audioFormat = AudioFormat_Null;

    // Start statistic timers
    if (_mcNetwork->isBroker()) {
        START_TIMER(_bitrateUpdateTimerId, 1000);
    }
    if (_mcNetwork->getRole() != SpectatorRole) {
        // spectator has no UDP connection to monitor
        START_TIMER(_droppedPacketTimerId, 5000);
        START_TIMER(_rttStatTimerId, 1000);
    }
}

/* Receives the signal of a video client starting or stopping a video stream. Only applicable to the broker,
 * as other mission control do not directly receive the rover's video streams.
 */
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

    _mcNetwork->sendSharedMessage(message.constData(), message.size());
}

/* Receives the signal of a video client starting or stopping a video stream. Only applicable to the broker,
 * as other mission control do not directly receive the rover's audio stream.
 */
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

    _mcNetwork->sendSharedMessage(message.constData(), message.size());
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
        if (_mcNetwork->isBroker()) {
            // Play direct rover audio stream
            _audioPlayer->play(SocketAddress(QHostAddress::LocalHost, _config->AudioStreamPort), _audioFormat);
        }
        else {
            // Play audio forwarded by the broker
            _audioPlayer->play(SocketAddress(QHostAddress::Any, _config->AudioStreamPort), _audioFormat);
        }
    }
}

void MissionControlProcess::handleAudioStateChanged(AudioClient::State state, AudioFormat encoding, QString errorString) {
    _audioFormat = encoding;
    _ui->onAudioFormatChanged(encoding);
    switch (state) {
    case AudioClient::StreamingState:
        if (!_ui->isMuteAudioSelected()) {
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
        _ui->onArmSubsystemStateChanged(UnknownSubsystemState);
        _ui->onDriveCameraSubsystemStateChanged(UnknownSubsystemState);
        _ui->onSecondaryComputerStateChanged(UnknownSubsystemState);
        break;
    }
    _ui->onRoverChannelStateChanged(state);
}

void MissionControlProcess::endStreamOnWidget(CameraWidget *widget, QString reason) {
    int oldCamera = _assignedCameraWidgets.key(widget, -1);
    if (oldCamera != -1) {
        _assignedCameraWidgets.remove(oldCamera);
        _videoFormats.replace(oldCamera, VideoFormat_Null);
        _ui->onCameraFormatChanged(oldCamera, VideoFormat_Null);
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
    _ui->onCameraFormatChanged(cameraID, format);
    if (_mcNetwork->isBroker()) {
        _assignedCameraWidgets.value(cameraID)->play(SocketAddress(QHostAddress::LocalHost, _config->FirstVideoPort + cameraID), format);
    }
    else {
        _assignedCameraWidgets.value(cameraID)->play(SocketAddress(QHostAddress::Any, _config->FirstVideoPort + cameraID), format);
    }
    _assignedCameraWidgets.value(cameraID)->setCameraName(_cameraNames.at(cameraID));
}

void MissionControlProcess::handleSharedChannelMessage(const char *message, Channel::MessageSize size) {
    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    SharedMessageType messageType;

    LOG_E(LOG_TAG, "Getting shared channel message");

    stream >> reinterpret_cast<quint32&>(messageType);
    switch (messageType) {
    case SharedMessage_RoverSharedChannelStateChanged: {
        Channel::State state;
        stream >> reinterpret_cast<quint32&>(state);
        handleRoverSharedChannelStateChanged(state);
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

        _ui->onArmSubsystemStateChanged(_lastArmSubsystemState);
        _ui->onDriveCameraSubsystemStateChanged(_lastDriveGimbalSubsystemState);
        _ui->onSecondaryComputerStateChanged(_lastSecondaryComputerSubsystemState);
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

        _ui->onBitrateUpdate(bpsRoverDown, bpsRoverUp);
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

        LOG_E(LOG_TAG, "Streaming error on camera " + QString::number(cameraId) + ": " + error);
    }
        break;
    case SharedMessage_RoverGpsUpdate: {
        NmeaMessage *message = new NmeaMessage;
        stream >> *message;
        _ui->onLocationUpdate(*message);

        if (_mcNetwork->isBroker()) {
            _gpsMessages.append(message);
        }
        else {
            delete message;
        }
    }
    default:
        LOG_E(LOG_TAG, "Got unknown message header on shared channel");
        break;
    }
}

/* Receives the signal when the shard channel connection to the rover is lost. Only applicable to the broker.
 */
void MissionControlProcess::roverSharedChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);

    handleRoverSharedChannelStateChanged(state);

    // broadcast the new state to all other mission controls
    SharedMessageType messageType = SharedMessage_RoverSharedChannelStateChanged;
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);

    stream << reinterpret_cast<quint32&>(messageType);
    stream << reinterpret_cast<quint32&>(state);

    _mcNetwork->sendSharedMessage(message.constData(), message.size());
}

void MissionControlProcess::roverSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);

    handleSharedChannelMessage(message, size);
    _mcNetwork->sendSharedMessage(message, size);
}

/* Receives the new client connected signal from the MissionControlNetwork module.
 * Only applicable to the broker.
 */
void MissionControlProcess::onNewMissionControlClient(Channel *channel) {
    // send the new mission controls all the information it needs to get up to date

    // send rover connection state

    SharedMessageType messageType = SharedMessage_RoverSharedChannelStateChanged;
    QByteArray roverConnectionMessage;
    QDataStream stream(&roverConnectionMessage, QIODevice::WriteOnly);
    Channel::State roverState = _roverChannel->getState();
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

        channel->sendMessage(message.constData(), message.size());
    }

    // send gps locations

    foreach (NmeaMessage *nmeaMessage, _gpsMessages) {
        // rebroadcast to other mission controls
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        SharedMessageType messageType = SharedMessage_RoverGpsUpdate;

        stream << reinterpret_cast<quint32&>(messageType);
        stream << *nmeaMessage;

        channel->sendMessage(message.constData(), message.size());
    }
}

/* Receives the signal from the UI when a new camera format is selected
 */
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

    if (_mcNetwork->isBroker() && _roverChannel) {
        _roverChannel->sendMessage(message);
    }
    else {
        _mcNetwork->sendSharedMessage(message.constData(), message.size());
    }
}

/* Receives the signal from the UI when the name of a camera has been changed
 */
void MissionControlProcess::cameraNameEdited(int camera, QString newName) {
    handleCameraNameChanged(camera, newName);

    // rebroadcast to other mission controls
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_CameraNameChanged;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << (qint32)camera;
    stream << newName;

    _mcNetwork->sendSharedMessage(message.constData(), message.size());
}

/* Receives the signal from the UI when a new audio stream format is selected
 */
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

    if (_mcNetwork->isBroker() && _roverChannel) {
        _roverChannel->sendMessage(message);
    }
    else {
        _mcNetwork->sendSharedMessage(message.constData(), message.size());
    }
}

/* Receives the signal from the UI when the user requests to mute the audio
 */
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
    _ui->setCameraName(camera, newName);
    if (_assignedCameraWidgets.contains(camera)) {
        _assignedCameraWidgets.value(camera)->setCameraName(newName);
    }
}

void MissionControlProcess::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _droppedPacketTimerId) {
        /****************************************
         * This timer runs every few seconds to update the
         * dropped packet statistic
         */
        _ui->onDroppedPacketRateUpdate(_controlSystem->getChannel()->getUdpDroppedPacketsPercent());
    }
    else if (e->timerId() == _rttStatTimerId) {
        /****************************************
         * This timer runs regularly to update the
         * rtt (ping) statistic
         */
        _ui->onRttUpdate(_controlSystem->getChannel()->getLastRtt());
    }
    else if (e->timerId() == _bitrateUpdateTimerId) {
        /*****************************************
         * (broker) This timer regularly updates the total bitrate count,
         * and also broadcasts it to slave mission controls since they
         * cannot calculate video bitrate
         */
        quint64 bpsRoverDown = 0, bpsRoverUp = 0;
        foreach (VideoClient *client, _videoClients) {
            bpsRoverUp += client->getBitrate();
        }
        bpsRoverUp += _audioClient->getBitrate();
        bpsRoverUp += _roverChannel->getBitsPerSecondDown();
        bpsRoverDown += _roverChannel->getBitsPerSecondUp();
        if (_controlSystem != NULL) {
            bpsRoverUp += _controlSystem->getChannel()->getBitsPerSecondDown();
            bpsRoverDown += _controlSystem->getChannel()->getBitsPerSecondUp();
        }

        _ui->onBitrateUpdate(bpsRoverDown, bpsRoverUp);

        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        SharedMessageType messageType = SharedMessage_BitrateUpdate;

        stream << reinterpret_cast<quint32&>(messageType);
        stream << bpsRoverDown;
        stream << bpsRoverUp;

        _mcNetwork->sendSharedMessage(message.constData(), message.size());
    }
}

void MissionControlProcess::cycleVideosClockwise() {
    int oldTop = _assignedCameraWidgets.key(_ui->getTopCameraWidget(), -1);
    int oldBottom = _assignedCameraWidgets.key(_ui->getBottomCameraWidget(), -1);
    int oldFullscreen = _assignedCameraWidgets.key(_ui->getFullscreenCameraWidget(), -1);
    _ui->getTopCameraWidget()->stop("Switching video mode...");
    _ui->getBottomCameraWidget()->stop("Switching video mode...");
    _ui->getFullscreenCameraWidget()->stop("Switching video mode...");
    _assignedCameraWidgets.clear();
    if (oldFullscreen >= 0) {
        playStreamOnWidget(oldFullscreen, _ui->getBottomCameraWidget(), _videoFormats.at(oldFullscreen));
    }
    else {
        endStreamOnWidget(_ui->getBottomCameraWidget(), "This camera isn't being streamed right now.");
    }
    if (oldTop >= 0) {
        playStreamOnWidget(oldTop, _ui->getFullscreenCameraWidget(), _videoFormats.at(oldTop));
    }
    else {
        endStreamOnWidget(_ui->getFullscreenCameraWidget(), "This camera isn't being streamed right now.");
    }
    if (oldBottom >= 0) {
        playStreamOnWidget(oldBottom, _ui->getTopCameraWidget(), _videoFormats.at(oldBottom));
    }
    else {
        endStreamOnWidget(_ui->getTopCameraWidget(), "This camera isn't being streamed right now.");
    }
}

void MissionControlProcess::cycleVideosCounterClockwise() {
    int oldTop = _assignedCameraWidgets.key(_ui->getTopCameraWidget(), -1);
    int oldBottom = _assignedCameraWidgets.key(_ui->getBottomCameraWidget(), -1);
    int oldFullscreen = _assignedCameraWidgets.key(_ui->getFullscreenCameraWidget(), -1);
    _ui->getTopCameraWidget()->stop("Switching video mode...");
    _ui->getBottomCameraWidget()->stop("Switching video mode...");
    _ui->getFullscreenCameraWidget()->stop("Switching video mode...");
    _assignedCameraWidgets.clear();
    if (oldFullscreen >= 0) {
        playStreamOnWidget(oldFullscreen, _ui->getTopCameraWidget(), _videoFormats.at(oldFullscreen));
    }
    else {
        endStreamOnWidget(_ui->getTopCameraWidget(), "This camera isn't being streamed right now.");
    }
    if (oldTop >= 0) {
        playStreamOnWidget(oldTop, _ui->getBottomCameraWidget(), _videoFormats.at(oldTop));
    }
    else {
        endStreamOnWidget(_ui->getBottomCameraWidget(), "This camera isn't being streamed right now.");
    }
    if (oldBottom >= 0) {
        playStreamOnWidget(oldBottom, _ui->getFullscreenCameraWidget(), _videoFormats.at(oldBottom));
    }
    else {
        endStreamOnWidget(_ui->getFullscreenCameraWidget(), "This camera isn't being streamed right now.");
    }
}

const Configuration *MissionControlProcess::getConfiguration() const {
    return _config;
}

MissionControlProcess::~MissionControlProcess() {
    if (_roverChannel) {
        disconnect(_roverChannel, 0, 0, 0);
        delete _roverChannel;
    }
}

}
}

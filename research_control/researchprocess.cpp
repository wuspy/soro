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

#include "researchprocess.h"

#define LOG_TAG "Research Control"

#define DEFAULT_VIDEO_STEREO_MODE VideoFormat::StereoMode_SideBySide

namespace Soro {
namespace MissionControl {

ResearchControlProcess::ResearchControlProcess(QHostAddress roverAddress, GamepadManager *gamepad, QQmlEngine *qml, QObject *parent) : QObject(parent) {
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "Starting research control process...");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");

    _gamepad = gamepad;
    connect(_gamepad, SIGNAL(gamepadChanged(SDL_GameController*,QString)),
            this, SLOT(gamepadChanged(SDL_GameController*,QString)));

    _settings = SettingsModel::Default(roverAddress);

    // Create UI for rover control
    _mainUi = new ResearchMainWindow();
    _mainUi->getCameraWidget()->setStereoMode(VideoFormat::StereoMode_SideBySide);
    connect(_mainUi, SIGNAL(closed()), this, SIGNAL(windowClosed()));

    // Create UI for settings and control
    QQmlComponent qmlComponent(qml, QUrl("qrc:/Control.qml"));
    QObject *componentObject = qmlComponent.create();
    if (!qmlComponent.errorString().isEmpty()) {
        LOG_E(LOG_TAG, "Cannot create QML: " + qmlComponent.errorString());
        QCoreApplication::exit(1);
    }
    else {
        _controlUi = qobject_cast<QQuickWindow*>(componentObject);
    }

    // Perform initial setup of control UI

    connect(_controlUi, SIGNAL(requestUiSync()),
            this, SLOT(ui_requestUiSync()));
    connect(_controlUi, SIGNAL(settingsApplied()),
            this, SLOT(ui_settingsApplied()));

    // This is the directory mbed parser will log to
    if (!QDir(QCoreApplication::applicationDirPath() + "/../research-data/sensors").exists()) {
        LOG_I(LOG_TAG, "/../research-data/sensors directory does not exist, creating it");
        if (!QDir().mkdir(QCoreApplication::applicationDirPath() + "/../research-data/sensors")) {
            LOG_E(LOG_TAG, "Cannot create /../research-data/sensors directory, sensor data may not be logged");
        }
    }
    // This is the directory gps logger will log to
    if (!QDir(QCoreApplication::applicationDirPath() + "/../research-data/gps").exists()) {
        LOG_I(LOG_TAG, "/../research-data/gps directory does not exist, creating it");
        if (!QDir().mkdir(QCoreApplication::applicationDirPath() + "/../research-data/gps")) {
            LOG_E(LOG_TAG, "Cannot create /../research-data/gps directory, sensor data may not be logged");
        }
    }

    LOG_I(LOG_TAG, "****************Initializing connections*******************");

    LOG_I(LOG_TAG, "Setting up rover shared connection");
    // Create the main shared channel to connect to the rover
    _roverChannel = Channel::createClient(this, SocketAddress(_settings.roverAddress, NETWORK_ALL_SHARED_CHANNEL_PORT), CHANNEL_NAME_SHARED,
            Channel::TcpProtocol, QHostAddress::Any);
    _roverChannel->open();
    connect(_roverChannel, SIGNAL(messageReceived(Channel*,const char*,Channel::MessageSize)),
            this, SLOT(roverSharedChannelMessageReceived(Channel*,const char*,Channel::MessageSize)));
    connect(_roverChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
            this, SLOT(updateUiConnectionState()));

    LOG_I(LOG_TAG, "Creating drive control system");
    _driveSystem = new DriveControlSystem(_settings.roverAddress, _gamepad, this);
    connect(_driveSystem, SIGNAL(connectionStateChanged(Channel::State)),
            this, SLOT(driveConnectionStateChanged(Channel::State)));
    QString err;
    if (!_driveSystem->init(&err)) {
        LOG_E(LOG_TAG, "Drive system failed to init: " + err);
        QCoreApplication::exit(1);
    }
    _driveSystem->enable();

    // create mbed data parser
    connect(&_mbedParser, SIGNAL(dataParsed(MbedDataParser::DataTag,float)),
            this, SLOT(newSensorData(MbedDataParser::DataTag,float)));

    LOG_I(LOG_TAG, "***************Initializing Video system******************");

    _stereoLVideoClient = new VideoClient(MEDIAID_RESEARCH_SL_CAMERA, SocketAddress(_settings.roverAddress, NETWORK_ALL_RESEARCH_SL_CAMERA_PORT), QHostAddress::Any, this);
    _stereoRVideoClient = new VideoClient(MEDIAID_RESEARCH_SR_CAMERA, SocketAddress(_settings.roverAddress, NETWORK_ALL_RESEARCH_SR_CAMERA_PORT), QHostAddress::Any, this);
    _aux1VideoClient = new VideoClient(MEDIAID_RESEARCH_A1_CAMERA, SocketAddress(_settings.roverAddress, NETWORK_ALL_RESEARCH_A1L_CAMERA_PORT), QHostAddress::Any, this);
    _monoVideoClient = new VideoClient(MEDIAID_RESEARCH_M_CAMERA, SocketAddress(_settings.roverAddress, NETWORK_ALL_RESEARCH_ML_CAMERA_PORT), QHostAddress::Any, this);

    connect(_stereoLVideoClient, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
            this, SLOT(videoClientStateChanged(MediaClient*,MediaClient::State)));
    connect(_stereoRVideoClient, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
            this, SLOT(videoClientStateChanged(MediaClient*,MediaClient::State)));
    connect(_aux1VideoClient, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
            this, SLOT(videoClientStateChanged(MediaClient*,MediaClient::State)));
    connect(_monoVideoClient, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
            this, SLOT(videoClientStateChanged(MediaClient*,MediaClient::State)));

    // add localhost bounce to the video stream so the in-app player can display it from a udpsrc
    _stereoLVideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_SL_CAMERA_PORT));
    _stereoRVideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_SR_CAMERA_PORT));
    _aux1VideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_A1L_CAMERA_PORT));
    _aux1VideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_A1R_CAMERA_PORT));
    _monoVideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_ML_CAMERA_PORT));
    _monoVideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_MR_CAMERA_PORT));

    LOG_I(LOG_TAG, "***************Initializing Audio system******************");

    _audioClient = new AudioClient(MEDIAID_AUDIO, SocketAddress(_settings.roverAddress, NETWORK_ALL_AUDIO_PORT), QHostAddress::Any, this);
    // forward audio stream through localhost
    _audioClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_AUDIO_PORT));
    connect(_audioClient, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
            this, SLOT(audioClientStateChanged(MediaClient*,MediaClient::State)));

    _audioPlayer = new Soro::Gst::AudioPlayer(this);

    // Show UI's

    _mainUi->show();
    _controlUi->setVisible(true);

    // Start timers

    START_TIMER(_bitrateUpdateTimerId, 1000);
    START_TIMER(_pingTimerId, 1000);
}

void ResearchControlProcess::startTestLog() {
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    _mbedParser.startLog(QCoreApplication::applicationDirPath() + "/../research-data/sensors/" + QString::number(startTime));
    _gpsLogger.startLog(QCoreApplication::applicationDirPath() + "/../research-data/gps/" + QString::number(startTime));
}

void ResearchControlProcess::stopTestLog() {
    _mbedParser.stopLog();
    _gpsLogger.stopLog();
}

void ResearchControlProcess::gamepadChanged(SDL_GameController *controller, QString name) {
    Q_UNUSED(controller);
    _controlUi->setProperty("gamepad", name);
    if (controller) {
        QMetaObject::invokeMethod(_controlUi,
                                  "notify",
                                  Q_ARG(QVariant,"information"),
                                  Q_ARG(QVariant, "Input Device Connected"),
                                  Q_ARG(QVariant, name + " is connected and ready for use."));
    }
    else {
        QMetaObject::invokeMethod(_controlUi,
                                  "notify",
                                  Q_ARG(QVariant,"warning"),
                                  Q_ARG(QVariant, "No Input Device"),
                                  Q_ARG(QVariant, "Driving will be unavailable until a compatible controller is connected."));
    }
}

void ResearchControlProcess::ui_requestUiSync() {
    // Sync state
    updateUiConnectionState();
    // Sync settings
    _settings.syncUi(_controlUi);
}

void ResearchControlProcess::ui_settingsApplied() {
    _settings.syncModel(_controlUi);

    if (_settings.enableVideo) {
        VideoFormat format = _settings.getSelectedVideoFormat();
        if (format.isUseable()) {
            if (_settings.selectedCamera == _settings.mainCameraIndex) {
                if (_settings.enableStereoUi) {
                    if (_settings.enableStereoVideo) {
                        // Stream main camera in stereo
                        format.setStereoMode(DEFAULT_VIDEO_STEREO_MODE);
                        startStereoCameraStream(format);
                    }
                    else {
                        // Stream main camera in mono on stereo UI
                        format.setStereoMode(DEFAULT_VIDEO_STEREO_MODE);
                        startMonoCameraStream(format);
                    }
                }
                else {
                    _settings.enableStereoVideo = false; // Just in case the UI let them
                    // Stream main camera in mono on mono UI
                    format.setStereoMode(VideoFormat::StereoMode_None);
                    startMonoCameraStream(format);
                }
            }
            else if (_settings.selectedCamera == _settings.aux1CameraIndex) {
                _settings.enableStereoVideo = false;
                if (_settings.enableStereoUi) {
                    // Stream aux1 camera in mono on stereo UI
                    format.setStereoMode(DEFAULT_VIDEO_STEREO_MODE);
                    startAux1CameraStream(format);
                }
                else {
                    // Stream aux1 camera in mono on mono UI
                    format.setStereoMode(VideoFormat::StereoMode_None);
                    startAux1CameraStream(format);
                }
            }
            else {
                LOG_E(LOG_TAG, "Unknown camera index selected in UI");
                _settings.enableVideo = false;
                _settings.selectedCamera = 0;
                _settings.selectedVideoFormat = 0;
            }
        }
        else {
            LOG_E(LOG_TAG, "Unknown video format index selected in UI");
            _settings.enableVideo = false;
            _settings.selectedCamera = 0;
            _settings.selectedVideoFormat = 0;
        }
    }
    else {
        stopAllRoverCameras();
    }
    if (_settings.enableHud) {
        //TODO
    }
    else {
        //TODO
    }
    if (_settings.enableAudio) {
        startAudioStream(_settings.defaultAudioFormat);
    }
    else {
        stopAudio();
    }
    _driveSystem->getChannel()->setSimulatedDelay(_settings.selectedLatency);
}

void ResearchControlProcess::videoClientStateChanged(MediaClient *client, MediaClient::State state) {
    if ((client == _stereoLVideoClient) || (client == _stereoRVideoClient)) {
        if ((_stereoLVideoClient->getState() == MediaClient::StreamingState) &&
                (_stereoRVideoClient->getState() == MediaClient::StreamingState)) {
            // Only play the stream once both left and right cameras are streaming
            _mainUi->getCameraWidget()->playStereo(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_SL_CAMERA_PORT),
                                               _stereoLVideoClient->getVideoFormat(),
                                               SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_SR_CAMERA_PORT),
                                               _stereoRVideoClient->getVideoFormat());
            if (!_settings.enableStereoUi || !_settings.enableStereoVideo) {
                LOG_E(LOG_TAG, "Video clients are playing stereo, but UI is not in stereo mode");
                _settings.enableStereoUi = true;
                _settings.enableStereoVideo = true;
                _settings.syncUi(_controlUi);
            }
        }
    }
    else if ((client == _aux1VideoClient) && (_aux1VideoClient->getState() == MediaClient::StreamingState)) {
        if (_settings.enableStereoUi) {
            _mainUi->getCameraWidget()->playStereo(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_A1L_CAMERA_PORT),
                                             _aux1VideoClient->getVideoFormat(),
                                             SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_A1R_CAMERA_PORT),
                                             _aux1VideoClient->getVideoFormat());
        }
        else {
            _mainUi->getCameraWidget()->playMono(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_A1L_CAMERA_PORT),
                                             _aux1VideoClient->getVideoFormat());
        }
    }
    else if ((client == _monoVideoClient) && (_monoVideoClient->getState() == MediaClient::StreamingState)) {
        if (_settings.enableStereoUi) {
            _mainUi->getCameraWidget()->playStereo(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_ML_CAMERA_PORT),
                                             _monoVideoClient->getVideoFormat(),
                                             SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_MR_CAMERA_PORT),
                                             _monoVideoClient->getVideoFormat());
        }
        else {
            _mainUi->getCameraWidget()->playMono(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_ML_CAMERA_PORT),
                                             _monoVideoClient->getVideoFormat());
        }
    }

    if (state == MediaClient::StreamingState) {
        _settings.enableVideo = true;
        _settings.setSelectedCamera(client->getMediaId());
        _settings.syncUi(_controlUi);
    }
    else if ((_stereoLVideoClient->getState() != MediaClient::StreamingState) &&
             (_stereoRVideoClient->getState() != MediaClient::StreamingState) &&
             (_monoVideoClient->getState() != MediaClient::StreamingState) &&
             (_aux1VideoClient->getState() != MediaClient::StreamingState)) {
        // No cameras streaming
        _settings.enableVideo = false;
        _settings.syncUi(_controlUi);
    }
}

void ResearchControlProcess::audioClientStateChanged(MediaClient *client, MediaClient::State state) {
    Q_UNUSED(client);

    switch (state) {
    case AudioClient::StreamingState:
        _audioPlayer->play(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_AUDIO_PORT),
                           _audioClient->getAudioFormat());
        _settings.enableAudio = true;
        _settings.syncUi(_controlUi);
        break;
    case AudioClient::ConnectingState:
        _audioPlayer->stop();
        _settings.enableAudio = false;
        _settings.syncUi(_controlUi);
        break;
    default:
        break;
    }
}

void ResearchControlProcess::updateUiConnectionState() {
    switch (_roverChannel->getState()) {
    case Channel::ErrorState:
        _controlUi->setProperty("state", "error");
        QMetaObject::invokeMethod(_controlUi,
                                  "notify",
                                  Q_ARG(QVariant,"error"),
                                  Q_ARG(QVariant, "Control Channel Error"),
                                  Q_ARG(QVariant, "An unrecoverable netowork error occurred. Please exit and check the log."));
        break;
    case Channel::ConnectedState:
        _controlUi->setProperty("state", "connected");
        break;
    default:
        _controlUi->setProperty("state", "connecting");
        break;
    }
}

void ResearchControlProcess::newSensorData(MbedDataParser::DataTag tag, float value) {
    //TODO
}

void ResearchControlProcess::timerEvent(QTimerEvent *e) {
    if (e->timerId() == _pingTimerId) {
        /****************************************
         * This timer runs regularly to update the
         * ping statistic
         */
        QMetaObject::invokeMethod(_controlUi,
                                  "updatePing",
                                  Q_ARG(QVariant, _driveSystem->getChannel()->getLastRtt()));
        if (_roverChannel->getLastRtt() > 1000) {
            // The REAL ping is over 1 second
            QMetaObject::invokeMethod(_controlUi,
                                      "notify",
                                      Q_ARG(QVariant,"warning"),
                                      Q_ARG(QVariant, "Ping Warning"),
                                      Q_ARG(QVariant, "Actual (non-simulated) ping is over 1 second."));
        }
    }
    else if (e->timerId() == _bitrateUpdateTimerId) {
        /*****************************************
         * This timer regularly updates the total bitrate count,
         * and also broadcasts it to slave mission controls since they
         * cannot calculate video bitrate
         */
        quint64 bpsRoverDown = 0, bpsRoverUp = 0;
        bpsRoverUp += _monoVideoClient->getBitrate();
        bpsRoverUp += _stereoLVideoClient->getBitrate();
        bpsRoverUp += _stereoRVideoClient->getBitrate();
        bpsRoverUp += _aux1VideoClient->getBitrate();
        bpsRoverUp += _audioClient->getBitrate();
        bpsRoverUp += _roverChannel->getBitsPerSecondDown();
        bpsRoverDown += _roverChannel->getBitsPerSecondUp();
        bpsRoverUp += _driveSystem->getChannel()->getBitsPerSecondDown();
        bpsRoverDown += _driveSystem->getChannel()->getBitsPerSecondUp();

        QMetaObject::invokeMethod(_controlUi,
                                  "updateBitrate",
                                  Q_ARG(QVariant, bpsRoverUp),
                                  Q_ARG(QVariant, bpsRoverDown));
    }
    else {
        QObject::timerEvent(e);
    }
}

void ResearchControlProcess::roverSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);

    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    SharedMessageType messageType;

    LOG_D(LOG_TAG, "Getting shared channel message");

    stream >> reinterpret_cast<quint32&>(messageType);
    switch (messageType) {
    case SharedMessage_RoverStatusUpdate: {
        bool driveNormal;
        stream >> driveNormal;
        if (!driveNormal) {
            QMetaObject::invokeMethod(_controlUi,
                                      "notify",
                                      Q_ARG(QVariant, "error"),
                                      Q_ARG(QVariant, "Mbed Error"),
                                      Q_ARG(QVariant, "The rover has lost connection to its mbed. Driving and data recording will no longer work."));
            _controlUi->setProperty("driveStatus", "Mbed Error");
        }
    }
        break;
    case SharedMessage_RoverMediaServerError: {
        qint32 mediaId;
        QString error;
        stream >> mediaId;
        stream >> error;

        if (mediaId == _audioClient->getMediaId()) {
            QMetaObject::invokeMethod(_controlUi,
                                      "notify",
                                      Q_ARG(QVariant, "warning"),
                                      Q_ARG(QVariant, "Audio Stream Error"),
                                      Q_ARG(QVariant, "The rover encountered an error trying to stream audio."));
            LOG_E(LOG_TAG, "Audio streaming error: " + error);
        }
        else {
            QMetaObject::invokeMethod(_controlUi,
                                      "notify",
                                      Q_ARG(QVariant, "warning"),
                                      Q_ARG(QVariant, "Video Stream Error"),
                                      Q_ARG(QVariant, "The rover encountered an error trying to stream this camera."));
            LOG_E(LOG_TAG, "Streaming error on camera " + QString::number(mediaId) + ": " + error);
        }
    }
        break;
    case SharedMessage_RoverGpsUpdate: {
        NmeaMessage location;
        stream >> location;
        // Forward to UI
        QMetaObject::invokeMethod(_controlUi,
                                  "updateGpsLocation",
                                  Q_ARG(QVariant, location.Latitude),
                                  Q_ARG(QVariant, location.Longitude),
                                  Q_ARG(QVariant, location.Heading));

        // Forward to logger
        _gpsLogger.addLocation(location);
    }
        break;
    case SharedMessage_Research_RoverDriveOverrideStart:
        QMetaObject::invokeMethod(_controlUi,
                                  "notify",
                                  Q_ARG(QVariant, "information"),
                                  Q_ARG(QVariant, "Network Driving Disabled"),
                                  Q_ARG(QVariant, "The rover is being driven by serial override. Network drive commands will not be accepted."));
        _controlUi->setProperty("driveStatus", "Serial Override");
        break;
    case SharedMessage_Research_RoverDriveOverrideEnd:
        QMetaObject::invokeMethod(_controlUi,
                                  "notify",
                                  Q_ARG(QVariant, "information"),
                                  Q_ARG(QVariant, "Network Driving Enabled"),
                                  Q_ARG(QVariant, "The rover has resumed accepting network drive commands."));
        _controlUi->setProperty("driveStatus", "Operational");
        break;
    case SharedMessage_Research_SensorUpdate: {
        QByteArray data;
        stream >> data;
        // This raw data should be sent to an MbedParser to be decoded
        _mbedParser.newData(data.data(), data.length());
        break;
    }
    default:
        LOG_E(LOG_TAG, "Got unknown message header on shared channel");
        break;
    }
}

void ResearchControlProcess::driveConnectionStateChanged(Channel::State state) {
    switch (state) {
    case Channel::ErrorState:
        _controlUi->setProperty("status", "error");
        QMetaObject::invokeMethod(_controlUi,
                                  "notify",
                                  Q_ARG(QVariant,"error"),
                                  Q_ARG(QVariant, "Drive Channel Error"),
                                  Q_ARG(QVariant, "An unrecoverable netowork error occurred. Please exit and check the log."));
        _controlUi->setProperty("driveStatus", "Network Error");
        break;
    case Channel::ConnectedState:
        QMetaObject::invokeMethod(_controlUi,
                                  "notify",
                                  Q_ARG(QVariant,"information"),
                                  Q_ARG(QVariant, "Drive Channel Connected"),
                                  Q_ARG(QVariant, "You are now connected to the rover's drive system."));
        _controlUi->setProperty("driveStatus", "Operational");
        break;
    default:
        if (_driveSystem->getChannel()->wasConnected()) {
            QMetaObject::invokeMethod(_controlUi,
                                      "notify",
                                      Q_ARG(QVariant,"error"),
                                      Q_ARG(QVariant, "Drive Channel Disconnected"),
                                      Q_ARG(QVariant, "Although you are connected to the rover, the network connection to it's drive subsystem has been lost."));
            _controlUi->setProperty("driveStatus", "Network Disconnected");
        }
        break;
    }
}

void ResearchControlProcess::stopAllRoverCameras() {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_Research_StopAllCameraStreams;

    _mainUi->getCameraWidget()->stop(_settings.enableStereoUi);
    stream << reinterpret_cast<const quint32&>(messageType);
    _roverChannel->sendMessage(message);
}

void ResearchControlProcess::startMonoCameraStream(VideoFormat format) {
    stopAllRoverCameras();

    if (format.isUseable()) {
        // Start mono stream
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        SharedMessageType messageType = SharedMessage_Research_StartMonoCameraStream;
        stream << reinterpret_cast<const quint32&>(messageType);
        stream << format.serialize();

        _roverChannel->sendMessage(message);
    }
    else {
        LOG_E(LOG_TAG, "startMonoCameraStream(): This format is not useable. If you want to stop this camera, call stopAllRoverCameras() instead");
    }
}

void ResearchControlProcess::startStereoCameraStream(VideoFormat format) {
    stopAllRoverCameras();

    if (format.isUseable()) {
        // Start stereo stream
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        SharedMessageType messageType = SharedMessage_Research_StartStereoCameraStream;
        stream << reinterpret_cast<const quint32&>(messageType);
        stream << format.serialize();

        _roverChannel->sendMessage(message);
    }
    else {
        LOG_E(LOG_TAG, "startStereoCameraStream(): This format is not useable. If you want to stop this camera, call stopAllRoverCameras() instead");
    }
}

void ResearchControlProcess::startAux1CameraStream(VideoFormat format) {
    stopAllRoverCameras();

    if (format.isUseable()) {
        // Start stereo stream
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        SharedMessageType messageType = SharedMessage_Research_StartAux1CameraStream;
        stream << reinterpret_cast<const quint32&>(messageType);
        stream << format.serialize();

        _roverChannel->sendMessage(message);
    }
    else {
        LOG_E(LOG_TAG, "startAux1CameraStream(): This format is not useable. If you want to stop this camera, call stopAllRoverCameras() instead");
    }
}

void ResearchControlProcess::stopAudio() {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_RequestDeactivateAudioStream;

    stream << reinterpret_cast<const quint32&>(messageType);
    _roverChannel->sendMessage(message);
}

void ResearchControlProcess::startAudioStream(AudioFormat format) {
    if (format.isUseable()) {
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        SharedMessageType messageType;
        messageType = SharedMessage_RequestActivateAudioStream;

        stream << reinterpret_cast<const quint32&>(messageType);
        stream << format.serialize();
        _roverChannel->sendMessage(message);
    }
    else {
        LOG_E(LOG_TAG, "startAudioStream(): This format is not useable. If you want to stop the audio stream, call stopAudio() instead");
    }
}

ResearchControlProcess::~ResearchControlProcess() {
    if (_mainUi) {
        delete _mainUi;
    }
    if (_roverChannel) {
        disconnect(_roverChannel, 0, 0, 0);
        delete _roverChannel;
    }
    if (_driveSystem) {
        disconnect(_driveSystem, 0, 0, 0);
        _driveSystem->disable();
        delete _driveSystem;
    }
}

} // namespace MissionControl
} // namespace Soro

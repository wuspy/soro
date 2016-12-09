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
    connect(_gamepad, SIGNAL(poll()),
            this, SLOT(gamepadPoll()));
    _roverAddress = roverAddress;

    // Create UI for rover control
    _mainUI = new ResearchMainWindow();
    connect(_mainUI, SIGNAL(closed()), this, SIGNAL(windowClosed()));
    _mainUI->show();

    // Create UI for settings
    _settingsUI = new SettingsForm(qml);

    LOG_I(LOG_TAG, "****************Initializing connections*******************");

    LOG_I(LOG_TAG, "Setting up rover shared connection");
    // Create the main shared channel to connect to the rover
    _roverChannel = Channel::createClient(this, SocketAddress(_roverAddress, NETWORK_ALL_SHARED_CHANNEL_PORT), CHANNEL_NAME_SHARED,
            Channel::TcpProtocol, QHostAddress::Any);
    _roverChannel->open();
    connect(_roverChannel, SIGNAL(messageReceived(Channel*,const char*,Channel::MessageSize)),
            this, SLOT(roverSharedChannelMessageReceived(Channel*,const char*,Channel::MessageSize)));
    connect(_roverChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
            this, SLOT(roverSharedChannelStateChanged(Channel*,Channel::State)));

    LOG_I(LOG_TAG, "Creating drive control system");
    _driveSystem = new DriveControlSystem(_roverAddress, _gamepad, this);
    _driveSystem->enable();

    LOG_I(LOG_TAG, "***************Initializing Video system******************");

    _stereoLVideoClient = new VideoClient(MEDIAID_RESEARCH_SL_CAMERA, SocketAddress(_roverAddress, NETWORK_ALL_RESEARCH_SL_CAMERA_PORT), QHostAddress::Any, this);
    _stereoRVideoClient = new VideoClient(MEDIAID_RESEARCH_SR_CAMERA, SocketAddress(_roverAddress, NETWORK_ALL_RESEARCH_SR_CAMERA_PORT), QHostAddress::Any, this);
    _aux1VideoClient = new VideoClient(MEDIAID_RESEARCH_A1_CAMERA, SocketAddress(_roverAddress, NETWORK_ALL_RESEARCH_A1_CAMERA_PORT), QHostAddress::Any, this);
    _monoVideoClient = new VideoClient(MEDIAID_RESEARCH_M_CAMERA, SocketAddress(_roverAddress, NETWORK_ALL_RESEARCH_M_CAMERA_PORT), QHostAddress::Any, this);

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
    _aux1VideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_A1_CAMERA_PORT));
    _monoVideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_M_CAMERA_PORT));

    LOG_I(LOG_TAG, "***************Initializing Audio system******************");

    _audioClient = new AudioClient(MEDIAID_AUDIO, SocketAddress(_roverAddress, NETWORK_ALL_AUDIO_PORT), QHostAddress::Any, this);
    // forward audio stream through localhost
    _audioClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_AUDIO_PORT));
    connect(_audioClient, SIGNAL(stateChanged(MediaClient*,MediaClient::State)),
            this, SLOT(audioClientStateChanged(MediaClient*,MediaClient::State)));

    _audioPlayer = new Soro::Gst::AudioPlayer(this);
}

void ResearchControlProcess::videoClientStateChanged(MediaClient *client, MediaClient::State state) {
    if ((client == _stereoLVideoClient) || (client == _stereoRVideoClient)) {
        if ((_stereoLVideoClient->getState() == MediaClient::StreamingState) &&
                (_stereoRVideoClient->getState() == MediaClient::StreamingState)) {
            // Only play the stream once both left and right cameras are streaming
            _mainUI->getCameraWidget()->playStereo(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_SL_CAMERA_PORT),
                                               _stereoLVideoClient->getVideoFormat(),
                                               SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_SR_CAMERA_PORT),
                                               _stereoRVideoClient->getVideoFormat());
        }
    }
    else if ((client == _aux1VideoClient) && (_aux1VideoClient->getState() == MediaClient::StreamingState)) {
        _mainUI->getCameraWidget()->playMono(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_A1_CAMERA_PORT),
                                         _aux1VideoClient->getVideoFormat());
    }
    else if ((client == _monoVideoClient) && (_monoVideoClient->getState() == MediaClient::StreamingState)) {
        _mainUI->getCameraWidget()->playMono(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_RESEARCH_M_CAMERA_PORT),
                                         _aux1VideoClient->getVideoFormat());
    }
}

void ResearchControlProcess::audioClientStateChanged(MediaClient *client, MediaClient::State state) {
    Q_UNUSED(client);

    switch (state) {
    case AudioClient::StreamingState:
        _audioPlayer->play(SocketAddress(QHostAddress::LocalHost, NETWORK_ALL_AUDIO_PORT),
                           _audioClient->getAudioFormat());
        break;
    default:
        _audioPlayer->stop();
        break;
    }
}

void ResearchControlProcess::roverSharedChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    //TODO
}

void ResearchControlProcess::gamepadPoll() {

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
        bool driveCameraNormal;
        stream >> driveCameraNormal;
        //TODO
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
        NmeaMessage message;
        stream >> message;
        //TODO
    }
    default:
        LOG_E(LOG_TAG, "Got unknown message header on shared channel");
        break;
    }
}

void ResearchControlProcess::stopAllRoverCameras() {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_Research_EndStereoAndMonoCameraStream;

    _mainUI->getCameraWidget()->stop();
    stream << reinterpret_cast<const quint32&>(messageType);
    _roverChannel->sendMessage(message);
}

void ResearchControlProcess::ui_monoCameraFormatSelected(VideoFormat format) {
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
        LOG_E(LOG_TAG, "ui_monoCameraFormatSelected(): This format is not useable. If you want to stop this camera, call stopAllRoverCameras() instead");
    }
}

void ResearchControlProcess::ui_stereoCameraFormatSelected(VideoFormat format) {
    stopAllRoverCameras();

    if (format.isUseable()) {
        // Set the stereo option on the format
        format.setStereoMode(DEFAULT_VIDEO_STEREO_MODE);
        // Start stereo stream
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        SharedMessageType messageType = SharedMessage_Research_StartStereoCameraStream;
        stream << reinterpret_cast<const quint32&>(messageType);
        stream << format.serialize();

        _roverChannel->sendMessage(message);
    }
    else {
        LOG_E(LOG_TAG, "ui_stereoCameraFormatSelected(): This format is not useable. If you want to stop this camera, call stopAllRoverCameras() instead");
    }
}

void ResearchControlProcess::ui_aux1CameraFormatSelected(VideoFormat format) {
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
        LOG_E(LOG_TAG, "ui_aux1CameraFormatSelected(): This format is not useable. If you want to stop this camera, call stopAllRoverCameras() instead");
    }
}

void ResearchControlProcess::ui_audioStreamFormatSelected(AudioFormat format) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType;

    if (format.isUseable()) {
        messageType = SharedMessage_RequestActivateAudioStream;
        stream << reinterpret_cast<const quint32&>(messageType);
        stream << format.serialize();
    }
    else {
        messageType = SharedMessage_RequestDeactivateAudioStream;
        stream << reinterpret_cast<const quint32&>(messageType);
    }
    _roverChannel->sendMessage(message);
}

ResearchControlProcess::~ResearchControlProcess() {
    if (_mainUI) {
        delete _mainUI;
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

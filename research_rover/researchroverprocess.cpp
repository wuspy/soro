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

#include "researchroverprocess.h"
#include "libsoro/logger.h"
#include "libsoro/confloader.h"
#include "libsoro/usbcameraenumerator.h"

#define LOG_TAG "ResearchRover"

#define DEFAULT_AUDIO_DEVICE "hw:1"

namespace Soro {
namespace Rover {

ResearchRoverProcess::ResearchRoverProcess(QObject *parent) : QObject(parent)
{
    // Must initialize once the event loop has started.
    // This can be accomplished using a single shot timer.
    QTimer::singleShot(1, this, SLOT(init()));
}

void ResearchRoverProcess::init() {
    LOG_I(LOG_TAG, "*****************Loading Configuration*******************");


    LOG_I(LOG_TAG, "*************Initializing core networking*****************");

    _driveChannel = Channel::createServer(this, NETWORK_ALL_DRIVE_CHANNEL_PORT, CHANNEL_NAME_DRIVE,
                              Channel::UdpProtocol, QHostAddress::Any);
    _sharedChannel = Channel::createServer(this, NETWORK_ALL_SHARED_CHANNEL_PORT, CHANNEL_NAME_SHARED,
                              Channel::TcpProtocol, QHostAddress::Any);

    if (_driveChannel->getState() == Channel::ErrorState) {
        LOG_E(LOG_TAG, "The drive channel experienced a fatal error during initialization");
        exit(1); return;
    }
    if (_sharedChannel->getState() == Channel::ErrorState) {
        LOG_E(LOG_TAG, "The shared channel experienced a fatal error during initialization");
        exit(1); return;
    }

    _driveChannel->open();
    _sharedChannel->open();

    // observers for network channel connectivity changes
    connect(_sharedChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
            this, SLOT(sharedChannelStateChanged(Channel*,Channel::State)));
    connect(_driveChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
            this, SLOT(driveChannelStateChanged(Channel*,Channel::State)));


    LOG_I(LOG_TAG, "All network channels initialized successfully");

    LOG_I(LOG_TAG, "*****************Initializing MBED systems*******************");

    // create mbed channels
    _mbed = new MbedChannel(SocketAddress(QHostAddress::Any, NETWORK_ROVER_RESEARCH_MBED_PORT), MBED_ID_RESEARCH, this);

    // setup mbed data parser/logger
    _mbedParser = new MbedDataParser(_mbed, this);
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

    // observers for mbed events
    connect(_mbed, SIGNAL(messageReceived(MbedChannel*,const char*,int)),
            this, SLOT( mbedMessageReceived(MbedChannel*,const char*,int)));
    connect(_mbed, SIGNAL(stateChanged(MbedChannel*,MbedChannel::State)),
            this, SLOT(mbedChannelStateChanged(MbedChannel*,MbedChannel::State)));

    // observers for network channels message received
    connect(_driveChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
             this, SLOT(driveChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));
    connect(_sharedChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
             this, SLOT(sharedChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));

    LOG_I(LOG_TAG, "*****************Initializing GPS system*******************");

    _gpsServer = new GpsServer(SocketAddress(QHostAddress::Any, NETWORK_ROVER_GPS_PORT), this);
    connect(_gpsServer, SIGNAL(gpsUpdate(NmeaMessage)),
            this, SLOT(gpsUpdate(NmeaMessage)));

    _gpsLogger = new GpsLogger(this);
    connect(_gpsServer, SIGNAL(gpsUpdate(NmeaMessage)),
            _gpsLogger, SLOT(addLocation(NmeaMessage)));

    LOG_I(LOG_TAG, "*****************Initializing Video system*******************");

    _stereoRCameraServer = new VideoServer(MEDIAID_RESEARCH_SR_CAMERA, SocketAddress(QHostAddress::Any, NETWORK_ALL_RESEARCH_SR_CAMERA_PORT), this);
    _stereoLCameraServer = new VideoServer(MEDIAID_RESEARCH_SL_CAMERA, SocketAddress(QHostAddress::Any, NETWORK_ALL_RESEARCH_SL_CAMERA_PORT), this);
    _aux1CameraServer = new VideoServer(MEDIAID_RESEARCH_A1_CAMERA, SocketAddress(QHostAddress::Any, NETWORK_ALL_RESEARCH_A1L_CAMERA_PORT), this);
    _monoCameraServer = new VideoServer(MEDIAID_RESEARCH_M_CAMERA, SocketAddress(QHostAddress::Any, NETWORK_ALL_RESEARCH_ML_CAMERA_PORT), this);

    connect(_stereoRCameraServer, SIGNAL(error(MediaServer*,QString)),
            this, SLOT(mediaServerError(MediaServer*,QString)));
    connect(_stereoLCameraServer, SIGNAL(error(MediaServer*,QString)),
            this, SLOT(mediaServerError(MediaServer*,QString)));
    connect(_aux1CameraServer, SIGNAL(error(MediaServer*,QString)),
            this, SLOT(mediaServerError(MediaServer*,QString)));
    connect(_monoCameraServer, SIGNAL(error(MediaServer*,QString)),
            this, SLOT(mediaServerError(MediaServer*,QString)));

    UsbCameraEnumerator cameras;
    cameras.loadCameras();

    QFile camFile(QCoreApplication::applicationDirPath() + "/../config/research_cameras.conf");
    if (!camFile.exists()) {
        LOG_E(LOG_TAG, "The camera configuration file ../config/research_cameras.conf does not exist. Video will not work.");
    }
    else {
        ConfLoader camConfig;
        camConfig.load(camFile);

        const UsbCamera* stereoRight = cameras.find(camConfig.value("sr_matchName"),
                                        camConfig.value("sr_matchDevice"),
                                        camConfig.value("sr_matchVendorId"),
                                        camConfig.value("sr_matchProductId"),
                                        camConfig.value("sr_matchSerial"));

        const UsbCamera* stereoLeft = cameras.find(camConfig.value("sr_matchName"),
                                        camConfig.value("sl_matchDevice"),
                                        camConfig.value("sl_matchVendorId"),
                                        camConfig.value("sl_matchProductId"),
                                        camConfig.value("sl_matchSerial"));

        const UsbCamera* aux1 = cameras.find(camConfig.value("a1_matchName"),
                                        camConfig.value("a1_matchDevice"),
                                        camConfig.value("a1_matchVendorId"),
                                        camConfig.value("a1_matchProductId"),
                                        camConfig.value("a1_matchSerial"));

        if (stereoRight) {
            _stereoRCameraDevice = stereoRight->device;
            _monoCameraDevice = _stereoRCameraDevice;
            LOG_I(LOG_TAG, "Right stereo camera found: " + stereoRight->toString());
        }
        else {
            LOG_E(LOG_TAG, "Right stereo camera couldn't be found using provided definition.");
        }
        if (stereoLeft) {
            _stereoLCameraDevice = stereoLeft->device;
            if (!stereoRight) {
                _monoCameraDevice = _stereoLCameraDevice;
            }
            LOG_I(LOG_TAG, "Left stereo camera found: " + stereoLeft->toString());
        }
        else {
            LOG_E(LOG_TAG, "Left stereo camera couldn't be found using provided definition.");
        }
        if (aux1) {
            _aux1CameraDevice = aux1->device;
            LOG_I(LOG_TAG, "Aux1 camera found: " + aux1->toString());
        }
        else {
            LOG_E(LOG_TAG, "Aux1 camera couldn't be found using provided definition.");
        }

    }


    LOG_I(LOG_TAG, "*****************Initializing Audio system*******************");

    _audioServer = new AudioServer(MEDIAID_AUDIO, SocketAddress(QHostAddress::Any, NETWORK_ALL_AUDIO_PORT), this);

    connect(_audioServer, SIGNAL(error(MediaServer*,QString)),
            this, SLOT(mediaServerError(MediaServer*,QString)));

    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "Initialization complete");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
    LOG_I(LOG_TAG, "-------------------------------------------------------");
}


void ResearchRoverProcess::sharedChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    if (state == Channel::ConnectedState) {
        // send all status information since we just connected
        // TODO there is an implementation bug where a Channel will not send messages immediately after it connects
        QTimer::singleShot(1000, this, SLOT(sendSystemStatusMessage()));
    }
}

void ResearchRoverProcess::sendSystemStatusMessage() {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    SharedMessageType messageType = SharedMessage_Research_RoverStatusUpdate;
    bool driveState = _mbed->getState() == MbedChannel::ConnectedState;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << driveState;
    _sharedChannel->sendMessage(message);
}

void ResearchRoverProcess::driveChannelStateChanged(Channel* channel, Channel::State state) {
    //TODO
}

void ResearchRoverProcess::mbedChannelStateChanged(MbedChannel* channel, MbedChannel::State state) {
    Q_UNUSED(channel); Q_UNUSED(state);
    sendSystemStatusMessage();
}

void ResearchRoverProcess::driveChannelMessageReceived(Channel* channel, const char* message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    char header = message[0];
    MbedMessageType messageType;
    reinterpret_cast<quint32&>(messageType) = (quint32)reinterpret_cast<unsigned char&>(header);
    switch (messageType) {
    case MbedMessage_Drive:
        _mbed->sendMessage(message, (int)size);
        break;
    default:
        LOG_E(LOG_TAG, "Received invalid message from mission control on drive control channel");
        break;
    }
}

void ResearchRoverProcess::mediaServerError(MediaServer *server, QString message) {
    QByteArray byeArray;
    QDataStream stream(&byeArray, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    SharedMessageType messageType = SharedMessage_RoverMediaServerError;

    stream <<(qint32)server->getMediaId();
    stream << message;
    _sharedChannel->sendMessage(byeArray);
}

void ResearchRoverProcess::sharedChannelMessageReceived(Channel* channel, const char* message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    SharedMessageType messageType;

    stream >> reinterpret_cast<quint32&>(messageType);
    switch (messageType) {
    case SharedMessage_RequestActivateAudioStream: {
        QString formatString;
        AudioFormat format;
        stream >> formatString;
        format.deserialize(formatString);
        _audioServer->start(DEFAULT_AUDIO_DEVICE, format);
    }
        break;
    case SharedMessage_RequestDeactivateAudioStream:
        _audioServer->stop();
        break;
    case SharedMessage_Research_StartStereoCameraStream: {
        QString formatString;
        VideoFormat format;
        stream >> formatString;
        format.deserialize(formatString);
        if (!_stereoRCameraDevice.isEmpty()) {
            _stereoRCameraServer->start(_stereoRCameraDevice, format);
        }
        if (!_stereoLCameraDevice.isEmpty()) {
            _stereoLCameraServer->start(_stereoLCameraDevice, format);
        }
    }
        break;
    case SharedMessage_Research_EndStereoAndMonoCameraStream:
        _stereoRCameraServer->stop();
        _stereoLCameraServer->stop();
        _monoCameraServer->stop();
        break;
    case SharedMessage_Research_StopAllCameraStreams:
        _stereoRCameraServer->stop();
        _stereoLCameraServer->stop();
        _monoCameraServer->stop();
        _aux1CameraServer->stop();
        break;
    case SharedMessage_Research_StartMonoCameraStream: {
        QString formatString;
        VideoFormat format;
        stream >> formatString;
        format.deserialize(formatString);
        if (!_monoCameraDevice.isEmpty()) {
            _monoCameraServer->start(_monoCameraDevice, format);
        }
    }
        break;
    case SharedMessage_Research_StartAux1CameraStream:{
        QString formatString;
        VideoFormat format;
        stream >> formatString;
        format.deserialize(formatString);
        if (!_aux1CameraDevice.isEmpty()) {
            _aux1CameraServer->start(_aux1CameraDevice, format);
        }
    }
        break;
    case SharedMessage_Research_EndAux1CameraStream:
        _aux1CameraServer->stop();
        break;
    case SharedMessage_Research_TestStart: {
        QDateTime startTime;
        stream >> startTime;
        LOG_I(LOG_TAG, "Starting test log with start time of " + QString::number(startTime.toMSecsSinceEpoch()));

        _mbedParser->startLog(
                    QCoreApplication::applicationDirPath() + "/../research-data/sensors/" + QString::number(startTime.toMSecsSinceEpoch()),
                    startTime);

        _gpsLogger->startLog(
                    QCoreApplication::applicationDirPath() + "/../research-data/gps/" + QString::number(startTime.toMSecsSinceEpoch()),
                    startTime);
    }
        break;
    case SharedMessage_Research_TestEnd:
        LOG_I(LOG_TAG, "Ending test log");

        _mbedParser->stopLog();
        _gpsLogger->stopLog();
        break;
    default:
        LOG_W(LOG_TAG, "Got unknown shared channel message");
        break;
    }
}

void ResearchRoverProcess::mbedMessageReceived(MbedChannel* channel, const char* message, int size) {
    // Forward the message to mission control (MbedDataParser instance will take care of logging it)

    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_Research_SensorUpdate;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << QByteArray(message, size);

    _sharedChannel->sendMessage(byteArray);
}

void ResearchRoverProcess::gpsUpdate(NmeaMessage message) {
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_RoverGpsUpdate;
    stream.setByteOrder(QDataStream::BigEndian);

    stream << reinterpret_cast<quint32&>(messageType);
    stream << message;

    _sharedChannel->sendMessage(byteArray);
}

ResearchRoverProcess::~ResearchRoverProcess() {
    if (_driveChannel) {
        disconnect(_driveChannel, 0, 0, 0);
        delete _driveChannel;
    }
    if (_sharedChannel) {
        disconnect(_sharedChannel, 0, 0, 0);
        delete _sharedChannel;
    }
    if (_gpsLogger) {
        disconnect(_gpsLogger, 0, 0, 0);
        delete _gpsLogger;
    }
    if (_gpsServer) {
        disconnect(_gpsServer, 0, 0, 0);
        delete _gpsServer;
    }
    if (_audioServer) {
        disconnect(_audioServer, 0, 0, 0);
        delete _audioServer;
    }
    if (_stereoRCameraServer) {
        disconnect(_stereoRCameraServer, 0, 0, 0);
        delete _stereoRCameraServer;
    }
    if (_stereoLCameraServer) {
        disconnect(_stereoLCameraServer, 0, 0, 0);
        delete _stereoLCameraServer;
    }
    if (_aux1CameraServer) {
        disconnect(_aux1CameraServer, 0, 0, 0);
        delete _aux1CameraServer;
    }
    if (_mbedParser) {
        disconnect(_mbedParser, 0, 0, 0);
        delete _mbedParser;
    }
    if (_mbed) {
        disconnect(_mbed, 0, 0, 0);
        delete _mbed;
    }
}

} // namespace Rover
} // namespace Soro


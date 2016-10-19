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

#include "videoserverarray.h"

#define LOG_TAG "VideoServerArray"

namespace Soro {
namespace Rover {

VideoServerArray::VideoServerArray(QObject *parent) : QObject(parent) {
    LOG_I(LOG_TAG, "Creating new video server array");
}

VideoServerArray::~VideoServerArray() {
    clear();
}

int VideoServerArray::populate(const QStringList& usbCamBlacklist, quint16 firstNetworkPort, int firstId) {
    clear();

    /*LOG_I(LOG_TAG, "Searching for flycapture cameras");
    FlycapEnumerator flycapEnum;
    int flycapCount = flycapEnum.loadCameras();
    LOG_I(LOG_TAG, "Number of flycap cameras detected: " + QString::number(flycapCount));

    foreach (FlyCapture2::PGRGuid guid, flycapEnum.listByGuid()) {
        LOG_I(LOG_TAG, "Found flycapture camera *-" + QString::number(guid.value[3]));
        // create associated video server
        VideoServer *server = new VideoServer(firstId, SocketAddress(QHostAddress::Any, firstNetworkPort), this);
        _servers.insert(firstId, server);
        _flycapCameras.insert(firstId, guid);
        connect(server, SIGNAL(stateChanged(VideoServer*, VideoServer::State)), this, SLOT(serverStateChanged(VideoServer*, VideoServer::State)));
        connect(server, SIGNAL(error(VideoServer*,QString)), this, SLOT(serverError(VideoServer*,QString)));

        firstId++;
        firstNetworkPort++;
    }*/

    LOG_I(LOG_TAG, "Searching for USB cameras (" + QString::number(usbCamBlacklist.size()) + " blacklist entries)");
    UsbCameraEnumerator usbCamEnum;
    int uvdCount = usbCamEnum.loadCameras();
    LOG_I(LOG_TAG, "Number of USB cameras detected: " + QString::number(uvdCount));

    foreach (QString videoDevice, usbCamEnum.listByDeviceName()) {
        bool blacklisted = false;
        foreach (QString blacklistedDevice, usbCamBlacklist) {
            if (videoDevice.mid(videoDevice.size() - 1).compare(blacklistedDevice.mid(blacklistedDevice.size() - 1)) == 0) {
                LOG_I(LOG_TAG, "Found USB camera " + videoDevice + ", however it is blacklisted");
                blacklisted = true;
                break;
            }
        }
        if (blacklisted) continue;
        LOG_I(LOG_TAG, "Found USB camera at " + videoDevice);
        // create associated video server
        VideoServer *server = new VideoServer(firstId, SocketAddress(QHostAddress::Any, firstNetworkPort), this);
        _servers.insert(firstId, server);
        _usbCameras.insert(firstId, videoDevice);
        connect(server, SIGNAL(stateChanged(MediaServer*, MediaServer::State)), this, SLOT(serverStateChanged(MediaServer*, MediaServer::State)));
        connect(server, SIGNAL(error(MediaServer*,QString)), this, SLOT(serverError(MediaServer*,QString)));

        firstId++;
        firstNetworkPort++;
    }

    return firstId - 1;
}

void VideoServerArray::activate(int index, VideoFormat format) {
    if (_servers.contains(index)) {
        LOG_I(LOG_TAG, "Camera " + QString::number(index) + " is about to be streamed");
        /*if (_flycapCameras.contains(index)) {
            _servers.value(index)->start(_flycapCameras[index], format);
        }
        else {
            _servers.value(index)->start(_usbCameras[index], format);
        }*/
        _servers.value(index)->start(_usbCameras[index], format);
    }
}

void VideoServerArray::deactivate(int index) {
    if (_servers.contains(index)) {
        LOG_I(LOG_TAG, "Camera " + QString::number(index) + " is about to be stopped");
        _servers.value(index)->stop();
    }
}

int VideoServerArray::serverCount() {
    return _servers.size();
}

void VideoServerArray::clear() {
    foreach (VideoServer *server, _servers) {
        server->stop();
        delete server;
    }
    _servers.clear();
    //_flycapCameras.clear();
    _usbCameras.clear();
}

void VideoServerArray::remove(int index) {
    _servers.remove(index);
}

void VideoServerArray::serverStateChanged(MediaServer *server, MediaServer::State state) {
    emit videoServerStateChanged(server->getMediaId(), state);
}

void VideoServerArray::serverError(MediaServer *server, QString error) {
    emit videoServerError(server->getMediaId(), error);
}

}
}

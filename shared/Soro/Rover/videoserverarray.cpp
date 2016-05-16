#include "videoserverarray.h"

#define LOG_TAG "VideoServerArray"

namespace Soro {
namespace Rover {

VideoServerArray::VideoServerArray(Logger *log, QObject *parent) : QObject(parent) {
    _log = log;
    LOG_I("Creating new video server array");
}

VideoServerArray::~VideoServerArray() {
    clear();
}

int VideoServerArray::populate(const QStringList& uvdBlacklist, quint16 firstNetworkPort) {
    clear();

    LOG_I("Searching for flycapture cameras");
    FlycapEnumerator flycapEnum;
    int flycapCount = flycapEnum.loadCameras();
    LOG_I("Number of flycap cameras detected: " + QString::number(flycapCount));

    int cameraId = 0;
    foreach (FlyCapture2::PGRGuid guid, flycapEnum.listByGuid()) {
        LOG_I("Found flycapture camera *-" + QString::number(guid.value[3]));
        // create associated video server
        VideoServer *server = new VideoServer(cameraId, SocketAddress(QHostAddress::Any, firstNetworkPort), _log, this);
        _servers.append(server);
        _flycapCameras.insert(cameraId, guid);
        connect(server, SIGNAL(stateChanged(VideoServer*, VideoServer::State)), this, SLOT(serverStateChanged(VideoServer*, VideoServer::State)));
        connect(server, SIGNAL(error(VideoServer*,QString)), this, SLOT(serverError(VideoServer*,QString)));

        cameraId++;
        firstNetworkPort++;
    }

    LOG_I("Searching for UVD cameras (" + QString::number(uvdBlacklist.size()) + " blacklist entries)");
    UvdCameraEnumerator uvdEnum;
    int uvdCount = uvdEnum.loadCameras();
    LOG_I("Number of UVD\'s/webcams detected: " + QString::number(uvdCount));

    foreach (QString videoDevice, uvdEnum.listByDeviceName()) {
        bool blacklisted = false;
        foreach (QString blacklistedDevice, uvdBlacklist) {
            if (videoDevice.mid(videoDevice.size() - 1).compare(blacklistedDevice.mid(blacklistedDevice.size() - 1)) == 0) {
                LOG_I("Found UVD device " + videoDevice + ", however it is blacklisted");
                blacklisted = true;
                break;
            }
        }
        if (blacklisted) continue;
        LOG_I("Found UVD/Webcam device at " + videoDevice);
        // create associated video server
        VideoServer *server = new VideoServer(cameraId, SocketAddress(QHostAddress::Any, firstNetworkPort), _log, this);
        _servers.append(server);
        _uvdCameras.insert(cameraId, videoDevice);
        connect(server, SIGNAL(stateChanged(VideoServer*, VideoServer::State)), this, SLOT(serverStateChanged(VideoServer*, VideoServer::State)));
        connect(server, SIGNAL(error(VideoServer*,QString)), this, SLOT(serverError(VideoServer*,QString)));

        cameraId++;
        firstNetworkPort++;
    }

    return cameraId - 1;
}

void VideoServerArray::activate(int index, StreamFormat format) {
    if (index < _servers.size()) {
        LOG_I("Camera " + QString::number(index) + " is about to be streamed");
        if (_flycapCameras.contains(index)) {
            _servers[index]->start(_flycapCameras[index], format);
        }
        else {
            _servers[index]->start(_uvdCameras[index], format);
        }
    }
}

void VideoServerArray::deactivate(int index) {
    if (index < _servers.size()) {
        LOG_I("Camera " + QString::number(index) + " is about to be stopped");
        _servers[index]->stop();
    }
}

int VideoServerArray::cameraCount() {
    return _servers.size();
}

void VideoServerArray::clear() {
    foreach (VideoServer *server, _servers) {
        server->stop();
        delete server;
    }
    _servers.clear();
    _flycapCameras.clear();
    _uvdCameras.clear();
}

void VideoServerArray::remove(int index) {
    if (index < _servers.size()) {
        _servers.at(index)->stop();
        delete _servers.at(index);
        _servers.removeAt(index);
    }
}

void VideoServerArray::serverStateChanged(VideoServer *server, VideoServer::State state) {
    emit videoServerStateChanged(server->getCameraId(), state);
}

void VideoServerArray::serverError(VideoServer *server, QString error) {
    emit videoServerError(server->getCameraId(), error);
}

}
}

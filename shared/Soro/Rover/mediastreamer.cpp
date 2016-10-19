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

#include "mediastreamer.h"

namespace Soro {
namespace Rover {

MediaStreamer::MediaStreamer(QObject *parent) : QObject(parent) { }

MediaStreamer::~MediaStreamer() {
    stop();
}

void MediaStreamer::stop() {
    qDebug() << "Stopping";
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
    if (_ipcSocket) {
        delete _ipcSocket;
        _ipcSocket = NULL;
    }
}

bool MediaStreamer::connectToParent(quint16 port) {
    _ipcSocket = new QTcpSocket(this);

    connect(_ipcSocket, SIGNAL(readyRead()),
            this, SLOT(ipcSocketReadyRead()));
    connect(_ipcSocket, SIGNAL(disconnected()),
            this, SLOT(ipcSocketDisconnected()));
    connect(_ipcSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(ipcSocketError(QAbstractSocket::SocketError)));

    _ipcSocket->connectToHost(QHostAddress::LocalHost, port);
    if (!_ipcSocket->waitForConnected(1000)) {
        qCritical() << "Unable to connect to parent on port " << port;
        QCoreApplication::exit(0);
        return false;
    }
    return true;
}

void MediaStreamer::ipcSocketReadyRead() {
    char buffer[512];
    while (_ipcSocket->bytesAvailable() > 0) {
        _ipcSocket->readLine(buffer, 512);
        if (QString(buffer).compare("stop") == 0) {
            qDebug() << "Got exit request from parent";
            stop();
            QCoreApplication::exit(0);
        }
    }
}

QGst::PipelinePtr MediaStreamer::createPipeline() {
    qDebug() << "Creating pipeline";

    QGst::PipelinePtr pipeline = QGst::Pipeline::create();
    pipeline->bus()->addSignalWatch();
    QGlib::connect(pipeline->bus(), "message", this, &MediaStreamer::onBusMessage);

    qDebug() << "Pipeline created";
    return pipeline;
}

void MediaStreamer::onBusMessage(const QGst::MessagePtr & message) {
    qDebug() << "Getting bus message";
    QByteArray errorMessage;
    switch (message->type()) {
    case QGst::MessageEos:
        qWarning() << "End-of-Stream";
        QCoreApplication::exit(STREAMPROCESS_ERR_GSTREAMER_EOS);
        break;
    case QGst::MessageError:
        errorMessage = message.staticCast<QGst::ErrorMessage>()->error().message().toLatin1();
        qCritical() << "Bus error: " << errorMessage.constData();
        QCoreApplication::exit(STREAMPROCESS_ERR_GSTREAMER_ERROR);
        break;
    default:
        break;
    }
}

void MediaStreamer::ipcSocketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    stop();
    QCoreApplication::exit(STREAMPROCESS_ERR_SOCKET_ERROR);
}

void MediaStreamer::ipcSocketDisconnected() {
    stop();
    QCoreApplication::exit(STREAMPROCESS_ERR_SOCKET_ERROR);
}

} // namespace Rover
} // namespace Soro


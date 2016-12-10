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

#include "camerawidget.h"
#include "ui_camerawidget.h"
#include "util.h"

#include "libsoro/logger.h"

#define LOG_TAG "CameraWidget"

namespace Soro {
namespace MissionControl {


CameraWidget::CameraWidget(QWidget *parent) : QWidget(parent), ui(new Ui::CameraWidget) {
    ui->setupUi(this);

    ui->messageLabel->setVisible(false);
    addWidgetShadow(ui->controlsWidget, 10, 0);

    _isPlaying = true;
    stop();
}

CameraWidget::~CameraWidget() {
    resetPipeline();
}

void CameraWidget::play(SocketAddress address, VideoFormat format) {
    resetPipeline();
    ui->messageLabel->setVisible(false);

    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &CameraWidget::onBusMessage);

    // create a udpsrc to receive the stream
    QString binStr = "udpsrc address=%1 port=%2 ! %3";

    binStr = binStr.arg(address.host.toString(),
                        QString::number(address.port),
                        format.createGstDecodingArgs());

    // create a gstreamer bin from the description
    QGst::BinPtr source = QGst::Bin::fromDescription(binStr);
    // create a gstreamer sink
    QGst::ElementPtr sink = createSink();

    _pipeline->add(source, sink);
    source->link(sink);

    _isPlaying = true;
    _pipeline->setState(QGst::StatePlaying);
}

QGst::ElementPtr CameraWidget::createSink() {
    QGst::ElementPtr sink = QGst::ElementFactory::make("qt5videosink");
    sink->setProperty("force-aspect-ratio", true);
    ui->videoWidget->setVideoSink(sink);
    return sink;
}

void CameraWidget::stop(QString reason, CameraWidget::Pattern pattern) {
    if (reason.isEmpty()) {
        ui->messageLabel->setText("<html><h2>No video :(</h2></html>");
    }
    else {
        ui->messageLabel->setText("<html><h2>No video :(</h2><br><br>" + reason + "</html>");
    }
    ui->messageLabel->setVisible(_showText);

    //if (_isPlaying) {
        resetPipeline();
        //create videotestsrc pipeline for coolness
        QString binStr = "videotestsrc pattern=%1 ! video/x-raw,width=%2,height=%3 ! videoconvert";
        binStr = binStr.arg(QString::number((int)reinterpret_cast<quint32&>(pattern)),
                            QString::number(ui->videoWidget->width()),
                            QString::number(ui->videoWidget->height()));

        _pipeline = QGst::Pipeline::create();
        QGst::ElementPtr sink = QGst::ElementFactory::make("qt5videosink");
        QGst::BinPtr source = QGst::Bin::fromDescription(binStr);
        sink->setProperty("force-aspect-ratio", false);

        _pipeline->add(source, sink);
        source->link(sink);
        ui->videoWidget->setVideoSink(sink);

        _isPlaying = false;
        _pipeline->setState(QGst::StatePlaying);
    //}
}

void CameraWidget::resetPipeline() {
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
}

void CameraWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    ui->messageLabel->move(0, 0);
    ui->messageLabel->resize(width(), height());
    ui->videoWidget->move(0, 0);
    ui->videoWidget->resize(width(), height());

    ui->controlsWidget->move(width() - ui->controlsWidget->width() - 20, height() - ui->controlsWidget->height() - 20);
}

QString CameraWidget::getCameraName() {
    return ui->cameraNameLabel->text();
}

void CameraWidget::setCameraName(QString name) {
    ui->cameraNameLabel->setText(name);
}

bool CameraWidget::isPlaying() {
    return _isPlaying;
}

void CameraWidget::showText(bool show) {
    _showText = show;
    if (!_isPlaying) {
        ui->messageLabel->setVisible(_showText);
    }
}

void CameraWidget::showLabel(bool show) {
    ui->controlsWidget->setVisible(show);
}

void CameraWidget::onBusMessage(const QGst::MessagePtr & message) {
    LOG_I(LOG_TAG, "onBusMessage(): Got bus message type " + message->typeName());
    switch (message->type()) {
    case QGst::MessageEos:
        stop("Received end-of-stream message.");
        emit eosMessage();
        break;
    case QGst::MessageError:
    {
        QString errorMessage = message.staticCast<QGst::ErrorMessage>()->error().message().toLatin1();
        LOG_E(LOG_TAG, "onBusMessage(): Received error message from gstreamer '" + errorMessage + "'");
        emit error();
    }
    default:
        break;
    }
}

} // namespace MissionControl
} // namespace Soro

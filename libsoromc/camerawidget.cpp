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

    if (!format.isUseable()) {
        LOG_E(LOG_TAG, "play(): Given unusable format, refusing to play");
        stop();
        return;
    }
    _videoFormat = format;

    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &CameraWidget::onBusMessage);

    // create a udpsrc to receive the stream
    QString binStr = "udpsrc address=%1 port=%2 reuse=true ! %3 ! videoscale ! video/x-raw,width=%4,height=%5 ! videoconvert";
    binStr = binStr.arg(address.host.toString(),
                        QString::number(address.port),
                        format.createGstDecodingArgs(),
                        QString::number(format.getWidth()),
                        QString::number(format.getHeight()));

    qDebug() << binStr;
    // create a gstreamer bin from the description
    QGst::BinPtr source = QGst::Bin::fromDescription(binStr);
    // create a gstreamer sink
    QGst::ElementPtr sink = createSink();

    _pipeline->add(source, sink);
    source->link(sink);

    _isPlaying = true;
    _pipeline->setState(QGst::StatePlaying);
    adjustVideoSize();
}

QGst::ElementPtr CameraWidget::createSink() {
    QGst::ElementPtr sink = QGst::ElementFactory::make("qwidget5videosink");
    sink->setProperty("force-aspect-ratio", false);
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

    _videoFormat = VideoFormat();
    resetPipeline();
    //create videotestsrc pipeline for coolness
    QString binStr = "videotestsrc pattern=%1 ! video/x-raw,width=%2,height=%3 ! videoconvert";
    binStr = binStr.arg(QString::number(static_cast<qint32>(pattern)),
                        QString::number(ui->videoWidget->width()),
                        QString::number(ui->videoWidget->height()));

    _pipeline = QGst::Pipeline::create();
    QGst::BinPtr source = QGst::Bin::fromDescription(binStr);
    QGst::ElementPtr sink = createSink();

    _pipeline->add(source, sink);
    source->link(sink);

    _isPlaying = false;
    _pipeline->setState(QGst::StatePlaying);
    adjustVideoSize();
}

void CameraWidget::resetPipeline() {
    if (_pipeline) {
        _pipeline->bus()->removeSignalWatch();
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
}

void CameraWidget::adjustVideoSize() {
    float videoW = _videoFormat.getWidth();
    float videoH = _videoFormat.getHeight();
    float widgetW = width();
    float widgetH = height();

    float width, height, x, y;
    if (_isPlaying && (videoW > 0) && (videoH > 0)) {
        if ((widgetW / widgetH) > (videoW / videoH)) {
            // Scale to match height
            width = widgetH * (videoW / videoH);
            height = widgetH;
            x = (widgetW - width) / 2;
            y = 0;
        }
        else {
            // Scale to match width
            height = widgetW * (videoH / videoW);
            width = widgetW;
            y = (widgetH - height) / 2;
            x = 0;
        }
    }
    else {
        width = widgetW;
        height = widgetH;
        x = 0;
        y = 0;
    }
    ui->videoWidget->move(x, y);
    ui->videoWidget->resize(width, height);
}

void CameraWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    ui->messageLabel->move(0, 0);
    ui->messageLabel->resize(width(), height());
    ui->controlsWidget->move(width() - ui->controlsWidget->width() - 20, height() - ui->controlsWidget->height() - 20);
    adjustVideoSize();
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

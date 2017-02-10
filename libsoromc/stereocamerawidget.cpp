#include "stereocamerawidget.h"
#include "ui_stereocamerawidget.h"

#include "libsoro/logger.h"

#define LOG_TAG "StereoCameraWidget"

#define NO_VIDEO_PATTERN CameraWidget::Pattern_SMPTE

namespace Soro {
namespace MissionControl {

StereoCameraWidget::StereoCameraWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StereoCameraWidget)
{
    ui->setupUi(this);
    ui->stereoLCameraWidget->showText(false);
    ui->stereoLCameraWidget->showLabel(false);
    ui->stereoRCameraWidget->showText(false);
    ui->stereoRCameraWidget->showLabel(false);
    ui->monoCameraWidget->showText(false);
    ui->monoCameraWidget->showLabel(false);

    _stereoMode = VideoFormat::StereoMode_None;

    stop(false);
}

StereoCameraWidget::~StereoCameraWidget()
{
    delete ui;
}

void StereoCameraWidget::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    ui->monoCameraWidget->move(0, 0);
    ui->monoCameraWidget->resize(width(), height());

    // Determine how to position the stereo
    switch (_stereoMode) {
    case VideoFormat::StereoMode_SideBySide:
        ui->stereoLCameraWidget->move(0, 0);
        ui->stereoLCameraWidget->resize(width() / 2, height());
        ui->stereoRCameraWidget->move(width() / 2, 0);
        ui->stereoRCameraWidget->resize(width() / 2, height());
        break;
    default:
        break;
    }
}

void StereoCameraWidget::setStereoMode(VideoFormat::StereoMode mode) {
    if (_stereoMode != mode) {
        if (isPlaying() && isStereoOn()) {
            LOG_E(LOG_TAG, "setStereoMode(): Cannot change stereo mode while playing in stereo. Please stop the player, then try again.");
            return;
        }
        _stereoMode = mode;
    }
}

VideoFormat::StereoMode StereoCameraWidget::getStereoMode() const {
    return _stereoMode;
}

void StereoCameraWidget::playStereo(SocketAddress addressL, VideoFormat encodingL, SocketAddress addressR, VideoFormat encodingR) {
    if (_stereoMode == VideoFormat::StereoMode_None) {
        LOG_E(LOG_TAG, "playStereo(): Stereo mode is not set on widget. Please specify which stereo configuration you want.");
        return;
    }

    if (ui->monoCameraWidget->isVisible()) {
        // Stop and hide mono
        ui->monoCameraWidget->stop("", NO_VIDEO_PATTERN);
        ui->monoCameraWidget->hide();
    }
    // Play stereo
    encodingL.setStereoMode(_stereoMode);
    encodingR.setStereoMode(_stereoMode);
    ui->stereoRCameraWidget->show();
    ui->stereoRCameraWidget->play(addressR, encodingR);
    ui->stereoLCameraWidget->show();
    ui->stereoLCameraWidget->play(addressL, encodingL);
    _isStereo = true;
    emit videoChanged();
}

void StereoCameraWidget::playMono(SocketAddress address, VideoFormat encoding) {
    if (!ui->monoCameraWidget->isVisible()) {
        // Stop and hide stereo
        ui->stereoLCameraWidget->stop("", NO_VIDEO_PATTERN);
        ui->stereoLCameraWidget->hide();
        ui->stereoRCameraWidget->stop("", NO_VIDEO_PATTERN);
        ui->stereoRCameraWidget->hide();
    }
    // Play mono
    ui->monoCameraWidget->show();
    ui->monoCameraWidget->play(address, encoding);
    _isStereo = false;
    emit videoChanged();
}

void StereoCameraWidget::stop(bool stereo) {
    if (stereo && (_stereoMode == VideoFormat::StereoMode_None)) {
        LOG_E(LOG_TAG, "stop(): Stereo mode is not set, cannot stop with stereo visualization. Please specify which stereo mode you want");
        stereo = false;
    }
    _isStereo = stereo;
    ui->stereoLCameraWidget->stop("", NO_VIDEO_PATTERN);
    ui->stereoRCameraWidget->stop("", NO_VIDEO_PATTERN);
    ui->monoCameraWidget->stop("", NO_VIDEO_PATTERN);
    if (_isStereo) {
        ui->monoCameraWidget->hide();
        ui->stereoLCameraWidget->show();
        ui->stereoRCameraWidget->show();
    }
    else {
        ui->stereoLCameraWidget->hide();
        ui->stereoRCameraWidget->hide();
        ui->monoCameraWidget->show();
    }
    emit videoChanged();
}

bool StereoCameraWidget::isStereoOn() const {
    return _isStereo;
}

bool StereoCameraWidget::isPlaying() const {
    return ui->monoCameraWidget->isPlaying() ||
            ui->stereoLCameraWidget->isPlaying() ||
            ui->stereoRCameraWidget->isPlaying();
}

} // namespace MissionControl
} // namespace Soro

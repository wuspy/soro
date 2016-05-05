#include "videowindow.h"

namespace Soro {
namespace MissionControl {

VideoWindow::VideoWindow(QWidget *parent) : QMainWindow(parent) {
    _videoWidget = new VideoStreamWidget(this);
    resize(800, 600);
}

VideoWindow::~VideoWindow() {
    delete _videoWidget;
}

VideoStreamWidget* VideoWindow::getVideoStreamWidget() {
    return _videoWidget;
}

void VideoWindow::resizeEvent(QResizeEvent *e) {
    QMainWindow::resizeEvent(e);
    _videoWidget->move(0, 0);
    _videoWidget->resize(width(), height());
}

} // namespace MissionControl
} // namespace Soro

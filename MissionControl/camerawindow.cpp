#include "camerawindow.h"

namespace Soro {
namespace MissionControl {

CameraWindow::CameraWindow(QWidget *parent) : QMainWindow(parent) {
    _cameraWidget = new CameraWidget(this);
    resize(800, 600);
}

CameraWindow::~CameraWindow() {
    delete _cameraWidget;
}

CameraWidget* CameraWindow::getCameraWidget() {
    return _cameraWidget;
}

void CameraWindow::resizeEvent(QResizeEvent *e) {
    QMainWindow::resizeEvent(e);
    _cameraWidget->move(0, 0);
    _cameraWidget->resize(width(), height());
}

} // namespace MissionControl
} // namespace Soro

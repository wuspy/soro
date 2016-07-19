#include "camerawindow.h"

namespace Soro {
namespace MissionControl {

CameraWindow::CameraWindow(QWidget *parent) : QMainWindow(parent) {
    _cameraWidget = new CameraWidget(this);
    resize(800, 600); // Default size
    setWindowTitle("Mission Control");
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

void CameraWindow::close() {
    _shouldClose = true;
    QMainWindow::close();
}

void CameraWindow::showEvent(QShowEvent *e) {
    _shouldClose = false;
}

void CameraWindow::closeEvent(QCloseEvent *e) {
    // Does not close from user action, but will close if called through close()
    if (!_shouldClose) {
        e->ignore();
    }
}

} // namespace MissionControl
} // namespace Soro

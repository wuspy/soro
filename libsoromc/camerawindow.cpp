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
    Q_UNUSED(e);
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

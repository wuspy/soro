#include "videowindow.h"
#include "ui_videowindow.h"

using namespace Soro::MissionControl;

VideoWindow::VideoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoWindow) {
    ui->setupUi(this);
}

void VideoWindow::keyPressEvent(QKeyEvent *e) {
    QMainWindow::keyPressEvent(e);
    if (e->key() == Qt::Key_F11) {
        if (_fullscreen) showNormal(); else showFullScreen();
        _fullscreen = !_fullscreen;
    }
}

VideoWindow::~VideoWindow() {
    delete ui;
}

VideoPane* VideoWindow::getVideoPane() {
    return ui->videoPane;
}

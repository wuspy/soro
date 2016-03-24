#include "videopane.h"
#include "ui_videopane.h"

using namespace Soro::MissionControl;

VideoPane::VideoPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoPane) {

    ui->setupUi(this);
    _instance = new VlcInstance(VlcCommon::args(), this);
    _player = new VlcMediaPlayer(_instance);
    _player->setVideoWidget(ui->videoWidget);
    ui->videoWidget->setMediaPlayer(_player);

    addShadow(ui->controlsWidget);
}

void VideoPane::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    ui->videoWidget->move(0,0);
    ui->videoWidget->resize(width(), height());
    ui->controlsWidget->move(width() - ui->controlsWidget->width() - 10,
                             height() - ui->controlsWidget->height() - 10);
}

void VideoPane::close() {
    if (_media != NULL) {
        ui->videoWidget->close();
        delete _media;
        _media = NULL;
    }
}

void VideoPane::openLocalFile(QString path) {
    if (_media != NULL) {
        delete _media;
    }
    _media = new VlcMedia(path, true, _instance);
    _player->open(_media);
}

VideoPane::~VideoPane() {
    delete ui;
}

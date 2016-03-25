#include "videopane.h"
#include "ui_videopane.h"

using namespace Soro::MissionControl;

VideoPane::VideoPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoPane) {

    ui->setupUi(this);
    addShadow(ui->controlsWidget);
    //yes, you can forward signals in Qt
    connect(ui->fullscreenButton, SIGNAL(clicked(bool)),
            this, SIGNAL(fullscreenRequested()));
    connect(ui->video360Button, SIGNAL(clicked(bool)),
            this, SLOT(video360Clicked()));
    connect(ui->video480Button, SIGNAL(clicked(bool)),
            this, SLOT(video480Clicked()));
    connect(ui->video720Button, SIGNAL(clicked(bool)),
            this, SLOT(video720Clicked()));
    connect(ui->video1080Button, SIGNAL(clicked(bool)),
            this, SLOT(video1080Clicked()));

    _instance = new VlcInstance(VlcCommon::args(), this);
    _player = new VlcMediaPlayer(_instance);
    _player->setVideoWidget(ui->videoWidget);
    ui->videoWidget->setMediaPlayer(_player);
}

VideoPane::~VideoPane() {
    delete ui;
    delete _instance;
}

void VideoPane::setVideoQuality(VideoQuality quality) {
    if (_quality != quality) {
        _quality = quality;
        ui->video360Button->setChecked(_quality == VideoQuality360);
        ui->video480Button->setChecked(_quality == VideoQuality480);
        ui->video720Button->setChecked(_quality == VideoQuality720);
        ui->video1080Button->setChecked(_quality == VideoQuality1080);
        emit qualitySelectionChanged(_quality);
    }
}

void VideoPane::setShowFullscreenOption(bool showOption) {
    ui->fullscreenButton->setVisible(showOption);
    ui->videoWidget->move(0,0);
    ui->videoWidget->resize(width(), height());
    ui->controlsWidget->adjustSize();
    ui->controlsWidget->move(width() - ui->controlsWidget->width() - 10,
                             height() - ui->controlsWidget->height() - 10);
}

void VideoPane::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    ui->videoWidget->move(0,0);
    ui->videoWidget->resize(width(), height());
    ui->controlsWidget->adjustSize();
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

void VideoPane::connectCameraStream(quint16 port, QHostAddress hostAddress){
    //TODO
}

void VideoPane::setCameraName(QString name) {
    ui->videoFeedNameLabel->setText(name);
    ui->controlsWidget->adjustSize();
    ui->controlsWidget->move(width() - ui->controlsWidget->width() - 10,
                             height() - ui->controlsWidget->height() - 10);
}

void VideoPane::video360Clicked(){
    if (_quality != VideoQuality360) {
        _quality = VideoQuality360;
        ui->video360Button->setChecked(true);
        ui->video480Button->setChecked(false);
        ui->video720Button->setChecked(false);
        ui->video1080Button->setChecked(false);
        emit qualitySelectionChanged(_quality);
    }
}

void VideoPane::video480Clicked(){
    if (_quality != VideoQuality480) {
        _quality = VideoQuality480;
        ui->video360Button->setChecked(false);
        ui->video480Button->setChecked(true);
        ui->video720Button->setChecked(false);
        ui->video1080Button->setChecked(false);
        emit qualitySelectionChanged(_quality);
    }
}

void VideoPane::video720Clicked() {
    if (_quality != VideoQuality720) {
        _quality = VideoQuality720;
        ui->video360Button->setChecked(false);
        ui->video480Button->setChecked(false);
        ui->video720Button->setChecked(true);
        ui->video1080Button->setChecked(false);
        emit qualitySelectionChanged(_quality);
    }
}

void VideoPane::video1080Clicked() {
    if (_quality != VideoQuality1080) {
        _quality = VideoQuality1080;
        ui->video360Button->setChecked(false);
        ui->video480Button->setChecked(false);
        ui->video720Button->setChecked(false);
        ui->video1080Button->setChecked(true);
        emit qualitySelectionChanged(_quality);
    }
}

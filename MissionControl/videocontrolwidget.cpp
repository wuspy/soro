#include "videocontrolwidget.h"
#include "ui_videocontrolwidget.h"

namespace Soro {
namespace MissionControl {

VideoControlWidget::VideoControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoControlWidget) {
    ui->setupUi(this);

    connect(ui->disabledRadioButton, SIGNAL(clicked(bool)),
            this, SLOT(optionButtonClicked()));
    connect(ui->lowRadioButton, SIGNAL(clicked(bool)),
            this, SLOT(optionButtonClicked()));
    connect(ui->normalRadioButton, SIGNAL(clicked(bool)),
            this, SLOT(optionButtonClicked()));
    connect(ui->highRadioButton, SIGNAL(clicked(bool)),
            this, SLOT(optionButtonClicked()));
    connect(ui->ultraRadioButton, SIGNAL(clicked(bool)),
            this, SLOT(optionButtonClicked()));
    connect(ui->editNameButton, SIGNAL(clicked(bool)),
            this, SLOT(editButtonClicked()));
    connect(ui->nameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(nameEditReturnClicked()));

    setAvailable(true);
}

VideoControlWidget::~VideoControlWidget() {
    delete ui;
}

void VideoControlWidget::setName(QString name) {
    ui->nameLabel->setText(name);
}

void VideoControlWidget::selectOption(VideoFormat option) {
    switch (option) {
    case VideoFormat_Null:
        ui->disabledRadioButton->setChecked(true);
        break;
    case Mpeg2_360p_750Kpbs:
        ui->lowRadioButton->setChecked(true);
        break;
    case Mpeg2_480p_1500Kpbs:
        ui->normalRadioButton->setChecked(true);
        break;
    case Mpeg2_720p_3000Kpbs:
        ui->highRadioButton->setChecked(true);
        break;
    case Mpeg2_720p_5000Kpbs:
        ui->ultraRadioButton->setChecked(true);
        break;
    }
}

void VideoControlWidget::optionButtonClicked() {
    if (ui->disabledRadioButton->isChecked()) {
        emit optionSelected(VideoFormat_Null);
    }
    else if (ui->lowRadioButton->isChecked()) {
        emit optionSelected(Mpeg2_360p_750Kpbs);
    }
    else if (ui->normalRadioButton->isChecked()) {
        emit optionSelected(Mpeg2_480p_1500Kpbs);
    }
    else if (ui->highRadioButton->isChecked()) {
        emit optionSelected(Mpeg2_720p_3000Kpbs);
    }
    else if (ui->ultraRadioButton->isChecked()) {
        emit optionSelected(Mpeg2_720p_5000Kpbs);
    }
}

void VideoControlWidget::setAvailable(bool available) {
    if (available) {
        ui->unavailableLabel->hide();
        ui->disabledRadioButton->show();
        ui->lowRadioButton->show();
        ui->normalRadioButton->show();
        ui->highRadioButton->show();
        ui->ultraRadioButton->show();
    }
    else {
        ui->unavailableLabel->show();
        ui->disabledRadioButton->hide();
        ui->lowRadioButton->hide();
        ui->normalRadioButton->hide();
        ui->highRadioButton->hide();
        ui->ultraRadioButton->hide();
    }
}

void VideoControlWidget::editButtonClicked() {
    ui->nameLabel->setVisible(false);
    ui->editNameButton->setVisible(false);
    ui->nameLineEdit->setVisible(true);
    ui->nameLineEdit->setText(getName());

    ui->nameLineEdit->setFocus();
    ui->nameLineEdit->selectAll();
}

QString VideoControlWidget::getName() {
    return ui->nameLabel->text();
}

void VideoControlWidget::nameEditReturnClicked() {
    setName(ui->nameLineEdit->text());
    ui->nameLineEdit->setVisible(false);
    ui->nameLabel->setVisible(true);
    ui->editNameButton->setVisible(true);
    emit userEditedName(getName());
}

}
}

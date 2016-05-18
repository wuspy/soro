#include "mediacontrolwidget.h"
#include "ui_mediacontrolwidget.h"

namespace Soro {
namespace MissionControl {

MediaControlWidget::MediaControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaControlWidget) {
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
    // default to video mode
    setMode(VideoMode);
}

MediaControlWidget::~MediaControlWidget() {
    delete ui;
}

void MediaControlWidget::setMode(MediaControlWidget::Mode mode) {
    _mode = mode;

    switch (mode) {
    case AudioMode:
        ui->graphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/volume_up_black_18px.png);");
        ui->editNameButton->setVisible(false);
        break;
    case VideoMode:
        ui->graphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_black_18px.png);");
        ui->editNameButton->setVisible(true);
        break;
    }

    ui->nameLineEdit->setVisible(false);

    // default to disabled
    ui->disabledRadioButton->setChecked(true);

    // force button layout update
    setAvailable(_available);
}

void MediaControlWidget::setName(QString name) {
    ui->nameLabel->setText(name);
}

void MediaControlWidget::selectOption(MediaControlWidget::Option option) {
    switch (option) {
    case DisabledOption:
        ui->disabledRadioButton->setChecked(true);
        break;
    case LowOption:
        ui->lowRadioButton->setChecked(true);
        break;
    case NormalOption:
        ui->normalRadioButton->setChecked(true);
        break;
    case HighOption:
        ui->highRadioButton->setChecked(true);
        break;
    case UltraOption:
        ui->ultraRadioButton->setChecked(true);
        break;
    }
}

void MediaControlWidget::optionButtonClicked() {
    if (ui->disabledRadioButton->isChecked()) {
        emit optionSelected(DisabledOption);
    }
    else if (ui->lowRadioButton->isChecked()) {
        emit optionSelected(LowOption);
    }
    else if (ui->normalRadioButton->isChecked()) {
        emit optionSelected(NormalOption);
    }
    else if (ui->highRadioButton->isChecked()) {
        emit optionSelected(HighOption);
    }
    else if (ui->ultraRadioButton->isChecked()) {
        emit optionSelected(UltraOption);
    }
}

void MediaControlWidget::setAvailable(bool available) {
    if (available) {
        ui->unavailableLabel->hide();
        ui->disabledRadioButton->show();
        ui->lowRadioButton->show();
        ui->normalRadioButton->show();
        switch (_mode) {
        case AudioMode:
            ui->highRadioButton->hide();
            ui->ultraRadioButton->hide();
            break;
        case VideoMode:
            ui->highRadioButton->show();
            ui->ultraRadioButton->show();
            break;
        }
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

void MediaControlWidget::editButtonClicked() {
    ui->nameLabel->setVisible(false);
    ui->editNameButton->setVisible(false);
    ui->nameLineEdit->setVisible(true);
    ui->nameLineEdit->setText(getName());
}

QString MediaControlWidget::getName() {
    return ui->nameLabel->text();
}

void MediaControlWidget::nameEditReturnClicked() {
    setName(ui->nameLineEdit->text());
    ui->nameLineEdit->setVisible(false);
    ui->nameLabel->setVisible(true);
    ui->editNameButton->setVisible(true);
    emit userEditedName(getName());
}

}
}

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

#include "videocontrolwidget.h"
#include "ui_videocontrolwidget.h"

namespace Soro {
namespace MissionControl {

VideoControlWidget::VideoControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoControlWidget) {
    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(uiOptionSelected(int)));
    connect(ui->editNameButton, SIGNAL(clicked(bool)),
            this, SLOT(editButtonClicked()));
    connect(ui->nameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(nameEditReturnClicked()));

    ui->nameLineEdit->setVisible(false);
    ui->nameLabel->setVisible(true);
    ui->editNameButton->setVisible(true);
}

VideoControlWidget::~VideoControlWidget() {
    delete ui;
}

void VideoControlWidget::setFormats(QList<VideoFormat> formats) {
    _formats = formats;

    ui->comboBox->clear();

    foreach (VideoFormat format, _formats) {
        ui->comboBox->addItem(format.toHumanReadableString());
    }
}

QList<VideoFormat> VideoControlWidget::getFormats() const {
    return _formats;
}

void VideoControlWidget::setName(QString name) {
    ui->nameLabel->setText(name);
}

void VideoControlWidget::selectOption(int index) {
    ui->comboBox->setCurrentIndex(index);
}

void VideoControlWidget::uiOptionSelected(int index) {
    emit optionSelected(index);
}

void VideoControlWidget::setAvailable(bool available) {
    if (available) {
        ui->unavailableLabel->hide();
        ui->comboBox->show();
    }
    else {
        ui->unavailableLabel->show();
        ui->comboBox->hide();
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

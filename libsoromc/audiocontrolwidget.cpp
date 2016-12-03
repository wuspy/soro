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

#include "audiocontrolwidget.h"
#include "ui_audiocontrolwidget.h"

namespace Soro {
namespace MissionControl {

AudioControlWidget::AudioControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioControlWidget) {

    ui->setupUi(this);

    connect(ui->disabledRadioButton, SIGNAL(clicked(bool)),
            this, SLOT(optionButtonClicked()));
    connect(ui->playRadioButton, SIGNAL(clicked(bool)),
            this, SLOT(optionButtonClicked()));
    connect(ui->muteCheckBox, SIGNAL(clicked(bool)),
            this, SIGNAL(muteToggled(bool)));

    setAvailable(true);
    selectOption(AudioFormat_Null);
}

AudioControlWidget::~AudioControlWidget() {
    delete ui;
}

void AudioControlWidget::selectOption(AudioFormat option) {
    switch (option) {
    case AudioFormat_Null:
        ui->disabledRadioButton->setChecked(true);
        break;
    case AC3:
        ui->playRadioButton->setChecked(true);
        break;
    }
}

void AudioControlWidget::setMute(bool mute) {
    disconnect(ui->muteCheckBox, SIGNAL(clicked(bool)),
            this, SIGNAL(muteToggled(bool)));

    ui->muteCheckBox->setChecked(mute);

    connect(ui->muteCheckBox, SIGNAL(clicked(bool)),
            this, SIGNAL(muteToggled(bool)));
}

void AudioControlWidget::optionButtonClicked() {
    if (ui->disabledRadioButton->isChecked()) {
        emit optionSelected(AudioFormat_Null);
    }
    else if (ui->playRadioButton->isChecked()) {
        emit optionSelected(AC3);
    }
}

bool AudioControlWidget::isMuted() {
    return ui->muteCheckBox->isChecked();
}

void AudioControlWidget::setAvailable(bool available) {
    if (available) {
        ui->unavailableLabel->hide();
        ui->disabledRadioButton->show();
        ui->muteCheckBox->show();
        ui->playRadioButton->show();
    }
    else {
        ui->unavailableLabel->show();
        ui->disabledRadioButton->hide();
        ui->muteCheckBox->hide();
        ui->playRadioButton->hide();
    }
}

}
}
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

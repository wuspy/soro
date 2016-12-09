#include "settingsform.h"

namespace Soro {
namespace MissionControl {

SettingsForm::SettingsForm(QQmlEngine *engine, QObject *parent) : QObject(parent)
{
    QObject *component = QQmlComponent(engine, QUrl("qrc:/Settings.qml")).create();
    if (!component) {
        QCoreApplication::exit(1);
    }
    else {
        _ui = qobject_cast<QQuickWindow*>(component);
    }

    _ui->setWidth(400);
    _ui->setHeight(380);
    _ui->setMinimumWidth(400);
    _ui->setMinimumHeight(380);
    _ui->setVisible(true);

    _stereoUiSwitch = _ui->findChild<QQuickItem*>("stereoUiSwitch");
    _stereoVideoSwitch = _ui->findChild<QQuickItem*>("stereoVideoSwitch");
    _videoFormatCombo = _ui->findChild<QQuickItem*>("videoFormatCombo");

    connect (_stereoUiSwitch, SIGNAL(checkedChanged()),
             this, SLOT(stereoUiSwitchCheckChanged()));
    connect (_stereoVideoSwitch, SIGNAL(checkedChanged()),
             this, SLOT(stereoVideoSwitchCheckChanged()));


    //_videoFormatCombo->setProperty("model", videoFormats);
}

void SettingsForm::stereoUiSwitchCheckChanged() {
    _stereoUiSwitch->setProperty("text", _stereoUiSwitch->property("checked").toBool() ? "Stereo On" : "Stereo Off");
}


void SettingsForm::stereoVideoSwitchCheckChanged() {
    _stereoVideoSwitch->setProperty("text", _stereoVideoSwitch->property("checked").toBool() ? "Stereo On" : "Stereo Off");
}

} // namespace MissionControl
} // namespace Soro

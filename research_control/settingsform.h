#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QObject>
#include <QDebug>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QtQuickControls2>

#include "libsoro/videoformat.h"

namespace Soro {
namespace MissionControl {

class SettingsForm : public QObject
{
    Q_OBJECT
public:
    explicit SettingsForm(QQmlEngine *engine, QObject *parent = 0);

private:
    QQuickWindow *_ui;
    QQuickItem *_stereoUiSwitch;
    QQuickItem *_stereoVideoSwitch;
    QQuickItem *_videoFormatCombo;

private slots:
    void stereoUiSwitchCheckChanged();
    void stereoVideoSwitchCheckChanged();
signals:

public slots:
};

} // namespace MissionControl
} // namespace Soro


#endif // SETTINGSFORM_H

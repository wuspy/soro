#ifndef SORO_MISSIONCONTROL_RESEARCHMAINWINDOW_H
#define SORO_MISSIONCONTROL_RESEARCHMAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QQuickItem>
#include <QTimerEvent>
#include <QMouseEvent>

#include "libsoromc/stereocamerawidget.h"
#include "libsoromc/gamepadmanager.h"
#include "libsoromc/drivecontrolsystem.h"

namespace Ui {
class ResearchMainWindow;
}

namespace Soro {
namespace MissionControl {

class ResearchMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ResearchMainWindow(const GamepadManager *gamepad, const DriveControlSystem *driveSystem, QWidget *parent = 0);
    ~ResearchMainWindow();

    StereoCameraWidget *getCameraWidget();
    bool isHudVisible() const;
    int getHudParallax() const;

signals:
    void closed();

public slots:
    void sensorUpdate(char tag, int value);
    void setHudParallax(int parallax);
    void setHudVisible(bool visible);

protected:
    void closeEvent(QCloseEvent *event);
    void timerEvent(QTimerEvent *event);
    void resizeEvent(QResizeEvent *event);

private slots:
    void adjustHud();
    void adjustSizeAndPosition();
    void gamepadPoll();

private:
    Ui::ResearchMainWindow *ui;
    const GamepadManager *_gamepad;
    const DriveControlSystem *_driveSystem;
    int _updateLatencyTimerId = TIMER_INACTIVE;
    int _hudParallax = 50;
    bool _hudVisible = true;
    QPoint _mouseDownPos;
};

} // namespace MissionControl
} // namespace Soro

#endif // SORO_MISSIONCONTROL_RESEARCHMAINWINDOW_H

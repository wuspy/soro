#ifndef INITWINDOW_H
#define INITWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QCloseEvent>

#include "missioncontrolnetwork.h"
#include "missioncontrolprocess.h"
#include "gamepadmanager.h"
#include "armcontrolsystem.h"
#include "drivecontrolsystem.h"
#include "cameracontrolsystem.h"

namespace Ui {
class InitWindow;
}

namespace Soro {
namespace MissionControl {

class InitWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit InitWindow(QWidget *parent = 0);
    ~InitWindow();

private slots:
    void init_start();
    void init_address();
    void init_mcNetwork();
    void init_controlSystem();
    void init_gamepadManager();

    void mcNetworkConnected(bool broker);
    void mcNetworkDisconnected();
    void mcNetworkRoleGranted(Role role);
    void mcNetworkRoleDenied();
    void showInvalidAddressError();

    void roverAddressTextChanged(QString text);

    void mcWindowClosed();

    void armOperatorSelected();
    void cameraOperatorSelected();
    void driverSelected();
    void spectatorSelected();

    void setErrorText(QString text);
    void setStatusText(QString text);
    void setCompletedText(QString text);
    void showStatus();
    void showRoleButtons();

    void reset();

protected:
    void closeEvent(QCloseEvent *e);

private:
    bool _addressValid = false;
    Ui::InitWindow *ui;
    MissionControlNetwork *_mcNetwork = NULL;
    MissionControlProcess *_mc = NULL;
    ControlSystem *_controlSystem = NULL;
    GamepadManager *_gamepad = NULL;
};

}
}

#endif // INITWINDOW_H

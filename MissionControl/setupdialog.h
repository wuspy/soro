#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QMap>

#include "soroini.h"
#include "soro_global.h"
#include "missioncontrolprocess.h"
#include "soromainwindow.h"

namespace Ui {
class SetupDialog;
}

namespace Soro {
namespace MissionControl {

class SetupDialog : public QDialog {
    Q_OBJECT
public:
    explicit SetupDialog(QWidget *parent = 0);
    ~SetupDialog();
    MissionControlProcess::Role getSelectedRole() const;
    bool getIsMasterNode() const;
    /*QHostAddress getSelectedMainHost() const;
    QHostAddress getSelectedVideoHost() const;
    QHostAddress getSelectedLocalLanHost() const;
    QHostAddress getSelectedMasterArmHost() const;*/

private:
    Ui::SetupDialog *ui;
    MissionControlProcess::Role _role;
    bool _masterNode = false;
    /*QMap<QString, QHostAddress> _hosts;
    int _mainHostCurrentIndex = 0, _videoHostCurrentIndex = 0,
        _localLanHostCurrentIndex = 0, _masterArmHostCurrentIndex = 0;*/

private slots:
    void armOperatorClicked();
    void driverClicked();
    void cameraOperatorClicked();
    void spectatorClicked();
    void masterCheckBoxToggled(bool checked);
    /*void mainHostCurrentIndexChanged(int index);
    void videoHostCurrentIndexChanged(int index);
    void masterArmHostCurrentIndexChanged(int index);
    void localLanHostCurrentIndexChanged(int index);*/
};

}
}

#endif // SETUPDIALOG_H

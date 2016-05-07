#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QMap>
#include <QMessageBox>

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
    QString getName() const;

private:
    Ui::SetupDialog *ui;
    MissionControlProcess::Role _role;
    bool _masterNode = false;
    QString _name = "";
    bool verifyName();

private slots:
    void armOperatorClicked();
    void driverClicked();
    void cameraOperatorClicked();
    void spectatorClicked();
    void masterCheckBoxToggled(bool checked);
};

}
}

#endif // SETUPDIALOG_H

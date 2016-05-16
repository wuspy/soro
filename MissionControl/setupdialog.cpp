#include "setupdialog.h"
#include "ui_setupdialog.h"

namespace Soro {
namespace MissionControl {

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog) {
    ui->setupUi(this);
    addWidgetShadow(ui->controlsContainerWidget, 30, 0);

    connect(ui->armOperatorLabel, SIGNAL(clicked()),
            this, SLOT(armOperatorClicked()));
    connect(ui->driverLabel, SIGNAL(clicked()),
            this, SLOT(driverClicked()));
    connect(ui->cameraOperatorLabel, SIGNAL(clicked()),
            this, SLOT(cameraOperatorClicked()));
    connect(ui->spectatorLabel, SIGNAL(clicked()),
            this, SLOT(spectatorClicked()));
    connect(ui->masterNodeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(masterCheckBoxToggled(bool)));
}

SetupDialog::~SetupDialog() {
    delete ui;
}

bool SetupDialog::verifyName() {
    if (ui->nameLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::critical(this, "Error", "Enter a name for your mission control", QMessageBox::Ok);
        return false;
    }
    QRegularExpression antiBill("(bu(s|\\$)h.|d(i|1)ck.cheney)"
                                "*("
                                    "(made.*m(o|0)ney)"
                                    "|iraq"
                                    "|((9|n(i|1)ne)(-|\\/| )?(11|e(1|l)even))"
                                    "| wmd"
                                    "|weap(o|0)n.*mass destruct(i|1)(o|0)n"
                                    "|jet *fuel"
                                ")|inside.*j(o|0)b"
                                "|(george *(w( |\\.|$)|bu(s|\\$)h))"
                                "|((jet *fuel|melt).*(stee(l|1)|beams))"
                                "|((thermite|b(o|0)mb).*gr(o|0)und *(zer(o|0)|0))"
                                "|terr(o|0)r(i|1)st",
                                QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch match = antiBill.match(ui->nameLineEdit->text());
    if (match.hasMatch()) {
        QMessageBox::information(this, "Error", "Fuck off Bill", QMessageBox::Ok);
        return false;
    }

    _name = ui->nameLineEdit->text().trimmed();
    return true;
}

void SetupDialog::armOperatorClicked() {
    if (!verifyName()) return;
    _role = ArmOperatorRole;
    accept();
}

void SetupDialog::driverClicked() {
    if (!verifyName()) return;
    _role = DriverRole;
    accept();
}

void SetupDialog::cameraOperatorClicked() {
    if (!verifyName()) return;
    _role = CameraOperatorRole;
    accept();
}

void SetupDialog::spectatorClicked() {
    if (!verifyName()) return;
    _role = SpectatorRole;
    accept();
}

void SetupDialog::masterCheckBoxToggled(bool checked) {
    _masterNode = checked;
}

Role SetupDialog::getSelectedRole() const {
    return _role;
}

bool SetupDialog::getIsMasterNode() const {
    return _masterNode;
}

QString SetupDialog::getName() const {
    return _name;
}

} // namespace MissionControl
} // namespace Soro

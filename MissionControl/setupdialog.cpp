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

    //populate network hosts
    /*QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();
    _hosts.insert("Default", QHostAddress::Any); //add default option
    foreach (QNetworkInterface iface, all) {
        unsigned int flags = iface.flags();
        bool isLoopback = (bool)(flags & QNetworkInterface::IsLoopBack);
        bool isRunning = (bool)(flags & QNetworkInterface::IsRunning);
        if (isRunning && (iface.addressEntries().size() > 0)) {
            if (isLoopback) {
                _hosts.insert("Localhost on " + iface.addressEntries()[0].ip().toString(),
                        iface.addressEntries()[0].ip());
            }
            else {
                _hosts.insert(iface.humanReadableName() + " (" + iface.addressEntries()[0].ip().toString() + ")",
                        iface.addressEntries()[0].ip());
            }
        }
    }
    foreach (QString hostName, _hosts.keys()) {
        ui->mainHostComboBox->addItem(hostName);
        ui->videoHostComboBox->addItem(hostName);
        ui->masterArmHostComboBox->addItem(hostName);
        ui->localLanHostComboBox->addItem(hostName);
    }
    ui->mainHostComboBox->setCurrentIndex(0);
    ui->videoHostComboBox->setCurrentIndex(0);
    ui->masterArmHostComboBox->setCurrentIndex(0);
    ui->localLanHostComboBox->setCurrentIndex(0);

    connect(ui->mainHostComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(mainHostCurrentIndexChanged(int)));
    connect(ui->videoHostComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(videoHostCurrentIndexChanged(int)));
    connect(ui->localLanHostComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(localLanHostCurrentIndexChanged(int)));
    connect(ui->masterArmHostComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(masterArmHostCurrentIndexChanged(int)));*/
}

SetupDialog::~SetupDialog() {
    delete ui;
}

bool SetupDialog::verifyName() {
    QRegularExpression antiBill("Bush")
}

void SetupDialog::armOperatorClicked() {
    if (!verifyName()) return;
    _role = MissionControlProcess::ArmOperator;
    accept();
}

void SetupDialog::driverClicked() {
    if (!verifyName()) return;
    _role = MissionControlProcess::Driver;
    accept();
}

void SetupDialog::cameraOperatorClicked() {
    if (!verifyName()) return;
    _role = MissionControlProcess::CameraOperator;
    accept();
}

void SetupDialog::spectatorClicked() {
    if (!verifyName()) return;
    _role = MissionControlProcess::Spectator;
    accept();
}

void SetupDialog::masterCheckBoxToggled(bool checked) {
    _masterNode = checked;
}

/*void SetupDialog::mainHostCurrentIndexChanged(int index) {
    _mainHostCurrentIndex = index;
}

void SetupDialog::videoHostCurrentIndexChanged(int index) {
    _videoHostCurrentIndex = index;
}

void SetupDialog::localLanHostCurrentIndexChanged(int index) {
    _localLanHostCurrentIndex = index;
}

void SetupDialog::masterArmHostCurrentIndexChanged(int index) {
    _masterArmHostCurrentIndex = index;
}

QHostAddress SetupDialog::getSelectedMainHost() const {
    return _hosts.values()[_mainHostCurrentIndex];
}

QHostAddress SetupDialog::getSelectedVideoHost() const {
    return _hosts.values()[_videoHostCurrentIndex];
}

QHostAddress SetupDialog::getSelectedLocalLanHost() const {
    return _hosts.values()[_localLanHostCurrentIndex];
}

QHostAddress SetupDialog::getSelectedMasterArmHost() const {
    return _hosts.values()[_masterArmHostCurrentIndex];
}*/


MissionControlProcess::Role SetupDialog::getSelectedRole() const {
    return _role;
}

bool SetupDialog::getIsMasterNode() const {
    return _masterNode;
}

} // namespace MissionControl
} // namespace Soro

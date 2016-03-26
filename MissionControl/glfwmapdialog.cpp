#include "glfwmapdialog.h"
#include "ui_glfwmapdialog.h"

#define LABEL_NOT_BOUNT "NOT BOUND"
#define LABEL_AXIS "Axis"
#define LABEL_BUTTON "Button"

using namespace Soro::MissionControl;

GlfwMapDialog::GlfwMapDialog(QWidget *parent, int controllerId, Soro::GlfwMap *map) :
    QDialog(parent),
    ui(new Ui::GlfwMapDialog) {

    ui->setupUi(this);
    _map = map;
    _controllerId = controllerId;

    if (_map == NULL || !glfwJoystickPresent(_controllerId)) {
        //the controller we were given is not connected??
        ui->controllerLabel->setText("Controller: None");
        return; //nothing to do here
    }

    populateTable();

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(buttonClicked(QAbstractButton*)));
    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),
            this, SLOT(tableCellClicked(int,int)));
}

void GlfwMapDialog::showEvent(QShowEvent *e) {
    QDialog::showEvent(e);
    if (glfwJoystickPresent(_controllerId)) {
        START_TIMER(_joyTimerId, 20);
    }
}

void GlfwMapDialog::hideEvent(QHideEvent *e) {
    QDialog::hideEvent(e);
    KILL_TIMER(_joyTimerId);
}

void GlfwMapDialog::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    int row = ui->tableWidget->currentRow();
    if ((e->timerId() == _joyTimerId)) {
        if (!glfwJoystickPresent(_controllerId)) {
            //controller has been disconnected
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The controller you were using was disconnected. Reconnect it please.",
                        QMessageBox::Ok, this).exec();
            KILL_TIMER(_joyTimerId);
            close();
            return;
        }
        if (row >= 0) {
            int axisCount, buttonCount;
            const float *axes = glfwGetJoystickAxes(_controllerId, &axisCount);
            const unsigned char *buttons = glfwGetJoystickButtons(_controllerId, &buttonCount);
            //TODO
            if (row < _map->axisCount()) {
                for (int i = 0; i < axisCount; i++) {
                    float abs = qAbs(axes[i]);
                    //axes may default to 1, -1, or 0 so I need to get tricky
                    if ((abs < 0.9) & (abs > 0.7)) {
                        qDebug() << axes[i];
                        _map->AxisList[row].GlfwIndex = i;
                        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(LABEL_AXIS + QString::number(i)));
                        break;
                    }
                }
            }
            else {
                for (int i = 0; i < buttonCount; i++) {
                    if (buttons[i] == 1) {
                        _map->ButtonList[row - _map->axisCount()].GlfwIndex = i;
                        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(LABEL_BUTTON + QString::number(i)));
                        break;
                    }
                }
            }
        }
    }
}

void GlfwMapDialog::populateTable() {
    ui->tableWidget->clear();
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Type" << "Function" << "Binding" << "");
    if (_map != NULL) {
        int axisCount = _map->axisCount();
        int buttonCount = _map->buttonCount();
        ui->tableWidget->setRowCount(axisCount + buttonCount);
        for(int i = 0; i < axisCount; i++) {
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(LABEL_AXIS));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(_map->AxisList[i].DisplayName));
            if (_map->AxisList[i].isMapped()) {
                const int mapIndex = _map->AxisList[i].GlfwIndex;
                ui->tableWidget->setItem(i, 2, new QTableWidgetItem(LABEL_AXIS + QString::number(mapIndex)));
            }
            else {
                ui->tableWidget->setItem(i, 2, new QTableWidgetItem(LABEL_NOT_BOUNT));
            }
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem("Clear"));
        }
        for(int i = 0; i < buttonCount; i++) {
            ui->tableWidget->setItem(i + axisCount, 0, new QTableWidgetItem(LABEL_BUTTON));
            ui->tableWidget->setItem(i + axisCount, 1, new QTableWidgetItem(_map->ButtonList[i].DisplayName));
            if (_map->ButtonList[i].isMapped()) {
                const int mapIndex = _map->ButtonList[i].GlfwIndex;
                ui->tableWidget->setItem(i + axisCount, 2, new QTableWidgetItem(LABEL_BUTTON + QString::number(mapIndex)));
            }
            else {
                ui->tableWidget->setItem(i + axisCount, 2, new QTableWidgetItem(LABEL_NOT_BOUNT));
            }
            ui->tableWidget->setItem(i + axisCount, 3, new QTableWidgetItem("Clear"));
        }
    }
}

void GlfwMapDialog::tableCellClicked(int row, int column) {
    if (column == 3) {
        if (row < _map->axisCount()) {
            _map->AxisList[row].reset();
        }
        else {
            _map->ButtonList[row - _map->axisCount()].reset();
        }
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(LABEL_NOT_BOUNT));
    }
}

void GlfwMapDialog::buttonClicked(QAbstractButton *button) {
    switch (ui->buttonBox->standardButton(button)) {
    case QDialogButtonBox::Reset:
        _map->reset();
        populateTable();
        break;
    case QDialogButtonBox::Save:
        close();
        break;
    default: break;
    }
}

GlfwMapDialog::~GlfwMapDialog() {
    KILL_TIMER(_joyTimerId);
    delete ui;
}

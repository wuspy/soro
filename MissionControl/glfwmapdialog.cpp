#include "glfwmapdialog.h"
#include "ui_glfwmapdialog.h"

#define LABEL_NOT_BOUNT "NOT BOUND"
#define LABEL_AXIS "Axis"
#define LABEL_BUTTON "Button"

#define AXIS_THRESHOLD 0.8

using namespace Soro::MissionControl;

GlfwMapDialog::GlfwMapDialog(QWidget *parent, Soro::GlfwMap *map) :
    QDialog(parent),
    ui(new Ui::GlfwMapDialog) {

    ui->setupUi(this);
    glfwInit();

    _map = map;
    if (_map == NULL) {
        ui->gamepadsComboBox->addItem("None");
        return;
    }
    populateJoyMenu();
    _controllerId = joyIdByName(_map->ControllerName);

    populateTable();

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(buttonClicked(QAbstractButton*)));
    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),
            this, SLOT(tableCellClicked(int,int)));
    connect(ui->gamepadsComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(joyMenuSelectionChanged(int)));

    _joyTimerId = startTimer(50);
}

int GlfwMapDialog::joyIdByName(QString name) {
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
        if (glfwJoystickPresent(i) && (QString(glfwGetJoystickName(i)) == name)) {
            return i;
        }
    }
    return GLFW_JOYSTICK_1;
}

void GlfwMapDialog::populateJoyMenu() {
    ui->gamepadsComboBox->clear();
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
        if (glfwJoystickPresent(i)) {
            ui->gamepadsComboBox->addItem(QString(glfwGetJoystickName(i)), i);
        }
    }
}

void GlfwMapDialog::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _joyTimerId) {
        int buttonCount, axisCount;
        const float *axes = glfwGetJoystickAxes(_controllerId, &axisCount);
        const unsigned char *buttons = glfwGetJoystickButtons(_controllerId, &buttonCount);
        const int row = ui->tableWidget->currentRow();
        if (row == -1) return;
        switch ((*_map)[row].Type) {
        case GlfwMap::AXIS:
            for (int i = 0; i < axisCount; i++) {
                if ((abs(axes[i]) > AXIS_THRESHOLD) && ((*_map)[row].GlfwIndex != i)) {
                    (*_map)[row].GlfwIndex = i;
                    ui->tableWidget->item(row, 2)->setText(LABEL_AXIS + QString::number(i));
                    return;
                }
            }
            break;
        case GlfwMap::BUTTON:
            for (int i = 0; i < buttonCount; i++) {
                if ((buttons[i] == 1) && ((*_map)[row].GlfwIndex != i)) {
                    (*_map)[row].GlfwIndex = i;
                    ui->tableWidget->item(row, 2)->setText(LABEL_BUTTON + QString::number(i));
                    return;
                }
            }
            break;
        }
    }
}

void GlfwMapDialog::joyMenuSelectionChanged(int index) {
    _map->ControllerName = ui->gamepadsComboBox->itemText(index);
    _controllerId = ui->gamepadsComboBox->itemData(index).toInt();
}

void GlfwMapDialog::populateTable() {
    ui->tableWidget->clear();
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Type" << "Name" << "Binding" << "");
    if (_map != NULL) {
        ui->tableWidget->setRowCount(_map->count());
        for(int i = 0; i < _map->count(); i++) {
            GlfwMap::InputType type = (*_map)[i].Type;
            switch (type) {
            case GlfwMap::AXIS:
                ui->tableWidget->setItem(i, 0, new QTableWidgetItem(LABEL_AXIS));
                break;
            case GlfwMap::BUTTON:
                ui->tableWidget->setItem(i, 0, new QTableWidgetItem(LABEL_BUTTON));
                break;
            }
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem((*_map)[i].Name));
            const int mapIndex = (*_map)[i].GlfwIndex;
            switch (mapIndex) {
            case GlfwMap::MapItem::UNMAPPED:
                ui->tableWidget->setItem(i, 2, new QTableWidgetItem(LABEL_NOT_BOUNT));
                break;
            default:
                switch (type) {
                case GlfwMap::AXIS:
                    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(LABEL_AXIS + QString::number(mapIndex)));
                    break;
                case GlfwMap::BUTTON:
                    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(LABEL_BUTTON + QString::number(mapIndex)));
                    break;
                }
                break;
            }
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem("Clear"));
        }
    }
}

void GlfwMapDialog::tableCellClicked(int row, int column) {
    if (column == 3) {
        (*_map)[row].GlfwIndex = GlfwMap::MapItem::UNMAPPED;
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
        glfwTerminate();
        close();
        break;
    default: break;
    }
}

GlfwMapDialog::~GlfwMapDialog() {
    killTimer(_joyTimerId);
    delete ui;
    glfwTerminate();
}

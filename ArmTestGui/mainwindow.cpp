#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _mainTimerID = -1;

    _serial = new QSerialPort("COM3", this);
    _serial->setParity(QSerialPort::NoParity);
    _serial->setStopBits(QSerialPort::OneStop);
    _serial->setBaudRate(9600);
    if (_serial->open(QIODevice::WriteOnly)) {
        qDebug() << "mbed serial is opened";
        connect(_serial, SIGNAL(readyRead()),
                this, SLOT(serialReadyRead()));
    }
    else {
        qCritical() << "Could not open mbed serial";
    }
    glfwInit();
    if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
        ui->joyNameLabel->setText("Joystick: " + QString(glfwGetJoystickName(GLFW_JOYSTICK_1)));
        _mainTimerID = startTimer(50, Qt::PreciseTimer);
    }
    else {
        ui->joyNameLabel->setText("NO GAMEPAD");
        qCritical() << "No gamepad is connected!";
    }
}

void MainWindow::timerEvent(QTimerEvent *e) {
    if ((e->timerId() == _mainTimerID) && glfwJoystickPresent(GLFW_JOYSTICK_1)) {
        int axisCount, buttonCount;
        const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
        const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axisCount);
        signed char lx = joyAxisToByte(axes[0]);
        signed char ly = joyAxisToByte(axes[1]);
        signed char rx = joyAxisToByte(axes[2]);
        signed char ry = joyAxisToByte(axes[3]);
        unsigned char bx = buttons[0];
        unsigned char ba = buttons[1];
        unsigned char bb = buttons[2];
        unsigned char by = buttons[3];
        unsigned char lb = buttons[4];
        unsigned char rb = buttons[5];
        unsigned char lt = buttons[6];
        unsigned char rt = buttons[7];
        _serial->putChar((unsigned char)255); //start of block
        _serial->putChar(lx);
        _serial->putChar(ly);
        _serial->putChar(rx);
        _serial->putChar(ry);
        _serial->putChar(bx);
        _serial->putChar(ba);
        _serial->putChar(bb);
        _serial->putChar(by);
        _serial->putChar(lt);
        _serial->putChar(rt);
        _serial->putChar(lb);
        _serial->putChar(rb);
    }
}

void MainWindow::serialReadyRead() {
    qDebug() << QString(_serial->readAll()) << endl;
}

//keyboard implementaiton
/*void MainWindow::timerEvent(QTimerEvent *e) {
    if ((e->timerId() == _mainTimerID) && (_pressed != '\0')) {
        qDebug() << "Sending char " << _pressed;
        _serial->putChar(_pressed);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
    _pressed = QChar(e->key()).toLower().toLatin1();
    qDebug() << "Pressed " << _pressed;
}

void MainWindow::keyReleaseEvent(QKeyEvent *e) {
    char other = QChar(e->key()).toLower().toLatin1();
    qDebug() << "Released " << other;
    if (_pressed == other) {
        _pressed = '\0';
    }
}*/

MainWindow::~MainWindow() {
    delete ui;
    _serial->close();
    delete _serial;
}

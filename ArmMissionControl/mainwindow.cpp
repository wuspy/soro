#include "mainwindow.h"
#include "ui_mainwindow.h"

#define SEND_INTERVAL 25

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);

    QString appPath = QApplication::applicationDirPath();

    _log = new Logger(this);
    _log->setLogfile(appPath + "/arm_log_server.txt");

    _armChannel = new Channel(this, appPath + "/arm_channel_server.ini", _log);

    //Setup arm channel
    connect(_armChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
            this, SLOT(channelStateChanged(Channel*,Channel::State)));
    connect(_armChannel, SIGNAL(messageReceived(Channel*,QByteArray)),
            this, SLOT(channelMessageReceived(Channel*,QByteArray)));
    connect(_armChannel, SIGNAL(peerAddressChanged(Channel*,Channel::SocketAddress)),
            this, SLOT(channelPeerAddressChanged(Channel*,Channel::SocketAddress)));
    connect(_armChannel, SIGNAL(statisticsUpdate(Channel*,int,quint64,quint64,int,int)),
            this, SLOT(channelStatisticsUpdate(Channel*,int,quint64,quint64,int,int)));
    _armChannel->open();

    glfwInit();
    if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
        ui->joyNameLabel->setText("Joystick: " + QString(glfwGetJoystickName(GLFW_JOYSTICK_1)));
        _sendTimerID = startTimer(SEND_INTERVAL);
    }
    else {
        ui->joyNameLabel->setText("NO GAMEPAD");
        qCritical() << "No gamepad is connected!";
        showCriticalError("No gamepad is connected!");
    }

    if (_armChannel->getState() == Channel::ErrorState) {
        qCritical() << "Error configuring channel";
        showCriticalError("Error configuring channel");
    }
    updateChannelStateLabel(ui->channelStateLabel, _armChannel->getState());
}

MainWindow::~MainWindow() {
    delete ui;
    delete _armChannel;
    delete _log;
}

//keyboard implementation

/*void MainWindow::timerEvent(QTimerEvent *e) {
    if ((e->timerId() == _sendTimerID) && (_pressed != '\0')) {
        qDebug() << "Sending char " << _pressed;
        _armChannel->sendMessage(QByteArray(1, _pressed));
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

//joystick implementation
void MainWindow::timerEvent(QTimerEvent *e) {
    if (e->timerId() == _sendTimerID) {
        if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
            QString statusText;
            int axisCount, buttonCount;
            const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
            const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axisCount);
            signed char lx;
            signed char ly;
            signed char rx;
            signed char ry;
            unsigned char bx;
            unsigned char ba;
            unsigned char bb;
            unsigned char by;
            unsigned char lb;
            unsigned char rb;
            unsigned char lt;
            unsigned char rt;
            unsigned char dpu;
            unsigned char dpd;
            unsigned char dpr;
            unsigned char dpl;
            if (axisCount == 6) {
                //triggers are axies
                lx = joyAxisToByte(axes[0]);
                ly = joyAxisToByte(axes[1]);
                rx = joyAxisToByte(axes[4]);
                ry = joyAxisToByte(axes[5]);
                lt = axes[2] > 0.2 ? GLFW_PRESS : GLFW_RELEASE;
                rt = axes[3] > 0.2 ? GLFW_PRESS : GLFW_RELEASE;
                bx = buttons[0];
                ba = buttons[1];
                bb = buttons[2];
                by = buttons[3];
                lb = buttons[4];
                rb = buttons[5];
                dpu = buttons[6];
                dpd = buttons[7];
                dpr = buttons[8];
                dpl = buttons[9];
            }
            else {
                //triggers are buttons
                lx = joyAxisToByte(axes[0]);
                ly = joyAxisToByte(axes[1]);
                rx = joyAxisToByte(axes[2]);
                ry = joyAxisToByte(axes[3]);
                bx = buttons[0];
                ba = buttons[1];
                bb = buttons[2];
                by = buttons[3];
                lb = buttons[4];
                rb = buttons[5];
                lt = buttons[6];
                rt = buttons[7];
                dpu = buttons[8];
                dpd = buttons[9];
                dpr = buttons[10];
                dpl = buttons[11];
            }

            QByteArray message("", 12);
            message[0] = lx;
            message[1] = ly;
            message[2] = rx;
            message[3] = ry;
            message[4] = bx;
            message[5] = ba;
            message[6] = bb;
            message[7] = by;
            message[8] = lb;
            message[9] = rb;
            message[10] = lt;
            message[11] = rt;

            _armChannel->sendMessage(message);

            //update status text
            statusText += "AXES: " + QString ::number(axisCount) + "\tBUTTONS: " + QString::number(buttonCount) + "\n";
            statusText += "\nL AnalogX......." + QString::number(lx);
            statusText += "\nL AnalogY......." + QString::number(ly);
            statusText += "\nR AnalogX......." + QString::number(rx);
            statusText += "\nR AnalogY......." + QString::number(ry);
            statusText += "\nA Button........" + QString::number(ba);
            statusText += "\nB Button........" + QString::number(bb);
            statusText += "\nX Button........" + QString::number(bx);
            statusText += "\nY Button........" + QString::number(by);
            statusText += "\nL Trigger......." + QString::number(lt);
            statusText += "\nR Trigger......." + QString::number(rt);
            statusText += "\nL Bumper........" + QString::number(lb);
            statusText += "\nR Bumper........" + QString::number(rb);
            statusText += "\nU DPad.........." + QString::number(dpu);
            statusText += "\nR DPad.........." + QString::number(dpr);
            statusText += "\nD DPad.........." + QString::number(dpd);
            statusText += "\nL DPad.........." + QString::number(dpl);
            ui->joyPositionsLabel->setText(statusText);
        }
        else {
            killTimer(_sendTimerID);
            qCritical() << "No gamepad is connected!";
            showCriticalError("No gamepad is connected!");
        }
    }
}

void MainWindow::showCriticalError(QString error) {
    QMessageBox msg(QMessageBox::Critical, "Error", error, QMessageBox::Ok, this);
    connect(&msg, SIGNAL(finished(int)), this, SLOT(close()));
    msg.show();
}

void MainWindow::channelMessageReceived(Channel *channel, const QByteArray &message) {

}

void MainWindow::channelPeerAddressChanged(Channel *channel, Channel::SocketAddress address) {
    updateChannelPeerAddressLabel(ui->channelPeerLabel, address);
}

void MainWindow::channelStateChanged(Channel *channel, Channel::State state) {
    updateChannelStateLabel(ui->channelStateLabel, state);
}

void MainWindow::channelStatisticsUpdate(Channel *channel, int rtt, quint64 msg_up, quint64 msg_down, int rate_up, int rate_down) {
    updateChannelStatsLabel(ui->channelStatsLabel, rtt, rate_up, rate_down);
}

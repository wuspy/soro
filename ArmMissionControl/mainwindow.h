#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

#include "soro.h"
#include "soroui.h"

#include <stdio.h>
#include <stdlib.h>
#include <QKeyEvent>

#include "GLFW/glfw3.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Logger *_log;
    Channel *_armChannel;
    int _sendTimerID;
    //char _pressed;
    void showCriticalError(QString error);

private slots:
    void channelMessageReceived(Channel *channel, const QByteArray &message);
    void channelStateChanged(Channel *channel, Channel::State state);
    void channelPeerAddressChanged(Channel *channel, Channel::SocketAddress address);
    void channelStatisticsUpdate(Channel *channel, int rtt, quint64 msg_up, quint64 msg_down, int rate_up, int rate_down);

protected:
    void timerEvent(QTimerEvent *e);
    //void keyPressEvent(QKeyEvent *e);
    //void keyReleaseEvent(QKeyEvent *e);
};

#endif // MAINWINDOW_H

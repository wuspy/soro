#ifndef SOROMAINWINDOW_H
#define SOROMAINWINDOW_H

#include <QMainWindow>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QtWebEngineWidgets>
#include <QDebug>
#include <QFile>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "channel.h"
#include "soro_global.h"
#include "mbedchannel.h"
#include "glfwmapdialog.h"
#include "latlng.h"
#include "sorowindowcontroller.h"

namespace Ui {
    class SoroMainWindow;
}

namespace Soro {
namespace MissionControl {

    class SoroMainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit SoroMainWindow(QWidget *parent = 0);
        ~SoroMainWindow();

    private:
        Ui::SoroMainWindow *ui;
        SoroWindowController *_controller;
        bool _fullscreen = false;
        char _currentKey = '\0';

    signals:
        void settingsClicked();

    private slots:
        void sharedChannelStateChanged(Channel::State state);
        void controlChannelStateChanged(Channel::State state);
        void controlChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                                int rateUp, int rateDown);
        void masterArmStateChanged(MbedChannel::State);
        void controllerGamepadChanged(QString name);
        void controllerInitialized(const SoroIniConfig& soroConfig,
                                   const MissionControlIniConfig& mcConfig);
        void controllerError(QString description);
        void controllerWarning(QString description);
        void controllerConnectionQualityUpdate(int sharedRtt, int tcpLag);

    protected:
        void keyPressEvent(QKeyEvent *e);
        void keyReleaseEvent(QKeyEvent *e);
        void resizeEvent(QResizeEvent *e);
        void timerEvent(QTimerEvent *e);
    };

}
}

#endif // SOROMAINWINDOW_H

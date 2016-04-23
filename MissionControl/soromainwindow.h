#ifndef SOROMAINWINDOW_H
#define SOROMAINWINDOW_H

#include <QMainWindow>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QtWebEngineWidgets>
#include <QDebug>
#include <QHostAddress>
#include <QFile>
#include <QtCore/qmath.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include "channel.h"
#include "soro_global.h"
#include "mbedchannel.h"
#include "latlng.h"
#include "missioncontrolprocess.h"

namespace Ui {
    class SoroMainWindow;
}

namespace Soro {
namespace MissionControl {

    class SoroMainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit SoroMainWindow(QHostAddress mainHost, QHostAddress videoHost, QHostAddress localLanHost, QHostAddress masterArmHost,
                                bool masterSubnetNode, MissionControlProcess::Role role, QWidget *parent = 0);
        ~SoroMainWindow();

    private:
        Ui::SoroMainWindow *ui;
        MissionControlProcess *_controller;
        bool _fullscreen = false;
        int _initTimerId = TIMER_INACTIVE;

        void updateGamepadLabel(SDL_GameController *controller);

    signals:
        void settingsClicked();

    private slots:
        void sharedChannelStateChanged(Channel::State state);
        void controlChannelStateChanged(Channel::State state);
        void controlChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                                int rateUp, int rateDown);
        void masterArmStateChanged(MbedChannel::State);
        void controllerError(QString description);
        void controllerWarning(QString description);
        void controllerConnectionQualityUpdate(int sharedRtt, int tcpLag);
        void controllerGamepadChanged(SDL_GameController *controller);

    protected:
        void keyPressEvent(QKeyEvent *e);
        void resizeEvent(QResizeEvent *e);
        void timerEvent(QTimerEvent *e);
    };

}
}

#endif // SOROMAINWINDOW_H

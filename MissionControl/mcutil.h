#ifndef MCUTIL_H
#define MCUTIL_H

#include <QtGui>
#include <QtWidgets>
#include <QGraphicsDropShadowEffect>
#include <QMap>

#include "GLFW/glfw3.h"

#include "channel.h"

namespace Soro {

    static int glfwJoystickCount() {
        int count = 0;
        for (int i  = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i)) count++;
        }
        return count;
    }

    static QMap<int, QString> glfwJoystickNames() {
        QMap<int, QString> joys;
        for (int i  = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i)) {
                qDebug() << i << ": " << glfwGetJoystickName(i);
                joys.insert(i, glfwGetJoystickName(i));
            }
        }
        return joys;
    }

    static QString formatDataRate(int rate, QString units) {
        if (rate > 1000000000) {
            return QString::number(rate / 1000000000) + "G" + units;
        }
        if (rate > 1000000) {
            return QString::number(rate/ 1000000) + "M" + units;
        }
        if (rate > 1000) {
            return QString::number(rate / 1000) + "K" + units;
        }
        return QString::number(rate) + units;
    }

    static void updateChannelStateLabel(QLabel *label, Channel::State state) {
        switch (state) {
        case Channel::ConnectedState:
            label->setStyleSheet("QLabel { color : #1B5E20; }");
            label->setText("Connected");
            break;
        case Channel::ConnectingState:
            label->setStyleSheet("QLabel { color : #F57F17; }");
            label->setText("Connecting...");
            break;
        case Channel::ErrorState:
            label->setStyleSheet("QLabel { color : #B71C1C; }");
            label->setText("Fatal Error");
            break;
        case Channel::UnconfiguredState:
            label->setStyleSheet("QLabel { color : #F57F17; }");
            label->setText("Configuring...");
            break;
        case Channel::ReadyState:
            label->setStyleSheet("QLabel { color : #1B5E20; }");
            label->setText("Ready");
            break;
        }
    }

    static void updateChannelPeerAddressLabel(QLabel *label, Soro::SocketAddress peer) {
        if (peer.address == QHostAddress::Null) {
            label->setText("---");
        }
        else {
            label->setText(peer.toString());
        }
    }

    static void updateChannelRttLabel(QLabel *label, int rtt) {
        if (rtt >= 0) {
            label->setText(QString::number(rtt) + "ms");
        }
        else {
            label->setText("---");
        }
    }

    static void updateControlBandwithLabel(QLabel *label, int bitrateUp, int bitrateDown) {
        label->setText("▲ " + formatDataRate(bitrateUp, "b/s") + " ▼ " + formatDataRate(bitrateDown, "b/s"));
    }

    static void updateVideoBandwithLabel(QLabel *label, int bitrate) {
        label->setText("▼ " + formatDataRate(bitrate, "b/s"));
    }

    static void showCriticalError(QString message, QMainWindow *winToClose) {
        QMessageBox msg(QMessageBox::Critical, "WOW VERY ERROR",
                        "SOMETHING WENT TERRIBLY WRONG OMG GET JACOB IMMEDIATELY\n\n" + message,
                        QMessageBox::Ok, winToClose);
        msg.connect(&msg, SIGNAL(finished(int)), winToClose, SLOT(close()));
        msg.exec();
    }

    static void addShadow(QWidget *target, int radius, int offset) {
        QGraphicsDropShadowEffect* ef = new QGraphicsDropShadowEffect;
        ef->setBlurRadius(radius);
        ef->setOffset(offset);
        target->setGraphicsEffect(ef);
    }

    static void addShadow(QWidget *target) {
        addShadow(target, 15, 0);
    }

    static bool handleFullscreenEvent(QMainWindow *window, QKeyEvent *e, bool *stateHolder) {
        if (e->key() == Qt::Key_F11) {
            if (*stateHolder) window->showNormal(); else window->showFullScreen();
            *stateHolder = !*stateHolder;
            return true;
        }
        return false;
    }

}

#endif // MCUTIL_H

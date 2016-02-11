#ifndef SOROUI_H
#define SOROUI_H

#include "soroui_global.h"
#include "channel.h"

#include <QtGui>
#include <QtWidgets/QLabel>

static QString formatDataRate(int rate, QString timeUnit) {
    if (rate > 1000000000) {
        return QString::number(rate / 1000000000) + "GB/" + timeUnit;
    }
    if (rate > 1000000) {
        return QString::number(rate/ 1000000) + "MB/" + timeUnit;
    }
    if (rate > 1000) {
        return QString::number(rate / 1000) + "KB/" + timeUnit;
    }
    return QString::number(rate) + " B/" + timeUnit;
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
    case Channel::AwaitingConfigurationState:
        label->setStyleSheet("QLabel { color : #F57F17; }");
        label->setText("Configuring...");
        break;
    case Channel::ReadyState:
        label->setStyleSheet("QLabel { color : #1B5E20; }");
        label->setText("Ready");
        break;
    }
}

static void updateChannelPeerAddressLabel(QLabel *label, Channel::SocketAddress peer) {
    if (peer.address == QHostAddress::Null) {
        label->setText("---");
    }
    else {
        label->setText(peer.toString());
    }
}

static void updateChannelStatsLabel(QLabel *label, int rtt, int rateUp, int rateDown) {
    if (rtt > 500) {
        label->setStyleSheet("QLabel { color : #B71C1C; }");
    }
    else if (rtt > 200) {
        label->setStyleSheet("QLabel { color : #F57F17; }");
    }
    else {
        label->setStyleSheet("");
    }
    label->setText("RTT " + (rtt < 0 ? "---" : QString::number(rtt) + "ms") + "\t▲ "
                   + formatDataRate(rateUp, "s") + "\t▼ "
                   + formatDataRate(rateDown, "s"));
}

#endif // SOROUI_H

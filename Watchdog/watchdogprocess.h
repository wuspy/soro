#ifndef WATCHDOGPROCESS_H
#define WATCHDOGPROCESS_H

#include <QObject>
#include <QUdpSocket>
#include <QProcess>

namespace Soro {

class WatchdogProcess : public QObject {
    Q_OBJECT
private:
    QUdpSocket *_kickSocket;
    QProcess *_roverProcess = NULL;
    void startProcess();

public:
    explicit WatchdogProcess(QObject *parent = 0);
    ~WatchdogProcess();

private slots:
    void roverExited();
};

}

#endif // WATCHDOGPROCESS_H

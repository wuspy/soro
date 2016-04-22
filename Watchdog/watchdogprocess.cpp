#include "watchdogprocess.h"

#define ROVER_PATH "~/Rover"

namespace Soro {

WatchdogProcess::WatchdogProcess(QObject *parent) : QObject(parent) {
    startProcess();
}

WatchdogProcess::~WatchdogProcess() {
    if (_roverProcess != NULL) {
        _roverProcess->kill();
        delete _roverProcess;
    }
}

void WatchdogProcess::startProcess() {
    _roverProcess = new QProcess(this);
    _roverProcess->start(ROVER_PATH, QStringList() << "");
    connect(_roverProcess, SIGNAL(finished(int)),
            this, SLOT(roverExited()));
}

void WatchdogProcess::roverExited() {
    delete _roverProcess;
    startProcess();
}

}

#include <QCoreApplication>
#include "watchdogprocess.h"

using namespace Soro;

int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);
    WatchdogProcess watchdog(&a);

    return a.exec();
}

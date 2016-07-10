#include <QCoreApplication>

#include "soro_global.h"
#include "initcommon.h"
#include "configuration.h"
#include "roverprocess.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Soro::Configuration config;

    if (!Soro::init(&config, "rover")) {
        return 1;
    }

    // create main rover worker object
    Soro::Rover::RoverProcess worker(&config, &a);

    return a.exec();
}

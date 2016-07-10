#include <QCoreApplication>

#include "soro_global.h"
#include "configuration.h"
#include "initcommon.h"
#include "rover2process.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Soro::Configuration config;

    if (!Soro::init(&config, "rover")) {
        return 1;
    }
    
    // create secondary rover worker object
    Soro::Rover::Rover2Process worker(&config, &a);

    return a.exec();
}

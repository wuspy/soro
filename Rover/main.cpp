#include <QCoreApplication>

#include "roverprocess.h"

#include <Qt5GStreamer/QGst/Init>

using namespace Soro::Rover;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QGst::init();

    RoverProcess worker(&a);

    return a.exec();
}

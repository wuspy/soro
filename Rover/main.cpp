#include <QCoreApplication>

#include "roverworker.h"

using namespace Soro::Rover;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    RoverWorker worker(&a);

    return a.exec();
}

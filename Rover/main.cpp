#include <QCoreApplication>

#include "roverprocess.h"

using namespace Soro::Rover;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    RoverProcess worker(&a);

    return a.exec();
}

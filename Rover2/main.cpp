#include <QCoreApplication>

#include "rover2process.h"

using namespace Soro::Rover;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Rover2Process worker(&a);

    return a.exec();
}

#include <QCoreApplication>
#include "armnetworkinterface.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ArmNetworkInterface task(&a);

    return a.exec();
}

#include <QCoreApplication>
#include "videopuncherworker.h"

using namespace Soro;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    VideoPuncherWorker w(&a);
    return a.exec();
}

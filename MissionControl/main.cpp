#include <QApplication>

#include "soromainwindow.h"
#include "soro_global.h"

using namespace Soro::MissionControl;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    SoroMainWindow w;
    w.show();

    return a.exec();
}

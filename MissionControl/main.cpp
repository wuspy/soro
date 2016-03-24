#include <QApplication>

#include "mcmainwindow.h"

using namespace Soro::MissionControl;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    McMainWindow w;
    w.show();

    return a.exec();
}

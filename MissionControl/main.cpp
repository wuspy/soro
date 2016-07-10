#include <QApplication>

#include "missioncontrolprocess.h"
#include "soro_global.h"
#include "initcommon.h"
#include "initwindow.h"
#include "logger.h"

#include <Qt5GStreamer/QGst/Init>

using namespace Soro;
using namespace Soro::MissionControl;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QGst::init();

    InitWindow window;
    window.show();

    return a.exec();
}

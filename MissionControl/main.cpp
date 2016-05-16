#include <QApplication>

#include "missioncontrolprocess.h"
#include "setupdialog.h"
#include "soro_global.h"

#include <Qt5GStreamer/QGst/Init>

using namespace Soro::MissionControl;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    SetupDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        QGst::init();

        MissionControlProcess missionControl(dialog.getName(), dialog.getIsMasterNode(), dialog.getSelectedRole(), &a);
        int status = a.exec();

        return status;
    }

    return 0;
}

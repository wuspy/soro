#include <QApplication>

#include "soromainwindow.h"
#include "setupdialog.h"
#include "soro_global.h"

using namespace Soro::MissionControl;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    SetupDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        SoroMainWindow w(dialog.getSelectedMainHost(), dialog.getSelectedVideoHost(),
                         dialog.getSelectedLocalLanHost(), dialog.getSelectedMasterArmHost(),
                         dialog.getIsMasterNode(), dialog.getSelectedRole());
        w.show();

        return a.exec();
    }
    return 0;
}

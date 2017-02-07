#include "researchmainwindow.h"
#include "ui_researchmainwindow.h"

namespace Soro {
namespace MissionControl {

ResearchMainWindow::ResearchMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ResearchMainWindow)
{
    ui->setupUi(this);
}

ResearchMainWindow::~ResearchMainWindow()
{
    delete ui;
}

StereoCameraWidget* ResearchMainWindow::getCameraWidget() {
    return ui->cameraWidget;
}

void ResearchMainWindow::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);
    emit closed();
}

} // namespace MissionControl
} // namespace Soro

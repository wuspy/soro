#ifndef SORO_MISSIONCONTROL_RESEARCHMAINWINDOW_H
#define SORO_MISSIONCONTROL_RESEARCHMAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

#include "libsoromc/stereocamerawidget.h"

namespace Ui {
class ResearchMainWindow;
}

namespace Soro {
namespace MissionControl {

class ResearchMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ResearchMainWindow(QWidget *parent = 0);
    ~ResearchMainWindow();

    StereoCameraWidget *getCameraWidget();

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::ResearchMainWindow *ui;
};

} // namespace MissionControl
} // namespace Soro

#endif // SORO_MISSIONCONTROL_RESEARCHMAINWINDOW_H

#ifndef SORO_MISSIONCONTROL_CAMERAWINDOW_H
#define SORO_MISSIONCONTROL_CAMERAWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>

#include "camerawidget.h"

namespace Soro {
namespace MissionControl {

/* Wrapper for a CameraWidget that displays it in its
 * own window.
 */
class CameraWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CameraWindow(QWidget *parent = 0);
    ~CameraWindow();
    CameraWidget *getCameraWidget();

private:
    CameraWidget *_cameraWidget;

protected:
    void resizeEvent(QResizeEvent *e);
};

} // namespace MissionControl
} // namespace Soro

#endif // SORO_MISSIONCONTROL_CAMERAWINDOW_H

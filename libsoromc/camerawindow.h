#ifndef SORO_MISSIONCONTROL_CAMERAWINDOW_H
#define SORO_MISSIONCONTROL_CAMERAWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QShowEvent>

#include "soro_missioncontrol_global.h"
#include "camerawidget.h"

namespace Soro {
namespace MissionControl {

/* Wrapper for a CameraWidget that displays it in its
 * own window.
 */
class LIBSOROMC_EXPORT CameraWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CameraWindow(QWidget *parent = 0);
    ~CameraWindow();
    CameraWidget *getCameraWidget();
    void close();

private:
    CameraWidget *_cameraWidget;
    bool _shouldClose;

protected:
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);
};

} // namespace MissionControl
} // namespace Soro

#endif // SORO_MISSIONCONTROL_CAMERAWINDOW_H

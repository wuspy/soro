#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>

#include "camerawidget.h"

namespace Soro {
namespace MissionControl {

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

#endif // CAMERAWINDOW_H

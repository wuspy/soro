#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>

#include "videostreamwidget.h"

namespace Soro {
namespace MissionControl {

class VideoWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit VideoWindow(QWidget *parent = 0);
    ~VideoWindow();
    VideoStreamWidget *getVideoStreamWidget();

private:
    VideoStreamWidget *_videoWidget;

protected:
    void resizeEvent(QResizeEvent *e);
};

} // namespace MissionControl
} // namespace Soro

#endif // VIDEOWINDOW_H

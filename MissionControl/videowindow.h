#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>

#include "videopane.h"

namespace Ui {
class VideoWindow;
}

namespace Soro {
namespace MissionControl {

    class VideoWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit VideoWindow(QWidget *parent = 0);
        ~VideoWindow();
        VideoPane* getVideoPane();

    private:
        Ui::VideoWindow *ui;
        bool _fullscreen = false;

    protected:
        void keyPressEvent(QKeyEvent *);
    };

}
}

#endif // VIDEOWINDOW_H

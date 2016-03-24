#ifndef VIDEOPANE_H
#define VIDEOPANE_H

#include <QWidget>
#include <QResizeEvent>

#include "mcutil.h"

#include <VLCQtCore/Media.h>
#include <VLCQtCore/Common.h>
#include <VLCQtCore/Config.h>
#include <VLCQtCore/Instance.h>
#include <VLCQtCore/MediaPlayer.h>
#include <VLCQtWidgets/WidgetVideo.h>

namespace Ui {
    class VideoPane;
}

namespace Soro {
namespace MissionControl {

    class VideoPane: public QWidget {
        Q_OBJECT
    public:
        explicit VideoPane(QWidget *parent = 0);
        ~VideoPane();
    signals:

    public:
        void close();
        void openLocalFile(QString filePath);

    private:
        Ui::VideoPane *ui;
        VlcInstance *_instance = NULL;
        VlcMediaPlayer *_player = NULL;
        VlcMedia *_media = NULL;

    protected:
        void resizeEvent(QResizeEvent *);
    };

}
}

#endif // VIDEOPANE_H

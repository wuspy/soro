#ifndef VIDEOPANE_H
#define VIDEOPANE_H

#include <QWidget>
#include <QResizeEvent>
#include <QHostAddress>

#include <VLCQtCore/Media.h>
#include <VLCQtCore/Common.h>
#include <VLCQtCore/Config.h>
#include <VLCQtCore/Instance.h>
#include <VLCQtCore/MediaPlayer.h>
#include <VLCQtWidgets/WidgetVideo.h>

#include "soro_global.h"

namespace Ui {
    class VideoPane;
}

namespace Soro {
namespace MissionControl {

    class VideoPane: public QWidget {
        Q_OBJECT
    public:
        enum VideoQuality {
            VideoQuality360, VideoQuality480, VideoQuality720, VideoQuality1080
        };

        explicit VideoPane(QWidget *parent = 0);
        ~VideoPane();
        void close();
        void openLocalFile(QString filePath);
        void connectUdpStream(quint16 port, QHostAddress hostAddress = QHostAddress::Any);
        void setCameraName(QString name);
        void setVideoQuality(VideoQuality quality);
        void setShowFullscreenOption(bool showOption);

    signals:
        void qualitySelectionChanged(VideoQuality quality);
        void fullscreenRequested();

    private:
        Ui::VideoPane *ui;
        VlcInstance *_instance = NULL;
        VlcMediaPlayer *_player = NULL;
        VlcMedia *_media = NULL;
        VideoQuality _quality = VideoQuality360;

    protected:
        void resizeEvent(QResizeEvent *);

    private slots:
        void video360Clicked();
        void video480Clicked();
        void video720Clicked();
        void video1080Clicked();
    };

}
}

#endif // VIDEOPANE_H

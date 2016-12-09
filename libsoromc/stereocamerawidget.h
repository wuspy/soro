#ifndef SORO_MISSIONCONTROL_STEREOCAMERAWIDGET_H
#define SORO_MISSIONCONTROL_STEREOCAMERAWIDGET_H

#include <QWidget>

#include "libsoro/socketaddress.h"
#include "libsoro/videoformat.h"

namespace Ui {
class StereoCameraWidget;
}

namespace Soro {
namespace MissionControl {

/* A simple widget using CameraWidget that can display either side-by-side stereo video,
 * or mono video.
 */
class StereoCameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StereoCameraWidget(QWidget *parent = 0);
    ~StereoCameraWidget();

    void setStereoMode(VideoFormat::StereoMode mode);
    void stop(bool stereo=false);
    void playStereo(SocketAddress addressL, VideoFormat encodingL, SocketAddress addressR, VideoFormat encodingR);
    void playMono(SocketAddress address, VideoFormat encoding);

    bool isPlayingStereo() const;
    bool isPlayingMono() const;
    bool isPlaying() const;
    VideoFormat::StereoMode getStereoMode() const;

protected:
    void resizeEvent(QResizeEvent* event);

private:
    Ui::StereoCameraWidget *ui;
    VideoFormat::StereoMode _stereoMode;
};

} // namespace MissionControl
} // namespace Soro

#endif // SORO_MISSIONCONTROL_STEREOCAMERAWIDGET_H

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

    bool isPlaying() const;

    /* Gets the stereo mode set on the widget
     */
    VideoFormat::StereoMode getStereoMode() const;

    /* Returns true if the widget is playing in stereo, or
     * stopped with a stereo visualization
     */
    bool isStereoOn() const;

signals:
    void videoChanged();

protected:
    void resizeEvent(QResizeEvent* event);

private:
    Ui::StereoCameraWidget *ui;
    VideoFormat::StereoMode _stereoMode;
    bool _isStereo = false;
};

} // namespace MissionControl
} // namespace Soro

#endif // SORO_MISSIONCONTROL_STEREOCAMERAWIDGET_H

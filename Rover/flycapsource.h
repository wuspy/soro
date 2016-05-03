#ifndef FLYCAPSOURCE_H
#define FLYCAPSOURCE_H

#include <QObject>

#include <QTimerEvent>
#include <Qt5GStreamer/QGst/Buffer>
#include <Qt5GStreamer/QGst/Caps>
#include <Qt5GStreamer/QGst/Utils/ApplicationSource>
#include <flycapture/FlyCapture2.h>
#include <cstring>
#include <QThread>

#include "logger.h"
#include "soro_global.h"

namespace Soro {
namespace Rover {

/* Connects a Point Grey Flycapture camera as a source in gstreamer.
 * This source will always output at the camera's default resolution, however
 * the framerate is configurable.
 */
class FlycapSource : public QObject, public QGst::Utils::ApplicationSource {
    Q_OBJECT
public:

    explicit FlycapSource(FlyCapture2::PGRGuid guid, Logger *log = 0, QObject *parent = 0);
    ~FlycapSource();

    const FlyCapture2::Camera* getCamera() const;
    int getVideoWidth() const;
    int getVideoHeight() const;
    int getFramerate() const;

    void setFramerate(int framerate);

private:
    FlyCapture2::CameraInfo _cameraInfo;
    FlyCapture2::Camera _camera;
    int _framerate, _width, _height;
    int _captureTimerId = TIMER_INACTIVE;
    bool _enabled;
    QString LOG_TAG;
    Logger *_log;

protected:
    void timerEvent(QTimerEvent *e);

    /*! Called when the appsrc needs more data. A buffer or EOS should be pushed
     * to appsrc from this thread or another thread. length is just a hint and when
     * it is set to -1, any number of bytes can be pushed into appsrc. */
    void needData(uint length) Q_DECL_OVERRIDE;

    /*! Called when appsrc has enough data. It is recommended that the application
     * stops calling pushBuffer() until the needData() method is called again to
     * avoid excessive buffer queueing. */
    void enoughData() Q_DECL_OVERRIDE;
};

} // namespace Rover
} // namespace Soro

#endif // FLYCAPSOURCE_H

#ifndef FLYCAPCAMERA_H
#define FLYCAPCAMERA_H

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
#include "flycapsource.h"

namespace Soro {
namespace Rover {

/* Manages a flycapture camera for use with gstreamer.
 */
class FlycapCamera : public QObject {
    Q_OBJECT

public:
    explicit FlycapCamera(FlyCapture2::PGRGuid guid, Logger *log = 0, QObject *parent = 0);
    ~FlycapCamera();

    const FlyCapture2::Camera* getCamera() const;
    int getVideoWidth() const;
    int getVideoHeight() const;
    int getFramerate() const;

    /* Creates a gstreamer source element for this camera that will output
     * the given framerate. A FlycapCamera can only feed once source at a time,
     * so calling this method while another source is still active means the
     * previous source will no longer receive data.
     */
    QGst::ElementPtr createSource(int framerate);

    /* Clears pointer references to the existing gstreamer source
     * and stops feeding it.
     */
    void clearSource();

private:
    QString LOG_TAG;
    Logger *_log = NULL;
    FlyCapture2::Camera _camera;
    FlycapSource *_source = NULL;
    int _captureTimerId = TIMER_INACTIVE;
    int _framerate = 0;
    int _width, _height;
    FlyCapture2::Image _imageBuffer;

protected:
    void timerEvent(QTimerEvent *e);
};

/* Connects a Point Grey Flycapture camera as a source in gstreamer.
 * This source can be created from a FlycapCamera object.
 */
class FlycapSource : public QGst::Utils::ApplicationSource {

    friend class FlycapCamera;

    ~FlycapSource();

private:
    FlycapSource(int width, int height, int framerate);
    bool NeedsData = false;

protected:
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

#endif // FLYCAPCAMERA_H
#ifndef FLYCAPCAMERA_H
#define FLYCAPCAMERA_H

#include <QObject>
#include <QCoreApplication>
#include <QTimerEvent>

#include <Qt5GStreamer/QGst/Buffer>
#include <Qt5GStreamer/QGst/Caps>
#include <Qt5GStreamer/QGst/Utils/ApplicationSource>
//#include <flycapture/FlyCapture2.h>
//#include <flycapture/C/FlyCapture2_C.h>

#include "logger.h"
#include "soro_global.h"

namespace Soro {
namespace Rover {

/* Manages a flycapture camera for use with gstreamer.
 */
class FlycapCamera : public QObject, public QGst::Utils::ApplicationSource  {
    Q_OBJECT

public:
    explicit FlycapCamera(FlyCapture2::PGRGuid guid, QObject *parent = 0);
    ~FlycapCamera();

private:
    bool _needsData = false;

    FlyCapture2::Camera _camera;
    int _captureTimerId;
    int _errorCount = 0;

    void stopWithError(QString error);

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

#endif // FLYCAPCAMERA_H

#ifndef FLYCAPSOURCE_H
#define FLYCAPSOURCE_H

#include <Qt5GStreamer/QGst/Utils/ApplicationSource>

namespace Soro {
namespace Rover {

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

#endif // FLYCAPSOURCE_H

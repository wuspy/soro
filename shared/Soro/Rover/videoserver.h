#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <QObject>

#include "socketaddress.h"
#include "soro_global.h"
#include "logger.h"
#include "mediaserver.h"

#include <flycapture/FlyCapture2.h>

namespace Soro {
namespace Rover {

/* Sends a video stream to a VideoClient by
 * implementing MediaServer
 */
class VideoServer: public MediaServer {
    Q_OBJECT
public:

    explicit VideoServer(int mediaId, SocketAddress host, Logger *log = 0, QObject *parent = 0);

    void start(QString deviceName, VideoFormat format);
    void start(FlyCapture2::PGRGuid camera, VideoFormat format);

    VideoFormat getVideoFormat() const;

private:
    VideoFormat _format;
    QString _videoDevice;
    bool _starting = false;

protected:
    /* Begins streaming video to the provided address.
     * This will fail if the stream is not in WaitingState
     */
    void constructChildArguments(QStringList& outArgs, SocketAddress host, SocketAddress address, quint16 ipcPort) Q_DECL_OVERRIDE;

    void onStreamStoppedInternal() Q_DECL_OVERRIDE;

    void constructStreamingMessage(QDataStream& stream) Q_DECL_OVERRIDE;
};

} // namespace Rover
} // namespace Soro

#endif // VIDEOSERVER_H

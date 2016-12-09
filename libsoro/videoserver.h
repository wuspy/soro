#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <QObject>

#include "soro_global.h"
#include "socketaddress.h"
#include "mediaserver.h"
#include "videoformat.h"

//#include <flycapture/FlyCapture2.h>

namespace Soro {

/**
 * Sends a video stream to a VideoClient by
 * implementing MediaServer
 */
class LIBSORO_EXPORT VideoServer: public MediaServer {
    Q_OBJECT
public:
    /**
     * @param mediaId Used to identify this particular media stream. Must match on both ends, and should be unique across
     * all media streams to prevent any problems if ports are not properly configured.
     * @param host Address for the socket that will be used to communicate with the media client.
     * @param parent
     */
    explicit VideoServer(int mediaId, SocketAddress host, QObject *parent = 0);

    /**
     * Starts a video stream. If the server is already streaming, it will be stopped and restarted to
     * accomodate any configuration changes.
     *
     * @param deviceName The video device to connect to and start streaming (/dev/video*)
     * @param format The video format to stream.
     */
    void start(QString deviceName, VideoFormat format);
    //void start(FlyCapture2::PGRGuid camera, VideoFormat format);

    VideoFormat getVideoFormat() const;

private:
    VideoFormat _format;
    QString _videoDevice;
    bool _starting = false;

protected:
    /**
     * Begins streaming video to the provided address.
     * This will fail if the stream is not in WaitingState
     */
    void constructChildArguments(QStringList& outArgs, SocketAddress host, SocketAddress address, quint16 ipcPort) Q_DECL_OVERRIDE;

    void onStreamStoppedInternal() Q_DECL_OVERRIDE;

    void constructStreamingMessage(QDataStream& stream) Q_DECL_OVERRIDE;
};

} // namespace Soro

#endif // VIDEOSERVER_H

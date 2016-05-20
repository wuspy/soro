#ifndef VIDEOSERVERARRAY_H
#define VIDEOSERVERARRAY_H

#include <QObject>
#include <QMap>
#include <QList>

#include "videoencoding.h"
#include "videoserver.h"
#include "flycapenumerator.h"
#include "uvdcameraenumerator.h"
#include "logger.h"

#include <flycapture/FlyCapture2.h>

namespace Soro {
namespace Rover {

class VideoServerArray : public QObject {
    Q_OBJECT
public:
    explicit VideoServerArray(Logger *log = 0, QObject *parent = 0);
    ~VideoServerArray();

    /* Starts streaming a camera at the
     * specified index
     */
    void activate(int index, StreamFormat format);

    /* Stops streaming a camera at the
     * specified index
     */
    void deactivate(int index);

    /* Scans the system for cameras and returns
     * the number of cameras found
     */
    int populate(const QStringList& uvdBlacklist, quint16 firstNetworkPort, int firstId);

    /* Gets the number of cameras connected
     */
    int cameraCount();

    /* Clears the array and all stops all servers
     */
    void clear();

    /* Removes a video server from the array
     */
    void remove(int index);

signals:
    void videoServerError(int index, QString error);
    void videoServerStateChanged(int index, VideoServer::State state);

private:
    Logger *_log;

    // These hold the video server objects which spawn child processes to
    // stream the data to mission control
    QMap<int, VideoServer*> _servers;
    // These hold the gst elements for the cameras that flycapture
    QMap<int, FlyCapture2::PGRGuid> _flycapCameras;
    // These hold the gst elements for the cameras that not flycapture
    QMap<int, QString> _uvdCameras;

private slots:
    void serverStateChanged(VideoServer *server, VideoServer::State state);
    void serverError(VideoServer *server, QString error);
};

}
}

#endif // VIDEOSERVERARRAY_H

#ifndef SORO_ROVER_VIDEOSERVERARRAY_H
#define SORO_ROVER_VIDEOSERVERARRAY_H

#include <QObject>
#include <QMap>
#include <QList>

#include "videoserver.h"
#include "flycapenumerator.h"
#include "uvdcameraenumerator.h"
#include "logger.h"

#include <flycapture/FlyCapture2.h>

namespace Soro {
namespace Rover {

/* Stores an array of VideoServer objects and populates
 * itself from available cameras
 */
class VideoServerArray : public QObject {
    Q_OBJECT
public:
    explicit VideoServerArray(Logger *log = 0, QObject *parent = 0);
    ~VideoServerArray();

    /* Starts streaming a video server at the specified index
     */
    void activate(int index, VideoFormat format);

    /* Stops streaming a video server at the specified index
     */
    void deactivate(int index);

    /* Creates video servers by scanning the system for cameras, and returns the number of cameras found.
     * This will not add any camera's matching names in the provided blacklist, and will assign the
     * new video servers contiguous port numbers and ID's starting with firstNetworkPort and firstId,
     * respectively.
     */
    int populate(const QStringList& uvdBlacklist, quint16 firstNetworkPort, int firstId);

    /* Gets the number of video servers currently contained in the array
     */
    int serverCount();

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
    void serverStateChanged(MediaServer *server, MediaServer::State state);
    void serverError(MediaServer *server, QString error);
};

}
}

#endif // SORO_ROVER_VIDEOSERVERARRAY_H

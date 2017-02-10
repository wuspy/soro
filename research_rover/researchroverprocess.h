#ifndef RESEARCHROVERPROCESS_H
#define RESEARCHROVERPROCESS_H

#include <QObject>

#include "libsoro/channel.h"
#include "libsoro/mbedchannel.h"
#include "libsoro/gpsserver.h"
#include "libsoro/audioserver.h"
#include "libsoro/nmeamessage.h"
#include "libsoro/videoserver.h"
#include "libsoro/videoformat.h"
#include "libsoro/enums.h"
#include "libsoro/sensordatarecorder.h"
#include "libsoro/gpsdatarecorder.h"
#include "libsoro/drivemessage.h"

namespace Soro {
namespace Rover {

class ResearchRoverProcess : public QObject
{
    Q_OBJECT
private:
    /* Connects to mission control for command and status communication
     */
    Channel *_driveChannel = nullptr;
    Channel *_sharedChannel = nullptr;

    /* Interfaces with the mbed controlling the drive system
     */
    MbedChannel *_mbed = nullptr;
    SensorDataRecorder _sensorRecorder;

    /* Provides GPS coordinates back to mission control
     */
    GpsServer *_gpsServer = nullptr;

    /* Provides audio back to mission control
     */
    AudioServer *_audioServer = nullptr;

    /* Handles video streaming from each individual camera
     */
    VideoServer *_stereoRCameraServer = nullptr;
    QString _stereoRCameraDevice;
    VideoServer *_stereoLCameraServer = nullptr;
    QString _stereoLCameraDevice;
    VideoServer *_aux1CameraServer = nullptr;
    QString _aux1CameraDevice;
    VideoServer *_monoCameraServer = nullptr;
    QString _monoCameraDevice;

    GpsDataRecorder _gpsRecorder;

private slots:
    void init();
    void sendSystemStatusMessage();
    void sharedChannelStateChanged(Channel::State state);
    void driveChannelStateChanged(Channel::State state);
    void mbedChannelStateChanged(MbedChannel::State state);
    void driveChannelMessageReceived(const char* message, Channel::MessageSize size);
    void sharedChannelMessageReceived(const char* message, Channel::MessageSize size);
    void mbedMessageReceived(const char* message, int size);
    void gpsUpdate(NmeaMessage message);
    void mediaServerError(MediaServer* server, QString message);
    bool startDataRecording(QDateTime startTime);
    void stopDataRecording();

public:
    explicit ResearchRoverProcess(QObject *parent = 0);
    ~ResearchRoverProcess();

signals:

public slots:
};


} // namespace Rover
} // namespace Soro

#endif // RESEARCHROVERPROCESS_H

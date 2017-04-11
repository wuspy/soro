#ifndef SORO_MISSIONCONTROL_RESEARCHCONTROLPROCESS_H
#define SORO_MISSIONCONTROL_RESEARCHCONTROLPROCESS_H

#include <QtCore>
#include <QMainWindow>
#include <QObject>
#include <QQmlEngine>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QQmlComponent>

#include <SDL2/SDL.h>

#include "libsoro/constants.h"
#include "libsoro/enums.h"
#include "libsoro/channel.h"
#include "libsoro/logger.h"
#include "libsoro/confloader.h"
#include "libsoro/drivemessage.h"
#include "libsoro/mbedchannel.h"
#include "libsoro/nmeamessage.h"
#include "libsoro/videoclient.h"
#include "libsoro/audioclient.h"
#include "libsoro/videoformat.h"
#include "libsoro/sensordataparser.h"
#include "libsoro/gpscsvseries.h"
#include "libsoro/csvrecorder.h"

#include "libsorogst/audioplayer.h"

#include "libsoromc/camerawidget.h"
#include "libsoromc/drivecontrolsystem.h"
#include "libsoromc/cameracontrolsystem.h"
#include "libsoromc/gstreamerrecorder.h"

#include "researchmainwindow.h"
#include "latencycsvseries.h"
#include "connectioneventcsvseries.h"
#include "commentcsvseries.h"
#include "settingsmodel.h"

namespace Soro {
namespace MissionControl {

class ResearchControlProcess : public QObject {
    Q_OBJECT
public:

    explicit ResearchControlProcess(QHostAddress roverAddress, GamepadManager *gamepad, QQmlEngine *qml, QObject *parent = 0);
    ~ResearchControlProcess();

private:
    SettingsModel _settings;
    bool _initialized = false;

    // The main UI components
    ResearchMainWindow *_mainUi = nullptr;
    QQuickWindow *_controlUi = nullptr;
    QQuickWindow *_commentsUi = nullptr;
    QQmlEngine *_qml = nullptr;

    // Communicates with the rover shared channel
    Channel *_roverChannel = nullptr;

    // Connects to the drive system on the rover
    DriveControlSystem *_driveSystem = nullptr;

    GamepadManager *_gamepad = nullptr;

    // Timer ID's
    int _pingTimerId = TIMER_INACTIVE;
    int _bitrateUpdateTimerId = TIMER_INACTIVE;

    VideoClient *_stereoLVideoClient = nullptr;
    VideoClient *_stereoRVideoClient = nullptr;
    VideoClient *_monoVideoClient = nullptr;
    VideoClient *_aux1VideoClient = nullptr;

    GStreamerRecorder *_stereoLGStreamerRecorder = nullptr;
    GStreamerRecorder *_stereoRGStreamerRecorder = nullptr;
    GStreamerRecorder *_monoGStreamerRecorder = nullptr;
    GStreamerRecorder *_aux1GStreamerRecorder = nullptr;

    // Audio stream subsystem
    AudioClient *_audioClient = nullptr;
    Soro::Gst::AudioPlayer *_audioPlayer = nullptr;
    GStreamerRecorder *_audioGStreamerRecorder = nullptr;

    SensorDataParser *_sensorDataSeries = nullptr;
    GpsCsvSeries *_gpsDataSeries = nullptr;
    ConnectionEventCsvSeries *_connectionEventSeries = nullptr;
    LatencyCsvSeries *_latencyDataSeries = nullptr;
    CommentCsvSeries *_commentDataSeries = nullptr;

    CsvRecorder *_dataRecorder = nullptr;
    qint64 _recordStartTime;

private:
    void stopAllRoverCameras();
    void startMonoCameraStream(VideoFormat format);
    void startStereoCameraStream(VideoFormat format);
    void startAux1CameraStream(VideoFormat format);
    void startAudioStream(AudioFormat format);
    void stopAudio();
    void startDataRecording();
    void stopDataRecording();
    void sendStartRecordCommandToRover();
    void sendStopRecordCommandToRover();

private slots:
    void init();
    void updateUiConnectionState();
    void roverSharedChannelMessageReceived(const char *message, Channel::MessageSize size);
    void videoClientStateChanged(MediaClient *client, MediaClient::State state);
    void audioClientStateChanged(MediaClient *client, MediaClient::State state);
    void driveConnectionStateChanged(Channel::State state);
    void gamepadChanged(bool connected, QString name);
    void roverDataRecordResponseWatchdog();
    void onQmlUiClosed();

    /**
     * Receives the signal from the UI when the settings have been applied and should be enacted
     */
    void ui_settingsApplied();

    /**
     * Receives the signal from the UI when it requests that the settings and information in its view state
     * be updated
     */
    void ui_requestUiSync();

    /**
     * Receives the signal from the UI when the test start/stop button is clicked
     */
    void ui_toggleDataRecordButtonClicked();

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SORO_MISSIONCONTROL_RESEARCHCONTROLPROCESS_H

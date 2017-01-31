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
#include "libsoro/mbeddataparser.h"
#include "libsoro/gpslogger.h"

#include "libsorogst/audioplayer.h"

#include "libsoromc/camerawidget.h"
#include "libsoromc/drivecontrolsystem.h"
#include "libsoromc/cameracontrolsystem.h"

#include "researchmainwindow.h"
#include "settingsmodel.h"

namespace Soro {
namespace MissionControl {

class ResearchControlProcess : public QObject {
    Q_OBJECT
public:

    explicit ResearchControlProcess(QHostAddress roverAddress, GamepadManager *gamepad, QQmlEngine *qml, QObject *parent = 0);

    ~ResearchControlProcess();

signals:
    void windowClosed();

private:
    SettingsModel _settings;

    // The main UI components
    ResearchMainWindow *_mainUi = NULL;
    QQuickWindow *_controlUi = NULL;

    // Communicates with the rover shared channel
    Channel *_roverChannel = NULL;

    // Connects to the drive system on the rover
    DriveControlSystem *_driveSystem = NULL;

    GamepadManager *_gamepad = NULL;

    // Timer ID's
    int _pingTimerId = TIMER_INACTIVE;
    int _bitrateUpdateTimerId = TIMER_INACTIVE;

    VideoClient *_stereoLVideoClient = NULL;
    VideoClient *_stereoRVideoClient = NULL;
    VideoClient *_monoVideoClient = NULL;
    VideoClient *_aux1VideoClient = NULL;

    // Audio stream subsystem
    AudioClient *_audioClient = NULL;
    Soro::Gst::AudioPlayer *_audioPlayer = NULL;

    MbedDataParser _mbedParser;
    GpsLogger _gpsLogger;

private:
    void stopAllRoverCameras();
    void startMonoCameraStream(VideoFormat format);
    void startStereoCameraStream(VideoFormat format);
    void startAux1CameraStream(VideoFormat format);
    void startAudioStream(AudioFormat format);
    void stopAudio();

private slots:
    void updateUiConnectionState();
    void roverSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void videoClientStateChanged(MediaClient *client, MediaClient::State state);
    void audioClientStateChanged(MediaClient *client, MediaClient::State state);
    void driveConnectionStateChanged(Channel::State state);
    void gamepadChanged(SDL_GameController *controller, QString name);
    void newSensorData(MbedDataParser::DataTag tag, float value);
    void startTestLog();
    void stopTestLog();

    /**
     * Receives the signal from the UI when the settings have been applied and should be enacted
     */
    void ui_settingsApplied();

    /**
     * Receives the signal from the UI when it requests that the settings and information in its view state
     * be updated
     */
    void ui_requestUiSync();

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SORO_MISSIONCONTROL_RESEARCHCONTROLPROCESS_H

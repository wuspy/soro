#ifndef SORO_MISSIONCONTROL_SOROWINDOWCONTROLLER_H
#define SORO_MISSIONCONTROL_SOROWINDOWCONTROLLER_H

#include <QtCore>
#include <QMainWindow>
#include <QObject>
#include <QQmlEngine>
#include <QQmlEngine>

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

#include "libsorogst/audioplayer.h"

#include "libsoromc/camerawidget.h"
#include "libsoromc/drivecontrolsystem.h"
#include "libsoromc/cameracontrolsystem.h"

#include "researchmainwindow.h"
#include "settingsform.h"


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
    QHostAddress _roverAddress;

    //the main UI components
    ResearchMainWindow *_mainUI = NULL;
    SettingsForm *_settingsUI = NULL;

    // Communicates with the rover shared channel
    Channel *_roverChannel = NULL;

    // Connects to the drive system on the rover
    DriveControlSystem *_driveSystem = NULL;

    GamepadManager *_gamepad = NULL;

    VideoClient *_stereoLVideoClient = NULL;
    VideoClient *_stereoRVideoClient = NULL;
    VideoClient *_monoVideoClient = NULL;
    VideoClient *_aux1VideoClient = NULL;

    // Audio stream subsystem
    AudioClient *_audioClient = NULL;
    Soro::Gst::AudioPlayer *_audioPlayer = NULL;
private:
    void stopAllRoverCameras();

private slots:
    void roverSharedChannelStateChanged(Channel *channel, Channel::State state);
    void roverSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void videoClientStateChanged(MediaClient *client, MediaClient::State state);
    void audioClientStateChanged(MediaClient *client, MediaClient::State state);

    /**
     * Receives the signal from the UI when a new mono camera format is selected
     */
    void ui_monoCameraFormatSelected(VideoFormat format);
    /**
     * Receives the signal from the UI when a new stereo camera format is selected
     */
    void ui_stereoCameraFormatSelected(VideoFormat format);
    /**
     * Receives the signal from the UI when a new format is selected for aux camera 1
     */
    void ui_aux1CameraFormatSelected(VideoFormat format);
    /**
     * Receives the signal from the UI when a new audio stream format is selected
     */
    void ui_audioStreamFormatSelected(AudioFormat format);

    void gamepadPoll();

};

}
}

#endif // SORO_MISSIONCONTROL_SOROWINDOWCONTROLLER_H

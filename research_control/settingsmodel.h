#ifndef SORO_MISSIONCONTROL_SETTINGSMODEL_H
#define SORO_MISSIONCONTROL_SETTINGSMODEL_H

#include <QtCore>
#include <QQuickWindow>
#include <QHostAddress>

#include "libsoro/videoformat.h"
#include "libsoro/audioformat.h"
#include "libsoro/constants.h"

namespace Soro {
namespace MissionControl {

struct SettingsModel {

    bool enableStereoUi;
    bool enableVideo;
    bool enableStereoVideo;
    bool enableAudio;
    bool enableHud;
    bool enableGps;

    int selectedVideoEncoding;
    int selectedVideoResolution;
    int selectedVideoFramerate;
    int selectedVideoBitrate;
    int selectedMjpegQuality;
    int selectedCamera;
    int selectedLatency;
    int selectedHudParallax;
    int selectedHudLatency;

    int mainCameraIndex;
    int aux1CameraIndex;

    QHostAddress roverAddress;

    QStringList cameraNames;
    //QList<VideoFormat> defaultVideoFormats;
    AudioFormat defaultAudioFormat;

    static SettingsModel Default(QHostAddress roverAddress);

    void syncUi(QQuickWindow *window);
    void syncModel(const QQuickWindow *window);
    void setSelectedCamera(int mediaId);
    VideoFormat getSelectedVideoFormat();
};

} // namespace MissionControl
} // namespace Soro

#endif // SORO_MISSIONCONTROL_SETTINGSMODEL_H

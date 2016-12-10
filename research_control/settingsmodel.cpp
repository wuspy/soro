#include "settingsmodel.h"

namespace Soro {
namespace MissionControl {

SettingsModel SettingsModel::Default(QHostAddress roverAddress) {
    SettingsModel model;
    model.roverAddress = roverAddress;
    model.enableStereoUi = false;
    model.enableHud = true;
    model.enableVideo = false;
    model.enableStereoVideo = false;
    model.enableAudio = false;
    model.enableGps = true;
    model.selectedVideoFormat = 0;
    model.selectedCamera = 0;
    model.selectedLatency = 0;

    // Define media formats
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1920x1080, 12000000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1920x1080, 8000000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1280x720, 5000000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1152x648, 3000000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1024x576, 1500000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_640x360, 750000, 0, VideoFormat::StereoMode_None, 3));

    model.defaultAudioFormat = AudioFormat(AudioFormat::Encoding_AC3, 32000);

    model.cameraNames << "Main Camera [Mono/Stereo]";   model.mainCameraIndex = 0;
    model.cameraNames << "Fisheye Camera [Mono]";       model.aux1CameraIndex = 1;

    return model;
}

void SettingsModel::syncUi(QQuickWindow *window) {
    // Prepare the view to be synced
    QMetaObject::invokeMethod(window, "prepareForUiSync");

    QStringList formatNames;
    foreach (VideoFormat format, defaultVideoFormats) {
        formatNames << format.toHumanReadableString();
    }
    window->setProperty("videoFormatNames", formatNames);
    window->setProperty("cameraNames", cameraNames);
    window->setProperty("roverAddress", roverAddress.toString());

    window->setProperty("enableStereoUi", enableStereoUi);
    window->setProperty("enableHud", enableHud);
    window->setProperty("enableVideo", enableVideo);
    window->setProperty("enableStereoVideo", enableStereoVideo);
    window->setProperty("enableAudio", enableAudio);
    window->setProperty("enableGps", enableGps);
    window->setProperty("selectedVideoFormat", selectedVideoFormat);
    window->setProperty("selectedCamera", selectedCamera);
    window->setProperty("selectedLatency", selectedLatency);

    // Inform the view that the settings have been synced
    QMetaObject::invokeMethod(window, "uiSyncComplete");
}

void SettingsModel::syncModel(const QQuickWindow *window) {
    enableStereoUi = window->property("enableStereoUi").toBool();
    enableHud = window->property("enableHud").toBool();
    enableVideo = window->property("enableVideo").toBool();
    enableStereoVideo = window->property("enableStereoVideo").toBool();
    enableAudio = window->property("enableAudio").toBool();
    enableGps = window->property("enableGps").toBool();
    selectedVideoFormat = window->property("selectedVideoFormat").toInt();
    selectedCamera = window->property("selectedCamera").toInt();
    selectedLatency = window->property("selectedLatency").toInt();
}

void SettingsModel::setSelectedCamera(int mediaId) {
    switch (mediaId) {
    case MEDIAID_RESEARCH_A1_CAMERA:
        selectedCamera = aux1CameraIndex;
        break;
    case MEDIAID_RESEARCH_M_CAMERA:
    case MEDIAID_RESEARCH_SL_CAMERA:
    case MEDIAID_RESEARCH_SR_CAMERA:
        selectedCamera = mainCameraIndex;
        break;
    default:
        selectedCamera = 0;
        enableVideo = 0;
        enableStereoVideo = 0;
        break;
    }
}

VideoFormat SettingsModel::getSelectedVideoFormat() {
    if ((selectedVideoFormat >= 0) && (selectedVideoFormat < defaultVideoFormats.length())) {
        return defaultVideoFormats[selectedVideoFormat];
    }
    return VideoFormat();
}

} // namespace MissionControl
} // namespace Soro

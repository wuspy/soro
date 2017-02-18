/*
 * Copyright 2017 The University of Oklahoma.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
    model.selectedHudParallax = 0;
    model.selectedHudLatency = 100;

    // Define media formats
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1280x720, 8000000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1280x720, 5000000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1152x648, 3000000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_1024x576, 1500000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_640x360, 750000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_432_240, 400000, 0, VideoFormat::StereoMode_None, 3));
    model.defaultVideoFormats.append(VideoFormat(VideoFormat::Encoding_MPEG2, VideoFormat::Resolution_176_144, 100000, 0, VideoFormat::StereoMode_None, 3));

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
    window->setProperty("selectedHudParallax", selectedHudParallax);
    window->setProperty("selectedHudLatency", selectedHudLatency);

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
    selectedHudParallax = window->property("selectedHudParallax").toInt();
    selectedHudLatency = window->property("selectedHudLatency").toInt();
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

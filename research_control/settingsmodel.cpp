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
    model.selectedVideoEncoding = 0;
    model.selectedVideoResolution = 0;
    model.selectedVideoBitrate = 500;
    model.selectedVideoFramerate = 0;
    model.selectedMjpegQuality = 50;
    model.selectedCamera = 0;
    model.selectedLatency = 0;
    model.selectedHudParallax = 0;
    model.selectedHudLatency = 100;

    model.defaultAudioFormat = AudioFormat(AudioFormat::Encoding_AC3, 32000);

    model.cameraNames << "Main Camera [Mono/Stereo]";   model.mainCameraIndex = 0;
    model.cameraNames << "Fisheye Camera [Mono]";       model.aux1CameraIndex = 1;

    model.videoResolutionNames << "176x144";
    model.videoResolutionNames << "432x240";
    model.videoResolutionNames << "640x360";
    model.videoResolutionNames << "1024x576";
    model.videoResolutionNames << "1152x648";
    model.videoResolutionNames << "1280x720";
    model.videoResolutionNames << "1600x900";

    model.videoEncodingNames << "MP4";
    model.videoEncodingNames << "MJPEG";
    model.videoEncodingNames << "H264";
    model.videoEncodingNames << "VP8";
    model.videoEncodingNames << "H265";

    return model;
}

void SettingsModel::syncUi(QQuickWindow *window) {
    // Prepare the view to be synced
    QMetaObject::invokeMethod(window, "prepareForUiSync");

    window->setProperty("videoEncodingNames", videoEncodingNames);
    window->setProperty("videoResolutionNames", videoResolutionNames);
    window->setProperty("cameraNames", cameraNames);
    window->setProperty("roverAddress", roverAddress.toString());

    window->setProperty("enableStereoUi", enableStereoUi);
    window->setProperty("enableHud", enableHud);
    window->setProperty("enableVideo", enableVideo);
    window->setProperty("enableStereoVideo", enableStereoVideo);
    window->setProperty("enableAudio", enableAudio);
    window->setProperty("enableGps", enableGps);
    window->setProperty("selectedVideoEncoding", selectedVideoEncoding);
    window->setProperty("selectedVideoResolution", selectedVideoResolution);
    window->setProperty("selectedVideoFramerate", selectedVideoFramerate);
    window->setProperty("selectedVideoBitrate", selectedVideoBitrate);
    window->setProperty("selectedMjpegQuality", selectedMjpegQuality);
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
    selectedVideoEncoding = window->property("selectedVideoEncoding").toInt();
    selectedVideoResolution = window->property("selectedVideoResolution").toInt();
    selectedVideoFramerate = window->property("selectedVideoFramerate").toInt();
    selectedVideoBitrate = window->property("selectedVideoBitrate").toInt();
    selectedMjpegQuality = window->property("selectedMjpegQuality").toInt();
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
    VideoFormat format;
    format.setEncoding(static_cast<VideoFormat::Encoding>(selectedVideoEncoding));
    format.setResolution(static_cast<VideoFormat::Resolution>(selectedVideoResolution));
    format.setBitrate(selectedVideoBitrate * 1000);
    format.setFramerate(selectedVideoFramerate);
    format.setMjpegQuality(selectedMjpegQuality);
    format.setMaxThreads(3);
    return format;
}

} // namespace MissionControl
} // namespace Soro

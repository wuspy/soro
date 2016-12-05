/*
 * Copyright 2016 The University of Oklahoma.
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

/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#ifndef SORO_ENUMS_H
#define SORO_ENUMS_H

/**
 * Shared messages are passed between the rover and mission control. Values in this enum
 * are always appended as a header to these messages to indicate what kind of data they store.
 */
enum SharedMessageType {
    SharedMessage_RoverSharedChannelStateChanged = 1,
    SharedMessage_RoverStatusUpdate,
    SharedMessage_RoverGpsUpdate,
    SharedMessage_MissionControlConnected,
    SharedMessage_MissionControlDisconnected,
    SharedMessage_RequestActivateCamera,
    SharedMessage_RequestDeactivateCamera,
    SharedMessage_RoverVideoServerError,
    SharedMessage_MissionControlChat,
    SharedMessage_CameraChanged,
    SharedMessage_BitrateUpdate,
    SharedMessage_CameraNameChanged,
    SharedMessage_RequestActivateAudioStream,
    SharedMessage_RequestDeactivateAudioStream,
    SharedMessage_AudioStreamChanged,
    SharedMessage_Research_RoverStatusUpdate,
    SharedMessage_Research_SensorUpdate,
    SharedMessage_Research_StartStereoCameraStream,
    SharedMessage_Research_StartMonoCameraStream,
    SharedMessage_Research_EndStereoAndMonoCameraStream,
    SharedMessage_Research_StartAux1CameraStream,
    SharedMessage_Research_EndAux1CameraStream
};

enum VideoFormat {
    VideoFormat_Null = 0,
    Mpeg2_144p_300Kpbs,
    Mpeg2_360p_750Kpbs,
    Mpeg2_480p_1500Kpbs,
    Mpeg2_720p_3000Kpbs,
    Mpeg2_720p_5000Kpbs,
    Mpeg2_360p_500Kbps_BW
};

enum AudioFormat {
    AudioFormat_Null = 0,
    AC3
};

enum RoverSubsystemState {
    NormalSubsystemState, MalfunctionSubsystemState, UnknownSubsystemState
};

enum RoverCameraState {
    StreamingCameraState, DisabledCameraState, UnavailableCameraState
};

enum DriveGamepadMode {
    SingleStickDrive, DualStickDrive
};

/**
 * Indicates a role a single mission control instance can fill
 */
enum Role {
    ArmOperatorRole, DriverRole, CameraOperatorRole, SpectatorRole, ResearchRole
};

enum NotificationType {
    RoverNotification, MCCNotification, ChatNotification
};

enum MbedMessageType {
    MbedMessage_ArmMaster = 1,
    MbedMessage_ArmGamepad,
    MbedMessage_Drive,
    MbedMessage_Gimbal
};

#endif // SORO_ENUMS_H

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

#include <QtCore>

#ifndef SORO_ENUMS_H
#define SORO_ENUMS_H

/**
 * Shared messages are passed between the rover and mission control. Values in this enum
 * are always appended as a header to these messages to indicate what kind of data they store.
 */
enum SharedMessageType {
    // Start at a high bit number to vastly reduce the chance of mix-ups
    SharedMessage_RoverSharedChannelStateChanged = 1000000000,
    SharedMessage_RoverStatusUpdate,
    SharedMessage_RoverGpsUpdate,
    SharedMessage_MissionControlConnected,
    SharedMessage_MissionControlDisconnected,
    SharedMessage_RequestActivateCamera,
    SharedMessage_RequestDeactivateCamera,
    SharedMessage_RoverMediaServerError,
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
    SharedMessage_Research_EndAux1CameraStream,
    SharedMessage_Research_RoverDriveOverrideStart,
    SharedMessage_Research_RoverDriveOverrideEnd,
    SharedMessage_Research_TestStart,
    SharedMessage_Research_TestEnd
};

enum RoverSubsystemState {
    NormalSubsystemState = 1000000100, MalfunctionSubsystemState, UnknownSubsystemState
};

enum RoverCameraState {
    StreamingCameraState = 1000000200, DisabledCameraState, UnavailableCameraState
};

enum DriveGamepadMode {
    SingleStickDrive = 1000000300, DualStickDrive
};

/**
 * Indicates a role a single mission control instance can fill
 */
enum Role {
    ArmOperatorRole = 1000000400, DriverRole, CameraOperatorRole, SpectatorRole, ResearchRole
};

enum MbedMessageType {
    // These MUST stay in 8-bit range
    MbedMessage_ArmMaster = 1,
    MbedMessage_ArmGamepad,
    MbedMessage_Drive,
    MbedMessage_Gimbal
};

#endif // SORO_ENUMS_H

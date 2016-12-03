/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#ifndef SORO_ENUMS_H
#define SORO_ENUMS_H

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
    SharedMessage_AudioStreamChanged
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

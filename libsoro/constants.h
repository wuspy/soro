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

#ifndef SORO_CONSTANTS_H
#define SORO_CONSTANTS_H

#define TIMER_INACTIVE -1
#define START_TIMER(X,Y) if (X == TIMER_INACTIVE) X = startTimer(Y)
#define KILL_TIMER(X) if (X != TIMER_INACTIVE) { killTimer(X); X = TIMER_INACTIVE; }

#define IPV4_REGEX "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
#define IPV6_REGEX "(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))"

// Channel names, must be the same on the rover and mission control builds
#define CHANNEL_NAME_ARM "Soro_ArmChannel"
#define CHANNEL_NAME_DRIVE "Soro_DriveChannel"
#define CHANNEL_NAME_GIMBAL "Soro_GimbalChannel"
#define CHANNEL_NAME_SHARED "Soro_SharedTcpChannel"
#define CHANNEL_NAME_SECONDARY_COMPUTER "Soro_SecondaryComputerChannel"

#define SECONDARY_COMPUTER_BROADCAST_STRING "Soro_SecondaryComputer"
#define MASTER_COMPUTER_BROADCAST_STRING "Soro_MasterComputer"

#define STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS 91
#define STREAMPROCESS_ERR_INVALID_ARGUMENT 92
#define STREAMPROCESS_ERR_GSTREAMER_EOS 93
#define STREAMPROCESS_ERR_GSTREAMER_ERROR 94
#define STREAMPROCESS_ERR_UNKNOWN_CODEC 95
#define STREAMPROCESS_ERR_FLYCAP_ERROR 96
#define STREAMPROCESS_ERR_SOCKET_ERROR 97

#define STREAMPROCESS_IPC_START 's'
#define STREAMPROCESS_IPC_STREAMING 'v'
#define STREAMPROCESS_IPC_EXIT 'e'

#define NETWORK_ALL_ARM_CHANNEL_PORT 5508
#define NETWORK_ALL_DRIVE_CHANNEL_PORT 5509
#define NETWORK_ALL_GIMBAL_CHANNEL_PORT 5510
#define NETWORK_ALL_SHARED_CHANNEL_PORT 5511
#define NETWORK_ALL_AUDIO_PORT 5512
#define NETWORK_ALL_CAMERA_PORT_1 5520
#define NETWORK_MC_MASTER_ARM_PORT 5513
#define NETWORK_MC_BROADCAST_PORT 5514
#define NETWORK_ROVER_COMPUTER2_PORT 5515
#define NETWORK_ROVER_ARM_MBED_PORT 5516
#define NETWORK_ROVER_DRIVE_MBED_PORT 5517
#define NETWORK_ROVER_GPS_PORT 5518

#define MAX_CAMERAS 20

//TODO possibly make this configurable
#define GAMEPAD_POLL_INTERVAL 50
#define GAMEPAD_DEADZONE 0.2

// Mbed ID's
#define MBED_ID_MASTER_ARM 1
#define MBED_ID_ARM 2
#define MBED_ID_DRIVE_CAMERA 3
#define MBED_ID_RESEARCH 4

#endif // SORO_CONSTANTS_H

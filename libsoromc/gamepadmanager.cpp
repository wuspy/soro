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

#include "gamepadmanager.h"

#define LOG_TAG "GamepadManager"

namespace Soro {
namespace MissionControl {

GamepadManager::GamepadManager(QObject *parent) : QObject(parent) { }

bool GamepadManager::init(int interval, QString *error) {
    if (_sdlInitialized) {
        SDL_Quit();
    }
    KILL_TIMER(_inputSelectorTimerId);
    KILL_TIMER(_updateTimerId);
    _interval = interval;
    _sdlInitialized = SDL_Init(SDL_INIT_GAMECONTROLLER) == 0;
    if (_sdlInitialized) {
        // Add gamepad map
        QFile gamepadMap(":/libsoromc_assets/gcdb.txt"); // Loads from libsoromc_assets.qrc
        gamepadMap.open(QIODevice::ReadOnly);
        while (gamepadMap.bytesAvailable()) {
            if (SDL_GameControllerAddMapping(gamepadMap.readLine().data()) == -1) {
                if (error) *error = "Unable to apply SDL gamepad map";
                gamepadMap.close();
                return false;
            }
        }
        gamepadMap.close();
        START_TIMER(_inputSelectorTimerId, 1000);
        return true;
    }
    else {
        if (error) *error = "SDL failed to initialize: " + QString(SDL_GetError());
    }
    return false;
}

GamepadManager::~GamepadManager() {
    SDL_Quit();
}

SDL_GameController* GamepadManager::getGamepad() {
    return _gameController;
}

QString GamepadManager::getGamepadName() const {
    return _gamepadName;
}

bool GamepadManager::isGamepadConnected() const {
    return _gameController != nullptr;
}

void GamepadManager::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _inputSelectorTimerId) {
        /***************************************
         * This timer querys SDL at regular intervals to look for a
         * suitable controller
         */
        SDL_GameControllerUpdate();
        LOG_I(LOG_TAG, "Looking for usable joysticks...");
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            char guid_str[256];
            SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(i);
            SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
            LOG_I(LOG_TAG, QString("Considering joystick %1 with GUID %2...").arg(QString::number(i), QString(guid_str)));
            if (SDL_IsGameController(i)) {
                LOG_I(LOG_TAG, QString("Joystick %1 is a game controller").arg(i));
                SDL_GameController *controller = SDL_GameControllerOpen(i);
                if (controller) {
                    //this gamepad will do
                    setGamepad(controller);
                    KILL_TIMER(_inputSelectorTimerId);
                    START_TIMER(_updateTimerId, _interval);
                    break;
                }
                SDL_GameControllerClose(controller);
            }
        }
    }
    else if (e->timerId() == _updateTimerId) {
        SDL_GameControllerUpdate();
        if (_gameController && SDL_GameControllerGetAttached(_gameController)) {
            // Update gamepad data
            axisLeftX           = SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX);
            axisLeftY           = SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY);
            axisRightX          = SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_RIGHTX);
            axisRightY          = SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_RIGHTY);
            axisLeftTrigger     = SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
            axisRightTrigger    = SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
            buttonA             = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_A);
            buttonB             = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_B);
            buttonX             = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_X);
            buttonY             = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_Y);
            buttonLeftShoulder  = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            buttonRightShoulder = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            buttonStart         = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_START);
            buttonBack          = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_BACK);
            buttonLeftStick     = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_LEFTSTICK);
            buttonRightStick    = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
            dpadUp              = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_DPAD_UP);
            dpadLeft            = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            dpadDown            = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            dpadRight           = SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            emit poll();
        }
        else {
            // Controller is no longer attached
            setGamepad(nullptr);
            KILL_TIMER(_updateTimerId);
            START_TIMER(_inputSelectorTimerId, 1000);
            LOG_I(LOG_TAG, "The gamepad has been disconnected");
        }
    }
}

void GamepadManager::setGamepad(SDL_GameController *controller) {
    if (_gameController != controller) {
        _gameController = controller;
        _gamepadName = _gameController ? QString(SDL_GameControllerName(_gameController)) : "";
        if (controller) {
            LOG_I(LOG_TAG, "Active controller is \'" + _gamepadName + "\'");
        }
        else {
            LOG_W(LOG_TAG, "No usable controller connected");
        }
        emit gamepadChanged(isGamepadConnected(), _gamepadName);
    }
}

}
}

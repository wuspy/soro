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

#include <QApplication>
#include <QProcessEnvironment>
#include <QQmlApplicationEngine>
#include <QtWebEngine/qtwebengineglobal.h>
#include <QQuickStyle>

#include <Qt5GStreamer/QGst/Init>

#include "libsoro/constants.h"
#include "libsoromc/gamepadmanager.h"
#include "researchprocess.h"

#define LOG_TAG "Main"

using namespace Soro;
using namespace Soro::MissionControl;

int main(int argc, char *argv[]) {
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    QGst::init();
    QtWebEngine::initialize();

    // set root log output file
    Logger::rootLogger()->setLogfile(QCoreApplication::applicationDirPath()
                                     + "/../log/RoverControl_" + QDateTime::currentDateTime().toString("M-dd_h.mm_AP") + ".log");
    Logger::rootLogger()->setMaxFileLevel(Logger::LogLevelDebug);
    Logger::rootLogger()->setMaxQtLoggerLevel(Logger::LogLevelInformation);

    /*
     * Get rover IP from envvar
     */

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString roverIP = env.value("SORO_ROVER_IP");

    if (roverIP.isEmpty()) {
        LOG_E(LOG_TAG, "Envvar SORO_ROVER_IP is not set");
        return 1;
    }
    LOG_I(LOG_TAG, "SORO_ROVER_IP=" + roverIP);
    if (!QRegExp(IPV4_REGEX).exactMatch(roverIP) && !QRegExp(IPV6_REGEX).exactMatch(roverIP)) {
        LOG_E(LOG_TAG, "Rover IP is not a valid address");
        return 1;
    }

    /*
     * Init gamepad manager
     */

    GamepadManager gamepad(&a);
    QString err;
    if (!gamepad.init(GAMEPAD_POLL_INTERVAL, &err)) {
        LOG_E(LOG_TAG, "Cannot init gamepad: " + err);
        return 1;
    }

    QQmlEngine engine(&a);
    QQuickStyle::setStyle("Material");
    ResearchControlProcess process(QHostAddress(roverIP), &gamepad, &engine, &a);

    return a.exec();
}

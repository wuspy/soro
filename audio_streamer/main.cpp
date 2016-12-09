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

#include <QCoreApplication>
#include <QByteArray>
#include <QDataStream>

#include <Qt5GStreamer/QGst/Init>

#include "libsoro/socketaddress.h"
#include "libsoro/audioformat.h"
#include "libsoro/constants.h"
#include "libsoro/logger.h"

#include "audiostreamer.h"

#define LOG_TAG "Main"

using namespace Soro;
using namespace Soro::Rover;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Logger::rootLogger()->setLogfile(QCoreApplication::applicationDirPath()
                                     + "/../log/Audio_" + QDateTime::currentDateTime().toString("M-dd_h.mm.ss_AP") + ".log");
    Logger::rootLogger()->setMaxFileLevel(Logger::LogLevelDebug);
    Logger::rootLogger()->setMaxQtLoggerLevel(Logger::LogLevelDisabled);

    LOG_I(LOG_TAG, "Starting...");
    QGst::init();

    if (argc < 8) {
        LOG_E(LOG_TAG, "Not enough arguments (expected 8, got " + QString::number(argc) + ")");
        return STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS;
    }

    bool ok;
    AudioFormat format;
    QString device;
    SocketAddress address;
    SocketAddress bindAddress;
    quint16 ipcPort;

    /*
     * Parse device
     */
    device = argv[1];
    LOG_I(LOG_TAG, "Device: " + device);

    /*
     * Parse audio format
     */
    QString formatSerial = QString(argv[2]);
    LOG_I(LOG_TAG, "Format: " + formatSerial);
    format.deserialize(formatSerial);

    /*
     * Parse address
     */
    address.host = QHostAddress(argv[3]);
    address.port = QString(argv[4]).toInt(&ok);
    if ((address.host == QHostAddress::Null) | (address.host == QHostAddress::Any) | !ok) {
        // invalid address
        LOG_E(LOG_TAG, "Invalid address '" + QString(argv[3]) + ":" + QString(argv[4]) + "'");
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }
    LOG_I(LOG_TAG, "Address: " + address.toString());

    /*
     * Parse bind address
     */
    bindAddress.host = QHostAddress(argv[5]);
    bindAddress.port = QString(argv[6]).toInt(&ok);
    if ((bindAddress.host == QHostAddress::Null) | !ok) {
        // invalid host
        LOG_E(LOG_TAG, "Invalid bind address '" + QString(argv[5]) + ":" + QString(argv[6]) + "'");
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }
    LOG_I(LOG_TAG, "Bind Address: " + address.toString());

    /*
     * Parse IPC port
     */
    ipcPort = QString(argv[7]).toInt(&ok);
    if (!ok) {
        // invalid IPC port
        LOG_E(LOG_TAG, "Invalid IPC port '" + QString(argv[7]) + "'");
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }
    LOG_I(LOG_TAG, "IPC Port: " + QString::number(ipcPort));

    a.setApplicationName("AudioStream for " + device + " to " + address.toString());

    LOG_I(LOG_TAG, "Creating stream object");
    AudioStreamer stream(device, format, bindAddress, address, ipcPort, &a);
    LOG_I(LOG_TAG, "Stream object created");
    return a.exec();
}

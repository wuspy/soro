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

//#include <flycapture/FlyCapture2.h>

#include <Qt5GStreamer/QGst/Init>
#include <Qt5GStreamer/QGst/Element>
#include <Qt5GStreamer/QGst/ElementFactory>

#include "libsoro/socketaddress.h"
#include "libsoro/enums.h"
#include "libsoro/constants.h"
#include "libsoro/logger.h"
//#include "libsoro/flycapcamera.h"

#include "videostreamer.h"

#define LOG_TAG "Main"

using namespace Soro;
using namespace Soro::Rover;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Logger::rootLogger()->setLogfile(QCoreApplication::applicationDirPath()
                                     + "/../log/Video_" + QDateTime::currentDateTime().toString("M-dd_h.mm.ss_AP") + ".log");
    Logger::rootLogger()->setMaxFileLevel(Logger::LogLevelDebug);
    Logger::rootLogger()->setMaxStdoutLevel(Logger::LogLevelDisabled);

    LOG_I(LOG_TAG, "Starting...");
    QGst::init();

    if (argc < 8) {
        LOG_E(LOG_TAG, "Not enough arguments (expected 8, got " + QString::number(argc) + ")");
        return STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS;
    }

    bool ok;
    VideoFormat format;
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
     * Parse format
     */
    QString formatSerial = QString(argv[2]);
    LOG_I(LOG_TAG, "Format: " + formatSerial);
    format.deserialize(formatSerial);

    /*
     * Parse destination address
     */
    address.host = QHostAddress(argv[3]);
    address.port = QString(argv[4]).toInt(&ok);
    if ((address.host == QHostAddress::Null) | (address.host == QHostAddress::Any) | !ok) {
        LOG_E(LOG_TAG, "Invalid address '" + QString(argv[3]) + ":" + QString(argv[4]) + "'");
        // invalid address
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
     * Parse IPC Port
     */
    ipcPort = QString(argv[7]).toInt(&ok);
    if (!ok) {
        // invalid IPC port
        LOG_E(LOG_TAG, "Invalid IPC port '" + QString(argv[7]) + "'");
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }
    LOG_I(LOG_TAG, "IPC Port: " + QString::number(ipcPort));

    a.setApplicationName("VideoStream for " + device + " to " + address.toString());

    /*if (device.startsWith("FlyCapture2:", Qt::CaseInsensitive)) {
        QGst::ElementPtr source;
        // parse GUID
        FlyCapture2::PGRGuid guid;
        device = device.mid(device.indexOf(":") + 1);
        guid.value[0] = device.mid(0, device.indexOf(":")).toUInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        device = device.mid(device.indexOf(":") + 1);
        guid.value[1] = device.mid(0, device.indexOf(":")).toUInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        device = device.mid(device.indexOf(":") + 1);
        guid.value[2] = device.mid(0, device.indexOf(":")).toUInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        device = device.mid(device.indexOf(":") + 1);
        guid.value[3] = device.toUInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;

        FlycapCamera camera(guid, &a);
        source = camera.element();

        LOG_I(LOG_TAG, "Parset parameters for FlyCapture successfully");
        VideoStreamer stream(source, format, bindAddress, address, ipcPort, &a);
        LOG_I(LOG_TAG, "Stream initialized for FlyCapture successfully");
        return a.exec();
    }
    else {*/
        LOG_I(LOG_TAG, "Creating stream object");
        VideoStreamer stream(device, format, bindAddress, address, ipcPort, &a);
        LOG_I(LOG_TAG, "Stream object created");
        return a.exec();
    //}
}

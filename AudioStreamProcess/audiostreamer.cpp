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

#include "audiostreamer.h"

namespace Soro {
namespace Rover {

AudioStreamer::AudioStreamer(QString sourceDevice, AudioFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent)
        : MediaStreamer(parent) {
    if (!connectToParent(ipcPort)) return;

    _pipeline = createPipeline();

    // create gstreamer command
    QString binStr = makeEncodingBinString(format, bindAddress, address);
#ifdef __linux__
    binStr = "alsasrc device=" + sourceDevice + " ! " + binStr;
#endif
    QGst::BinPtr encoder = QGst::Bin::fromDescription(binStr);

    qDebug() << "Created gstreamer bin " << binStr;

    _pipeline->add(encoder);

    // play
    _pipeline->setState(QGst::StatePlaying);

    qDebug() << "Stream started";

}

QString AudioStreamer::makeEncodingBinString(AudioFormat format, SocketAddress bindAddress, SocketAddress address) {
    QString binStr = "audioconvert ! audio/x-raw,rate=32000 ! ";
    switch (format) {
    case AC3:
        binStr += "avenc_ac3 ! rtpac3pay ! ";
        break;
    default:
        //unknown codec
        QCoreApplication::exit(STREAMPROCESS_ERR_UNKNOWN_CODEC);
        return "";
    }

    binStr += "udpsink bind-address=" + bindAddress.host.toString() + " bind-port=" + QString::number(bindAddress.port)
                    + " host=" + address.host.toString() + " port=" + QString::number(address.port);

    return binStr;
}

} // namespace Rover
} // namespace Soro


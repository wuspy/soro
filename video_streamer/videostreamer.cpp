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

#include "videostreamer.h"
#include "libsoro/logger.h"
#include "libsoro/constants.h"

namespace Soro {
namespace Rover {

VideoStreamer::VideoStreamer(QGst::ElementPtr source, VideoFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent)
        : Soro::Gst::MediaStreamer("VideoStreamer", parent) {
    if (!connectToParent(ipcPort)) return;

    LOG_I(LOG_TAG, "Creating pipeline");
    _pipeline = createPipeline();

    // create gstreamer command
    QString binStr = makeEncodingBinString(format, bindAddress, address);
    QGst::BinPtr encoder = QGst::Bin::fromDescription(binStr);

    LOG_I(LOG_TAG, "Created gstreamer bin <source> ! " + binStr);

    // link elements
    _pipeline->add(source, encoder);
    source->link(encoder);

    LOG_I(LOG_TAG, "Elements linked on pipeline");

    // play<source> ! " +
    _pipeline->setState(QGst::StatePlaying);

    LOG_I(LOG_TAG, "Stream started");
}

VideoStreamer::VideoStreamer(QString sourceDevice, VideoFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent)
        : Soro::Gst::MediaStreamer("VideoStreamer", parent) {
    if (!connectToParent(ipcPort)) return;

     LOG_I(LOG_TAG, "Creating pipeline");
    _pipeline = createPipeline();

    // create gstreamer command
    QString binStr = "v4l2src device=" + sourceDevice + " ! " + makeEncodingBinString(format, bindAddress, address);

    QGst::BinPtr encoder = QGst::Bin::fromDescription(binStr);

    LOG_I(LOG_TAG, "Created gstreamer bin " + binStr);

    _pipeline->add(encoder);

    LOG_I(LOG_TAG, "Elements linked on pipeline");

    // play
    _pipeline->setState(QGst::StatePlaying);

    LOG_I(LOG_TAG, "Stream started");

}

QString VideoStreamer::makeEncodingBinString(VideoFormat format, SocketAddress bindAddress, SocketAddress address) {
    QString binStr = "videoconvert ! "
                     "videoscale method=nearest-neighbour ! "
                     "video/x-raw,height=";
    switch (format) {
    case Mpeg2_144p_300Kpbs:
        binStr += "144 ! avenc_mpeg4 bitrate=300000 bitrate-tolerance=1000000";
        break;
    case Mpeg2_360p_750Kpbs:
        binStr += "360 ! avenc_mpeg4 bitrate=750000 bitrate-tolerance=1000000";
        break;
    case Mpeg2_480p_1500Kpbs:
        binStr += "480 ! avenc_mpeg4 bitrate=1500000 bitrate-tolerance=1000000";
        break;
    case Mpeg2_720p_3000Kpbs:
        binStr += "720 ! avenc_mpeg4 bitrate=3000000 bitrate-tolerance=1000000";
        break;
    case Mpeg2_720p_5000Kpbs:
        binStr += "720 ! avenc_mpeg4 bitrate=5000000 bitrate-tolerance=2000000";
        break;
    case Mpeg2_360p_500Kbps_BW:
        binStr += "144 ! avenc_mpeg4 bitrate=100000 bitrate-tolerance=100000";
        break;
    default:
        //unknown codec
        LOG_E(LOG_TAG, "Unknown VideoFormat received");
        QCoreApplication::exit(STREAMPROCESS_ERR_UNKNOWN_CODEC);
        return "";
    }

    binStr += " max-threads=3 ! "
              "rtpmp4vpay config-interval=3 pt=96 ! "
              "udpsink bind-address=" + bindAddress.host.toString() + " bind-port=" + QString::number(bindAddress.port)
                    + " host=" + address.host.toString() + " port=" + QString::number(address.port);

    return binStr;
}

} // namespace Rover
} // namespace Soro


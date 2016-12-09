#include "videoformat.h"
#include "logger.h"

#define LOG_TAG "VideoFormat"

namespace Soro {

VideoFormat::VideoFormat() {
    // Default
    _encoding = Encoding_Null;
    _resolution = Resolution_480p;
    _stereoMode = StereoMode_None;
    _bitrate = 1000000;
    _maxThreads = 3;
    _framerate = 0;
}

VideoFormat::VideoFormat(const VideoFormat &other) {
    _encoding = other._encoding;
    _resolution = other._resolution;
    _stereoMode = other._stereoMode;
    _bitrate = other._bitrate;
    _maxThreads = other._maxThreads;
    _framerate = other._framerate;
}

VideoFormat::VideoFormat(VideoFormat::Encoding encoding, VideoFormat::Resolution resolution, quint32 bitrate,
                         quint32 framerate, StereoMode stereo, quint32 maxThreads) {
    _encoding = encoding;
    _resolution = resolution;
    _bitrate = bitrate;
    _framerate = framerate;
    _stereoMode = stereo;
    _maxThreads = maxThreads;
}

VideoFormat::~VideoFormat() { }

bool VideoFormat::isUseable() const {
    return (_encoding != Encoding_Null) &&
            (_bitrate > 0) &&
            (_framerate > 0);
}

VideoFormat::Encoding VideoFormat::getEncoding() const {
    return _encoding;
}

VideoFormat::Resolution VideoFormat::getResolution() const {
    return _resolution;
}

quint32 VideoFormat::getBitrate() const {
    return _bitrate;
}

quint32 VideoFormat::getFramerate() const {
    return _framerate;
}

VideoFormat::StereoMode VideoFormat::getStereoMode() const {
    return _stereoMode;
}

quint32 VideoFormat::getMaxThreads() const {
    return _maxThreads;
}

void VideoFormat::setEncoding(VideoFormat::Encoding encoding) {
    _encoding = encoding;
}

void VideoFormat::setResolution(VideoFormat::Resolution resolution) {
    _resolution = resolution;
}

void VideoFormat::setBitrate(quint32 bitrate) {
    _bitrate = bitrate;
}

void VideoFormat::setFramerate(quint32 framerate) {
    _framerate = framerate;
}

void VideoFormat::setStereoMode(VideoFormat::StereoMode stereo) {
    _stereoMode = stereo;
}

void VideoFormat::setMaxThreads(quint32 maxThreads) {
    _maxThreads = maxThreads;
}

quint32 VideoFormat::getResolutionWidth() const {
    return reinterpret_cast<const quint32&>(_resolution);
}

quint32 VideoFormat::getResolutionHeight() const {
    return getResolutionWidth() * 16 / 9;
}

QString VideoFormat::toHumanReadableString() const {
    QString str = "%1 %2p@%3fps (%4Kb)";
    QString encoding;
    switch (_encoding) {
    case Encoding_Null:
        return "No Video";
    case Encoding_MPEG2:
        encoding = "MPEG2";
        break;
    default:
        return "Unknown Encoding";
    }

    return str.arg(encoding,
                   QString::number(getResolutionHeight()),
                   QString::number(_framerate),
                   QString::number(_bitrate / 1000));
}

QString VideoFormat::createGstEncodingArgs() const {
    QString encString = "";
    QString stereoEncString = "";
    QString framerateEncString = "";
    quint32 height = reinterpret_cast<const quint32&>(_resolution);

    switch (_encoding) {
    case Encoding_MPEG2:
        encString = "videoscale method=0 ! "
                    "video/x-raw,height=%1 ! "
                    "%2" // For stereo
                    "%3" // For framerate
                    "videoconvert ! "
                    "avenc_mpeg4 bitrate=%4 bitrate-tolerance=%5 max-threads=%6 ! "
                    "rtpmp4vpay config-interval=3 pt=96";
        break;
    default:
        //unknown codec
        LOG_E(LOG_TAG, "Unknown video encoding");
        return "";
    }

    switch (_stereoMode) {
    case StereoMode_SideBySide:
        stereoEncString = "videoscale method=0 add-borders=false ! "
                          "video/x-raw,height=%1,width=%2 ! ";
        stereoEncString = stereoEncString.arg(QString::number(height),
                                              QString::number(height * 8 / 9));
        break;
    default:
        break;
    }

    if (_framerate > 0) {
        framerateEncString = "videorate ! "
                             "video/x-raw,framerate=%1/1 ! ";
        framerateEncString = framerateEncString.arg(QString::number(_framerate));
    }

    encString = encString.arg(QString::number(height),
                              stereoEncString,
                              framerateEncString,
                              QString::number(_bitrate),
                              QString::number(_bitrate / 4),
                              QString::number(_maxThreads));
    return encString;
}

QString VideoFormat::createGstDecodingArgs() const {
    switch (_encoding) {
    case Encoding_MPEG2:
        // Same decoding args for all MPEG2 formats
        return "application/x-rtp,media=video,clock-rate=90000,encoding-name=MP4V-ES,profile-level-id=1,payload=96,ssrc=2873740600,timestamp-offset=391825150,seqnum-offset=2980 ! "
               "rtpmp4vdepay ! "
               "avdec_mpeg4";
    default:
        // unknown codec
        LOG_E(LOG_TAG, "Unknown video encoding");
        return "";
    }
}

QString VideoFormat::serialize() const {
    QString serial;
    serial += QString::number(reinterpret_cast<const quint32&>(_encoding)) + "_";
    serial += QString::number(reinterpret_cast<const quint32&>(_resolution)) + "_";
    serial += QString::number(reinterpret_cast<const quint32&>(_stereoMode)) + "_";
    serial += QString::number(_framerate) + "_";
    serial += QString::number(_bitrate) + "_";
    serial += QString::number(_maxThreads);

    return serial;
}

void VideoFormat::deserialize(QString serial) {
    QStringList items = serial.split('_');
    if (items.size() < 6) {
        LOG_E(LOG_TAG, "deserialize(): Invalid string");
        return;
    }

    bool ok;
    reinterpret_cast<quint32&>(_encoding) = items[0].toUInt(&ok);
    if (!ok) {
        LOG_E(LOG_TAG, "deserialize(): Invalid option for encoding");
    }
    reinterpret_cast<quint32&>(_resolution) = items[1].toUInt(&ok);
    if (!ok) {
        LOG_E(LOG_TAG, "deserialize(): Invalid option for resolution");
    }
    reinterpret_cast<quint32&>(_stereoMode) = items[2].toUInt(&ok);
    if (!ok) {
        LOG_E(LOG_TAG, "deserialize(): Invalid option for stereo mode");
    }
    _framerate = items[3].toUInt(&ok);
    if (!ok) {
        LOG_E(LOG_TAG, "deserialize(): Invalid option for framerate");
    }
    _bitrate = items[4].toUInt(&ok);
    if (!ok) {
        LOG_E(LOG_TAG, "deserialize(): Invalid option for bitrate");
    }
    _maxThreads = items[5].toUInt(&ok);
    if (!ok) {
        LOG_E(LOG_TAG, "deserialize(): Invalid option for max threads");
    }
}

} // namespace Soro

#include "videoformat.h"
#include "logger.h"

#define LOG_TAG "VideoFormat"

namespace Soro {

VideoFormat::VideoFormat() {
    // Default
    _encoding = Encoding_Null;
    _resolution = Resolution_640x360;
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
    return (_encoding != Encoding_Null) && (_bitrate > 0);
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
    switch (_resolution) {
    case Resolution_640x360:    return 640;
    case Resolution_1024x576:   return 1024;
    case Resolution_1152x648:   return 1152;
    case Resolution_1280x720:   return 1280;
    case Resolution_1600x900:   return 1600;
    case Resolution_1920x1080:  return 1920;
    case Resolution_2560x1440:  return 2560;
    case Resolution_3840x2160:  return 3840;
    }
}

quint32 VideoFormat::getResolutionHeight() const {
    switch (_resolution) {
    case Resolution_640x360:    return 360;
    case Resolution_1024x576:   return 576;
    case Resolution_1152x648:   return 648;
    case Resolution_1280x720:   return 720;
    case Resolution_1600x900:   return 900;
    case Resolution_1920x1080:  return 1080;
    case Resolution_2560x1440:  return 1440;
    case Resolution_3840x2160:  return 2160;
    }
}

quint32 VideoFormat::getWidth() const {
    return _stereoMode == StereoMode_SideBySide ?
                getResolutionWidth() / 2 :
                getResolutionWidth();
}

quint32 VideoFormat::getHeight() const {
    return getResolutionHeight();
}

QString VideoFormat::toHumanReadableString() const {
    QString str = "%1 %2x%3@%4 (%5Kb)";
    QString encoding;
    QString framerate;
    switch (_encoding) {
    case Encoding_Null:
        return "No Video";
    case Encoding_MPEG2:
        encoding = "MPEG2";
        break;
    case Encoding_MJPEG:
        encoding = "MJPEG";
        break;
    case Encoding_X264:
        encoding = "x264";
        break;
    default:
        return "Unknown Encoding";
    }

    switch (_framerate) {
    case 0:
        framerate = "Auto";
        break;
    default:
        framerate = QString::number(_framerate) + "/1";
        break;
    }

    return str.arg(encoding,
                   QString::number(getResolutionWidth()),
                   QString::number(getResolutionHeight()),
                   framerate,
                   QString::number(_bitrate / 1000));
}

QString VideoFormat::createGstEncodingArgs() const {
    QString encString = "";
    QString stereoEncString = "";
    QString framerateEncString = "";
    int bitrate = _bitrate;

    switch (_stereoMode) {
    case StereoMode_SideBySide:
        stereoEncString = "videoscale method=0 add-borders=false ! "
                          "video/x-raw,width=%1,height=%2 ! ";
        stereoEncString = stereoEncString.arg(QString::number(getWidth()),
                                              QString::number(getHeight()));
        bitrate /= 2;
        break;
    default:
        break;
    }

    if (_framerate > 0) {
        framerateEncString = QString(
                        "videorate ! "
                        "video/x-raw,framerate=%1/1 ! "
                    ).arg(
                        QString::number(_framerate)
                    );
    }

    // Comming encoding params for all codecs
    encString = QString(
                    "videoscale method=0 ! "
                    "video/x-raw,width=%1,height=%2 ! "
                    "%3" // For stereo
                    "%4" // For framerate
                    "videoconvert ! "
                ).arg(
                    QString::number(getResolutionWidth()),
                    QString::number(getResolutionHeight()),
                    stereoEncString,
                    framerateEncString
                );

    switch (_encoding) {
    case Encoding_MPEG2:
        encString += QString(
                        "avenc_mpeg4 bitrate=%1 bitrate-tolerance=%2 max-threads=%3 ! "
                        "rtpmp4vpay config-interval=3 pt=96"
                    ).arg(
                        QString::number(bitrate),
                        QString::number(bitrate / 4),
                        QString::number(_maxThreads)
                    );
        break;
    case Encoding_MJPEG:
        // TODO adjustable mjpeg quality
        encString += "jpegenc quality=50 ! rtpjpegpay";
        break;
    case Encoding_X264:
        encString += QString(
                        "x264enc tune=zerolatency bitrate=%1 threads=%2 ! "
                        "rtph264pay config-interval=3 pt=96"
                    ).arg(
                        QString::number(bitrate / 1000),
                        QString::number(_maxThreads)
                    );
        break;
    default:
        //unknown codec
        LOG_E(LOG_TAG, "Unknown video encoding");
        return "";
    }

    return encString;
}

QString VideoFormat::createGstDecodingArgs() const {
    switch (_encoding) {
    case Encoding_MPEG2:
        // Same decoding args for all MPEG2 formats
        return "application/x-rtp,media=video,clock-rate=90000,encoding-name=MP4V-ES,profile-level-id=1,payload=96 ! "
               "rtpmp4vdepay ! "
               "avdec_mpeg4";
    case Encoding_X264:
        return "application/x-rtp,media=video,clock-rate=90000,encoding-name=H264 ! "
               "rtph264depay ! "
               "avdec_h264";
    case Encoding_MJPEG:
        return "application/x-rtp,media=video,encoding=JPEG,payload=26 ! "
               "rtpjpegdepay ! "
               "jpegdec";
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

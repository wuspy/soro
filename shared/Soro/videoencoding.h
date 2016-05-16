#ifndef VIDEOENCODING_H
#define VIDEOENCODING_H

#include <QDataStream>

namespace Soro {

enum VideoEncoding {
    UnknownOrNoEncoding = 0,
    MjpegEncoding,
    Mpeg2Encoding,
    x264Encoding
};

struct StreamFormat {
    VideoEncoding Encoding = UnknownOrNoEncoding;
    qint32 Height = 0;
    qint32 Mjpeg_Quality = 0;
    qint32 Bitrate = 0;

    StreamFormat() { }

    StreamFormat(const StreamFormat& other) {
        Encoding = other.Encoding;
        Height = other.Height;
        Mjpeg_Quality = other.Mjpeg_Quality;
        Bitrate = other.Bitrate;
    }

    inline bool isNull() const {
        return Encoding == UnknownOrNoEncoding;
    }

    inline bool operator==(const StreamFormat& other) const {
        return (Height == other.Height) && (Encoding == other.Encoding)
                && (Encoding == MjpegEncoding ? (Mjpeg_Quality == other.Mjpeg_Quality) : (Bitrate == other.Bitrate));
    }

    inline friend QDataStream& operator<<(QDataStream& stream, const StreamFormat& format) {
        VideoEncoding encoding = format.Encoding;
        stream << reinterpret_cast<qint32&>(encoding);
        stream << format.Height;
        stream << format.Mjpeg_Quality;
        stream << format.Bitrate;
        return stream;
    }

    inline friend QDataStream& operator>>(QDataStream& stream, StreamFormat& format) {
        stream >> reinterpret_cast<qint32&>(format.Encoding);
        stream >> format.Height;
        stream >> format.Mjpeg_Quality;
        stream >> format.Bitrate;
        return stream;
    }
};

inline StreamFormat streamFormatPreview() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 144;
    format.Bitrate = 300000;
    return format;
}

inline StreamFormat streamFormatLow() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 360;
    format.Bitrate = 750000;
    return format;
}

inline StreamFormat streamFormatMedium() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 480;
    format.Bitrate = 1500000;
    return format;
}

inline StreamFormat streamFormatHigh() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 720;
    format.Bitrate = 3000000;
    return format;
}

inline StreamFormat streamFormatUltra() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 720;
    format.Bitrate = 5000000;
    return format;
}

}

#endif // VIDEOENCODING_H

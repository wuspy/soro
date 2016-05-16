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
<<<<<<< HEAD
    qint32 Height = 0;
=======
    qint32 Height = 0, Framerate = 0;
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
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
<<<<<<< HEAD
        return (Height == other.Height) && (Encoding == other.Encoding)
=======
        return (Height == other.Height) && (Framerate == other.Framerate) && (Encoding == other.Encoding)
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
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
<<<<<<< HEAD
=======
    format.Framerate = 5;
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
    format.Bitrate = 300000;
    return format;
}

inline StreamFormat streamFormatLow() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 360;
<<<<<<< HEAD
=======
    format.Framerate = 15;
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
    format.Bitrate = 750000;
    return format;
}

inline StreamFormat streamFormatMedium() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 480;
<<<<<<< HEAD
=======
    format.Framerate = 15;
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
    format.Bitrate = 1500000;
    return format;
}

inline StreamFormat streamFormatHigh() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 720;
<<<<<<< HEAD
=======
    format.Framerate = 15;
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
    format.Bitrate = 3000000;
    return format;
}

inline StreamFormat streamFormatUltra() {
    StreamFormat format;
    format.Encoding = Mpeg2Encoding;
    format.Height = 720;
<<<<<<< HEAD
=======
    format.Framerate = 20;
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
    format.Bitrate = 5000000;
    return format;
}

}

#endif // VIDEOENCODING_H

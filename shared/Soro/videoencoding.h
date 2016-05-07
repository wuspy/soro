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
    qint32 Width = 0, Height = 0, Framerate = 0;
    qint32 Mjpeg_Quality = 0;
    qint32 Bitrate = 0;

    StreamFormat() { }

    StreamFormat(const StreamFormat& other) {
        Encoding = other.Encoding;
        Width = other.Width;
        Height = other.Height;
        Mjpeg_Quality = other.Mjpeg_Quality;
        Bitrate = other.Bitrate;
        Framerate = other.Framerate;
    }

    VideoEncoding Encoding = UnknownOrNoEncoding;

    float aspectRatio() {
        if (Height == 0) return 0;
        return (float)Width / (float)Height;
    }

    inline friend QDataStream& operator<<(QDataStream& stream, const StreamFormat& format) {
        VideoEncoding encoding = format.Encoding;
        stream << reinterpret_cast<qint32&>(encoding);
        stream << format.Width;
        stream << format.Height;
        stream << format.Framerate;
        stream << format.Mjpeg_Quality;
        stream << format.Bitrate;
        return stream;
    }

    inline friend QDataStream& operator>>(QDataStream& stream, StreamFormat& format) {
        stream >> reinterpret_cast<qint32&>(format.Encoding);
        stream >> format.Width;
        stream >> format.Height;
        stream >> format.Framerate;
        stream >> format.Mjpeg_Quality;
        stream >> format.Bitrate;
        return stream;
    }
};

inline StreamFormat mjpeg_streamFormatLowQuality() {
    StreamFormat format;
    format.Encoding = MjpegEncoding;
    format.Width = 480;
    format.Height = 360;
    format.Framerate = 15;
    format.Mjpeg_Quality = 30;
    return format;
}

inline StreamFormat mjpeg_streamFormatNormalQuality() {
    StreamFormat format;
    format.Encoding = MjpegEncoding;
    format.Width = 640;
    format.Height = 480;
    format.Framerate = 15;
    format.Mjpeg_Quality = 40;
    return format;
}

inline StreamFormat mjpeg_streamFormatHighQuality() {
    StreamFormat format;
    format.Encoding = MjpegEncoding;
    format.Width = 960;
    format.Height = 720;
    format.Framerate = 15;
    format.Mjpeg_Quality = 30;
    return format;
}

inline StreamFormat mjpeg_streamFormatUltraQuality() {
    StreamFormat format;
    format.Encoding = MjpegEncoding;
    format.Width = 960;
    format.Height = 720;
    format.Framerate = 20;
    format.Mjpeg_Quality = 50;
    return format;
}

inline StreamFormat x264() {
    StreamFormat format;
    format.Encoding = x264Encoding;
    format.Width = 960;
    format.Height = 720;
    format.Framerate = 30;
    format.Bitrate = 1000;
    return format;
}

}

#endif // VIDEOENCODING_H

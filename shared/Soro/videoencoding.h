#ifndef VIDEOENCODING_H
#define VIDEOENCODING_H

#include <QDataStream>

namespace Soro {

enum VideoEncoding {
    UnknownOrNoEncoding = 0,
    MjpegEncoding,
    Mpeg2Encoding
};

struct StreamFormat {
    qint32 Width = 0, Height = 0, Framerate = 0;
    qint32 Mjpeg_Quality = 0;
    qint32 Mpeg2_Bitrate = 0;

    StreamFormat() { }

    StreamFormat(const StreamFormat& other) {
        Encoding = other.Encoding;
        Width = other.Width;
        Height = other.Height;
        Mjpeg_Quality = other.Mjpeg_Quality;
        Mpeg2_Bitrate = other.Mpeg2_Bitrate;
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
        stream << format.Mpeg2_Bitrate;
        return stream;
    }

    inline friend QDataStream& operator>>(QDataStream& stream, StreamFormat& format) {
        stream >> reinterpret_cast<qint32&>(format.Encoding);
        stream >> format.Width;
        stream >> format.Height;
        stream >> format.Framerate;
        stream >> format.Mjpeg_Quality;
        stream >> format.Mpeg2_Bitrate;
        return stream;
    }
};

}

#endif // VIDEOENCODING_H

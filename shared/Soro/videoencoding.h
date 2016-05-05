#ifndef VIDEOENCODING_H
#define VIDEOENCODING_H

namespace Soro {

enum VideoEncoding {
    UnknownEncoding = 0,
    MjpegEncoding,
    Mpeg2Encoding
};

struct StreamFormat {
    int Width = 0, Height = 0, Framerate = 0;
    int Mjpeg_Quality = 0;
    int Mpeg2_Bitrate = 0;

    StreamFormat() { }

    StreamFormat(const StreamFormat& other) {
        Encoding = other.Encoding;
        Width = other.Width;
        Height = other.Height;
        Mjpeg_Quality = other.Mjpeg_Quality;
        Mpeg2_Bitrate = other.Mpeg2_Bitrate;
        Framerate = other.Framerate;
    }

    VideoEncoding Encoding = UnknownEncoding;

    float aspectRatio() {
        if (Height == 0) return 0;
        return (float)Width / (float)Height;
    }
};

}

#endif // VIDEOENCODING_H

#ifndef VIDEOENCODING_H
#define VIDEOENCODING_H

#include <QtCore>

namespace Soro {

enum VideoEncoding {
    UNKNOWN,
    MJPEG,
    MPEG2
};

struct StreamFormat {
    int Width, Height, Framerate;

    virtual VideoEncoding encoding() const = 0;

    StreamFormat(int width, int height, int framerate) {
        Width = width;
        Height = height;
        Framerate = framerate;
    }

    float aspectRatio() {
        return (float)Width / (float)Height;
    }
};

struct MjpegStreamFormat: public StreamFormat {
    int Quality;

    MjpegStreamFormat(int width, int height, int framerate, int quality)
        : StreamFormat(width, height, framerate) {
        Quality = quality;
    }

    VideoEncoding encoding() const Q_DECL_OVERRIDE {
        return MJPEG;
    }
};

struct Mpeg2StreamFormat: public StreamFormat {
    int Bitrate;

    Mpeg2StreamFormat(int width, int height, int framerate, int bitrate)
        : StreamFormat(width, height, framerate) {
        Bitrate = bitrate;
    }

    VideoEncoding encoding() const Q_DECL_OVERRIDE {
        return MPEG2;
    }
};

}

#endif // VIDEOENCODING_H

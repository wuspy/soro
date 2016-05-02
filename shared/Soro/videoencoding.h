#ifndef VIDEOENCODING_H
#define VIDEOENCODING_H

#include <QtCore>

namespace Soro {

enum VideoEncoding {
    MJPEG,
    MPEG2
};

struct StreamFormat {
    int Width, Height, Framerate;

    virtual VideoEncoding encoding() const = 0;

    float aspectRatio() {
        return (float)Width / (float)Height;
    }
};

struct MjpegStreamFormat: public StreamFormat {
    int Quality;

    VideoEncoding encoding() const Q_DECL_OVERRIDE {
        return MJPEG;
    }
};

struct Mpeg2StreamFormat: public StreamFormat {
    int Bitrate;

    VideoEncoding encoding() const Q_DECL_OVERRIDE {
        return MPEG2;
    }
};

}

#endif // VIDEOENCODING_H

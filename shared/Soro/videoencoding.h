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
    int Mjpeg_Quality;
    int Mpeg2_Bitrate;

    VideoEncoding Encoding = UnknownEncoding;

    float aspectRatio() {
        if (Height == 0) return 0;
        return (float)Width / (float)Height;
    }
};

}

#endif // VIDEOENCODING_H

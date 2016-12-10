#ifndef VIDEOFORMAT_H
#define VIDEOFORMAT_H

#include <QtCore>
#include "mediaformat.h"

namespace Soro {

class VideoFormat: public MediaFormat {
    Q_GADGET
public:

    enum Encoding {
        Encoding_Null = 0,
        Encoding_MPEG2
    };
    Q_ENUM(Encoding)

    enum Resolution {
        Resolution_144p = 144,
        Resolution_240p = 240,
        Resolution_360p = 360,
        Resolution_480p = 480,
        Resolution_720p = 720,
        Resolution_1080p = 1080
    };
    Q_ENUM(Resolution)

    enum StereoMode {
        StereoMode_None = 0,
        StereoMode_SideBySide
    };
    Q_ENUM(StereoMode)

    VideoFormat();
    VideoFormat(const VideoFormat& other);
    VideoFormat(VideoFormat::Encoding encoding, VideoFormat::Resolution resolution, quint32 bitrate,
                quint32 framerate=0, StereoMode stereo=StereoMode_None, quint32 maxThreads=0);

    ~VideoFormat();

    QString toHumanReadableString() const Q_DECL_OVERRIDE;
    QString createGstEncodingArgs() const Q_DECL_OVERRIDE;
    QString createGstDecodingArgs() const Q_DECL_OVERRIDE;

    VideoFormat::Encoding getEncoding() const;
    VideoFormat::Resolution getResolution() const;
    quint32 getBitrate() const;
    quint32 getFramerate() const;
    VideoFormat::StereoMode getStereoMode() const;
    quint32 getMaxThreads() const;

    void setEncoding(VideoFormat::Encoding encoding);
    void setResolution(VideoFormat::Resolution resolution);
    void setBitrate(quint32 bitrate);
    void setFramerate(quint32 framerate);
    void setStereoMode(VideoFormat::StereoMode stereo);
    void setMaxThreads(quint32 maxThreads);

    quint32 getResolutionHeight() const;
    quint32 getResolutionWidth() const;

    bool isUseable() const Q_DECL_OVERRIDE;

    QString serialize() const Q_DECL_OVERRIDE;
    void deserialize(QString serial) Q_DECL_OVERRIDE;

    inline bool operator==(const VideoFormat& other) {
        return (_encoding == other._encoding) &&
                (_resolution == other._resolution) &&
                (_stereoMode == other._stereoMode) &&
                (_bitrate == other._bitrate) &&
                (_framerate == other._framerate) &&
                (_maxThreads == other._maxThreads);
    }

    inline bool operator!=(const VideoFormat& other) {
        return !(*this==other);
    }

private:
    VideoFormat::Encoding _encoding;
    VideoFormat::Resolution _resolution;
    VideoFormat::StereoMode _stereoMode;
    quint32 _bitrate;
    quint32 _maxThreads;
    quint32 _framerate;


};

} // namespace Soro

Q_DECLARE_METATYPE(Soro::VideoFormat)

#endif // VIDEOFORMAT_H

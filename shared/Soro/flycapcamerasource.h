#ifndef FLYCAPCAMERASOURCE_H
#define FLYCAPCAMERASOURCE_H

#include <Qt5GStreamer/QGst/Utils/ApplicationSource>

class FlycapCameraSource : QObject, QGst::Utils::ApplicationSource {
public:
    FlycapCameraSource();
    ~FlycapCameraSource();

    QGst::AppStreamType streamType() const Q_DECL_OVERRIDE;
};

#endif // FLYCAPCAMERASOURCE_H

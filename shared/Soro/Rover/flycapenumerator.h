#ifndef SORO_ROVER_FLYCAPENUMERATOR_H
#define SORO_ROVER_FLYCAPENUMERATOR_H

#include <flycapture/FlyCapture2.h>
#include <QMap>

#include "logger.h"

namespace Soro {
namespace Rover {

class FlycapEnumerator {

public:
    /* Enumerates all flycapture cameras connected. If successful, this will
     * return the number of cameras detected, otherwise -1 will be returned.
     */
    int loadCameras();

    QList<FlyCapture2::PGRGuid> listByGuid();
    bool cameraExists(unsigned int serial);
    FlyCapture2::PGRGuid getGUIDForSerial(unsigned int serial);

private:
    QMap<unsigned int, FlyCapture2::PGRGuid> _cameras;
};

} // namespace Rover
} // namespace Soro

#endif // SORO_ROVER_FLYCAPENUMERATOR_H

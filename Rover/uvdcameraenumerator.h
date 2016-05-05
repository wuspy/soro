#ifndef UVDCAMERAENUMERATOR_H
#define UVDCAMERAENUMERATOR_H

#include <QMap>

#include "logger.h"

namespace Soro {
namespace Rover {

class UvdCameraEnumerator {

public:
    /* Enumerates all USB/UVD cameras connected. If successful, this will
     * return the number of cameras detected, otherwise -1 will be returned.
     */
    int loadCameras();

    const QList<QString>& listByDeviceName();

private:
    QList<QString> _cameras;
};

} // namespace Rover
} // namespace Soro

#endif // UVDCAMERAENUMERATOR_H

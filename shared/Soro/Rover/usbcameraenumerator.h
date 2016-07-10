#ifndef SORO_ROVER_USBCAMERAENUMERATOR_H
#define SORO_ROVER_USBCAMERAENUMERATOR_H

#include <QMap>

#include "logger.h"

namespace Soro {
namespace Rover {

class UsbCameraEnumerator {

public:
    /* Enumerates all USB cameras connected. If successful, this will
     * return the number of cameras detected, otherwise -1 will be returned.
     */
    int loadCameras();

    const QList<QString>& listByDeviceName();

private:
    QList<QString> _cameras;
};

} // namespace Rover
} // namespace Soro

#endif // SORO_ROVER_USBCAMERAENUMERATOR_H

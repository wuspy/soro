#ifndef CAMERAENUMERATOR_H
#define CAMERAENUMERATOR_H

#include <QMap>

#ifdef __linux__

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#endif

#include "logger.h"

namespace Soro {
namespace Rover {

class CameraEnumerator {

public:

};

} // namespace Rover
} // namespace Soro

#endif // CAMERAENUMERATOR_H

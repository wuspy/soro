#ifndef ROVERCONFIGLOADER_H
#define ROVERCONFIGLOADER_H

#include <QList>

#include "soro_global.h"

namespace Soro {

class LIBSORO_EXPORT RoverConfigLoader
{
private:
    QList<QString> _blacklistCameras;
    int _computer1CameraCount;
    int _computer2CameraCount;

public:
    bool load(QString *error);
    int getComputer1CameraCount();
    int getComputer2CameraCount();
    QList<QString> getBlacklistedCameras();
};

} // namespace Soro



#endif // ROVERCONFIGLOADER_H

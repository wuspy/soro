#ifndef ROVERCONFIGLOADER_H
#define ROVERCONFIGLOADER_H

#include <QList>
#include "iniparser.h"

namespace Soro {
namespace Rover {

class RoverConfigLoader
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

} // namespace Rover
} // namespace Soro



#endif // ROVERCONFIGLOADER_H

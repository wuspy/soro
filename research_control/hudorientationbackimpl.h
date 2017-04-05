#ifndef HUDORIENTATIONBACKIMPL_H
#define HUDORIENTATIONBACKIMPL_H

#include <QQuickItem>
#include <QPainter>

#include "libsoro/constants.h"
#include "abstracthudorientationimpl.h"

namespace Soro {
namespace MissionControl {

class HudOrientationBackImpl: public AbstractHudOrientationImpl
{
    Q_OBJECT
    Q_PROPERTY(float frontRoll READ getFrontRoll WRITE setFrontRoll)
    Q_PROPERTY(float rearRoll READ getRearRoll WRITE setRearRoll)
public:
    HudOrientationBackImpl(QQuickItem *parent=0);

    void paint(QPainter *painter);

    float getRearRoll() const;
    void setRearRoll(float rearRoll);

    float getFrontRoll() const;
    void setFrontRoll(float frontRoll);

private:
    float _frontRoll = 500;
    float _rearRoll = 500;
};

} // namespace MissionControl
} // namespace Soro

#endif // HUDORIENTATIONBACKIMPL_H

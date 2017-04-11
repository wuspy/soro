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
    Q_PROPERTY(float frontRollZero READ getFrontRollZero WRITE setFrontRollZero)
    Q_PROPERTY(float rearRollZero READ getRearRollZero WRITE setRearRollZero)

public:
    HudOrientationBackImpl(QQuickItem *parent=0);

    void paint(QPainter *painter);

    float getRearRoll() const;
    void setRearRoll(float rearRoll);

    float getFrontRoll() const;
    void setFrontRoll(float frontRoll);

    float getRearRollZero() const;
    void setRearRollZero(float rearRollZero);

    float getFrontRollZero() const;
    void setFrontRollZero(float frontRollZero);

private:
    float rollToDegrees(float roll, float rollZero);

    float _frontRollZero;
    float _rearRollZero;

    float _frontRoll;
    float _rearRoll;
};

} // namespace MissionControl
} // namespace Soro

#endif // HUDORIENTATIONBACKIMPL_H

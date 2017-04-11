#ifndef HUDORIENTATIONSIDEIMPL_H
#define HUDORIENTATIONSIDEIMPL_H

#include <QQuickItem>
#include <QPainter>

#include "libsoro/constants.h"
#include "abstracthudorientationimpl.h"

namespace Soro {
namespace MissionControl {

class HudOrientationSideImpl: public AbstractHudOrientationImpl
{
    Q_OBJECT
    Q_PROPERTY(float frontPitch READ getFrontPitch WRITE setFrontPitch)
    Q_PROPERTY(float rearPitch READ getRearPitch WRITE setRearPitch)
    Q_PROPERTY(float frontPitchZero READ getFrontPitchZero WRITE setFrontPitchZero)
    Q_PROPERTY(float rearPitchZero READ getRearPitchZero WRITE setRearPitchZero)

public:
    HudOrientationSideImpl(QQuickItem *parent=0);

    void paint(QPainter *painter);

    void setFrontPitch(float frontPitch);
    float getFrontPitch() const;

    void setRearPitch(float rearPitch);
    float getRearPitch() const;

    void setFrontPitchZero(float frontPitchZero);
    float getFrontPitchZero() const;

    void setRearPitchZero(float rearPitchZero);
    float getRearPitchZero() const;

private:
    float pitchToDegrees(float pitch, float pitchZero);

    float _rearPitch;
    float _frontPitch;

    float _rearPitchZero;
    float _frontPitchZero;
};

} // namespace MissionControl
} // namespace Soro

#endif // HUDORIENTATIONSIDEIMPL_H

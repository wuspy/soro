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
public:
    HudOrientationSideImpl(QQuickItem *parent=0);

    void paint(QPainter *painter);

    void setFrontPitch(float frontPitch);
    float getFrontPitch() const;

    void setRearPitch(float rearPitch);
    float getRearPitch() const;

private:
    float _rearPitch = 0;
    float _frontPitch = 0;
};

} // namespace MissionControl
} // namespace Soro

#endif // HUDORIENTATIONSIDEIMPL_H

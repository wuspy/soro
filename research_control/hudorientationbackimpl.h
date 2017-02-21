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
public:
    HudOrientationBackImpl(QQuickItem *parent=0);

    void paint(QPainter *painter);
};

} // namespace MissionControl
} // namespace Soro

#endif // HUDORIENTATIONBACKIMPL_H

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
public:
    HudOrientationSideImpl(QQuickItem *parent=0);

    void paint(QPainter *painter);
};

} // namespace MissionControl
} // namespace Soro

#endif // HUDORIENTATIONSIDEIMPL_H

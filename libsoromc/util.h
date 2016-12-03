#ifndef SORO_MISSIONCONTROL_UTIL_H
#define SORO_MISSIONCONTROL_UTIL_H

#include <QWidget>
#include <QGraphicsDropShadowEffect>

#include "soro_missioncontrol_global.h"

namespace Soro {
namespace MissionControl {

inline void addWidgetShadow(QWidget *target, int radius, int offset) {
    QGraphicsDropShadowEffect* ef = new QGraphicsDropShadowEffect;
    ef->setBlurRadius(radius);
    ef->setColor(QColor::fromHsv(0, 0, 0, 255));
    ef->setOffset(offset);
    target->setGraphicsEffect(ef);
}

} // namespace MissionControl
} // namespace Soro

#endif // SORO_MISSIONCONTROL_UTIL_H

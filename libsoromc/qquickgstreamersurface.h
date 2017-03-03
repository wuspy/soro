#ifndef QQUICKGSTREAMERSURFACE_H
#define QQUICKGSTREAMERSURFACE_H

#include "soro_missioncontrol_global.h"

#include <QQuickPaintedItem>
#include <QColor>

#include <Qt5GStreamer/QGst/Element>

namespace Soro {
namespace MissionControl {

class LIBSOROMC_EXPORT QQuickGStreamerSurface : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor)

public:
    QQuickGStreamerSurface();

    void setSink(const QGst::ElementPtr & sink);
    void setBackgroundColor(QColor color);
    QColor getBackgroundColor() const;
    QGst::ElementPtr createRecommendedSink() const;

    void paint(QPainter *painter) override;

private:
    QGst::ElementPtr _sink;
    QColor _backgroundColor;

private:
    void onUpdate();
};

} // namespace MissionControl
} // namespace Soro

#endif // QQUICKGSTREAMERSURFACE_H

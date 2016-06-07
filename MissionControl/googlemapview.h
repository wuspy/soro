#ifndef SORO_MISSIONCONTROL_GOOGLEMAPVIEW_H
#define SORO_MISSIONCONTROL_GOOGLEMAPVIEW_H

#include <QWidget>
#include <QWebEnginePage>
#include <QWebEngineView>

#include "nmeamessage.h"

namespace Soro {
namespace MissionControl {

/* UI control for displaying a location track using the
 * Google Maps API.
 */
class GoogleMapView: public QWebEngineView {
    Q_OBJECT

public:
    explicit GoogleMapView(QWidget *parent = 0);

public slots:
    void addMarker(QString type);
    void updateLocation(const NmeaMessage& location);
};

}
}

#endif // SORO_MISSIONCONTROL_GOOGLEMAPVIEW_H

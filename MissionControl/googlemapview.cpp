#include "googlemapview.h"

#define MIN_UPDATE_DISTANCE 5

namespace Soro {
namespace MissionControl {

GoogleMapView::GoogleMapView(QWidget *parent) : QWebEngineView(parent) {
    setUrl(QUrl("qrc:/map.html")); //load from resources
}

void GoogleMapView::addMarker(QString type) {
    page()->runJavaScript("addMarker(\"" + type + "\");");
}

void GoogleMapView::updateLocation(const NmeaMessage& location) {
    page()->runJavaScript("updateLocation("
                          + QString::number(location.Latitude) + ", "
                          + QString::number(location.Longitude) + ", "
                          + QString::number(location.Heading) + ");");
}

}
}

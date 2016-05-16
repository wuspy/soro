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

void GoogleMapView::updateLocation(const LatLng& location) {
    if (_lastSetLocation.isEmpty() || (_lastSetLocation.metersTo(location) >= MIN_UPDATE_DISTANCE)) {
        page()->runJavaScript("updateLocation(" + location.toString() + ");");
        _lastSetLocation = location;
    }
    _lastLocation = location;
}

void GoogleMapView::updateHeading(int degrees) {
     page()->runJavaScript("updateHeading(" + QString::number(degrees) + ");");
}

const Soro::LatLng& GoogleMapView::getLastLocation() const {
    return _lastLocation;
}

}
}

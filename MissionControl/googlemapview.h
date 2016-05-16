#ifndef GOOGLEMAPVIEW_H
#define GOOGLEMAPVIEW_H

#include <QWidget>
#include <QWebEnginePage>
#include <QWebEngineView>

#include "latlng.h"

namespace Soro {
namespace MissionControl {

    class GoogleMapView: public QWebEngineView {
        Q_OBJECT
    public:
        explicit GoogleMapView(QWidget *parent = 0);
        const LatLng& getLastLocation() const;
    public slots:
        void addMarker(QString type);
        void updateLocation(const LatLng& location);
        void updateHeading(int degrees);
    private:
        LatLng _lastSetLocation;
        LatLng _lastLocation;
    };

}
}

#endif // GOOGLEMAPVIEW_H

#ifndef GOOGLEMAPVIEW_H
#define GOOGLEMAPVIEW_H

#include <QWidget>
#include <QWebEnginePage>
#include <QWebEngineView>

#include "nmeamessage.h"

namespace Soro {
namespace MissionControl {

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

#endif // GOOGLEMAPVIEW_H

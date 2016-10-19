/*
 * Copyright 2016 The University of Oklahoma.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
                          + QString::number(location.Latitude, 'f', 10) + ", "
                          + QString::number(location.Longitude, 'f', 10) + ", "
                          + QString::number(location.Heading, 'f', 10) + ");");
}

}
}

/*
 * Copyright 2017 The University of Oklahoma.
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

#include "hudorientationbackimpl.h"

namespace Soro {
namespace MissionControl {

HudOrientationBackImpl::HudOrientationBackImpl(QQuickItem *parent): AbstractHudOrientationImpl(parent)
{

}

void HudOrientationBackImpl::paint(QPainter *painter) {
    painter->setRenderHint(QPainter::Antialiasing);
    //painter->rotate(90);

    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidth(width() / 20);
    painter->setPen(pen);

    // Draw front axel
    QPainterPath frontAxelPath;
    frontAxelPath.moveTo(width() * 2 / 5 - pen.width(), height() / 2);
    frontAxelPath.lineTo(width() - width() * 2 / 5 + pen.width(), height() / 2);
    painter->drawPath(frontAxelPath);

    // Draw FL wheel
    QPainterPath flPath;
    flPath.moveTo(0, height() * 4 / 15);
    flPath.lineTo(width() / 5, height() * 4 / 15);
    flPath.lineTo(width() * 2 / 5, height() / 2);
    flPath.lineTo(width() / 5, height() * 11 / 15);
    flPath.lineTo(0, height() * 11 / 15);
    flPath.closeSubpath();
    painter->fillPath(flPath, QBrush(Qt::white));

    // Draw FR wheel
    QPainterPath frPath;
    frPath.moveTo(width(), height() * 4 / 15);
    frPath.lineTo(width() - width() / 5, height() * 4 / 15);
    frPath.lineTo(width() - width() * 2 / 5, height() / 2);
    frPath.lineTo(width() - width() / 5, height() * 11 / 15);
    frPath.lineTo(width(), height()  * 11 / 15);
    frPath.closeSubpath();
    painter->fillPath(frPath, QBrush(Qt::white));
}

float HudOrientationBackImpl::getRearRoll() const {
    return _rearRoll;
}

void HudOrientationBackImpl::setRearRoll(float rearRoll) {
    _rearRoll = rearRoll;
    update();
}

float HudOrientationBackImpl::getFrontRoll() const {
    return _frontRoll;
}

void HudOrientationBackImpl::setFrontRoll(float frontRoll) {
    _frontRoll = frontRoll;
    update();
}

} // namespace MissionControl
} // namespace Soro

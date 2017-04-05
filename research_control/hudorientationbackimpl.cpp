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

    //
    // Draw FRONT wheel
    //
    painter->resetTransform();
    painter->translate(width() / 2, height() / 2);
    painter->rotate(1.5 * -((_frontRoll - 100.0) * (180.0/800.0) - 90.0));

    QPen pen3;
    pen3.setColor(Qt::red);
    pen3.setWidth(width() / 20);
    painter->setPen(pen3);

    // Draw front axel
    QPainterPath frontAxelPath3;
    frontAxelPath3.moveTo(width() * 2 / 5 - pen3.width() - (width() / 2), 0);
    frontAxelPath3.lineTo((width() / 2) - width() * 2 / 5 + pen3.width(), 0);
    painter->drawPath(frontAxelPath3);

    // Draw FL wheel
    QPainterPath flPath3;
    flPath3.moveTo(-width() / 2, height() * 4 / 15 - (height() / 2));
    flPath3.lineTo(width() / 5 - (width() / 2), height() * 4 / 15 - (height() / 2));
    flPath3.lineTo(width() * 2 / 5 - (width() / 2), 0);
    flPath3.lineTo(width() / 5 - (width() / 2), height() * 11 / 15 - (height() / 2));
    flPath3.lineTo(-width() / 2, height() * 11 / 15 - (height() / 2));
    flPath3.closeSubpath();
    painter->fillPath(flPath3, QBrush(Qt::red));

    // Draw FR wheel
    QPainterPath frPath3;
    frPath3.moveTo(width() / 2, height() * 4 / 15 - (height() / 2));
    frPath3.lineTo(width() / 2 - width() / 5, height() * 4 / 15 - (height() / 2));
    frPath3.lineTo(width() / 2 - width() * 2 / 5, 0);
    frPath3.lineTo(width() / 2 - width() / 5, height() * 11 / 15 - (height() / 2));
    frPath3.lineTo(width() / 2, height()  * 11 / 15 - (height() / 2));
    frPath3.closeSubpath();
    painter->fillPath(frPath3, QBrush(Qt::red));

    //
    // Draw MIDDLE wheel
    //
    painter->resetTransform();
    painter->translate(width() / 2, height() / 2);
    painter->rotate(1.5 * -((_rearRoll - 100.0) * (180.0/800.0) - 90.0));

    QPen pen2;
    pen2.setColor(Qt::green);
    pen2.setWidth(width() / 20);
    painter->setPen(pen2);

    // Draw front axel
    QPainterPath frontAxelPath2;
    frontAxelPath2.moveTo(width() * 2 / 5 - pen2.width() - (width() / 2), 0);
    frontAxelPath2.lineTo((width() / 2) - width() * 2 / 5 + pen2.width(), 0);
    painter->drawPath(frontAxelPath2);

    // Draw mast
    QPainterPath mastPath;
    mastPath.moveTo(0, 0);
    mastPath.lineTo(0, -height() / 2);
    painter->drawPath(mastPath);

    // Draw FL wheel
    QPainterPath flPath2;
    flPath2.moveTo(-width() / 2, height() * 4 / 15 - (height() / 2));
    flPath2.lineTo(width() / 5 - (width() / 2), height() * 4 / 15 - (height() / 2));
    flPath2.lineTo(width() * 2 / 5 - (width() / 2), 0);
    flPath2.lineTo(width() / 5 - (width() / 2), height() * 11 / 15 - (height() / 2));
    flPath2.lineTo(-width() / 2, height() * 11 / 15 - (height() / 2));
    flPath2.closeSubpath();
    painter->fillPath(flPath2, QBrush(Qt::green));

    // Draw FR wheel
    QPainterPath frPath2;
    frPath2.moveTo(width() / 2, height() * 4 / 15 - (height() / 2));
    frPath2.lineTo(width() / 2 - width() / 5, height() * 4 / 15 - (height() / 2));
    frPath2.lineTo(width() / 2 - width() * 2 / 5, 0);
    frPath2.lineTo(width() / 2 - width() / 5, height() * 11 / 15 - (height() / 2));
    frPath2.lineTo(width() / 2, height()  * 11 / 15 - (height() / 2));
    frPath2.closeSubpath();
    painter->fillPath(frPath2, QBrush(Qt::green));

    //
    // Draw BACK wheel
    //
    painter->resetTransform();
    painter->translate(width() / 2, height() / 2);

    QPen pen;
    pen.setColor(Qt::blue);
    pen.setWidth(width() / 20);
    painter->setPen(pen);

    // Draw front axel
    QPainterPath frontAxelPath;
    frontAxelPath.moveTo(width() * 2 / 5 - pen.width() - (width() / 2), 0);
    frontAxelPath.lineTo((width() / 2) - width() * 2 / 5 + pen.width(), 0);
    painter->drawPath(frontAxelPath);

    // Draw FL wheel
    QPainterPath flPath;
    flPath.moveTo(-width() / 2, height() * 4 / 15 - (height() / 2));
    flPath.lineTo(width() / 5 - (width() / 2), height() * 4 / 15 - (height() / 2));
    flPath.lineTo(width() * 2 / 5 - (width() / 2), 0);
    flPath.lineTo(width() / 5 - (width() / 2), height() * 11 / 15 - (height() / 2));
    flPath.lineTo(-width() / 2, height() * 11 / 15 - (height() / 2));
    flPath.closeSubpath();
    painter->fillPath(flPath, QBrush(Qt::blue));

    // Draw FR wheel
    QPainterPath frPath;
    frPath.moveTo(width() / 2, height() * 4 / 15 - (height() / 2));
    frPath.lineTo(width() / 2 - width() / 5, height() * 4 / 15 - (height() / 2));
    frPath.lineTo(width() / 2 - width() * 2 / 5, 0);
    frPath.lineTo(width() / 2 - width() / 5, height() * 11 / 15 - (height() / 2));
    frPath.lineTo(width() / 2, height()  * 11 / 15 - (height() / 2));
    frPath.closeSubpath();
    painter->fillPath(frPath, QBrush(Qt::blue));
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

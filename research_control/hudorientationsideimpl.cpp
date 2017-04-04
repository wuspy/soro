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

#include "hudorientationsideimpl.h"

namespace Soro {
namespace MissionControl {

HudOrientationSideImpl::HudOrientationSideImpl(QQuickItem *parent): AbstractHudOrientationImpl(parent)
{

}

void HudOrientationSideImpl::paint(QPainter *painter) {
    painter->setRenderHint(QPainter::Antialiasing);
    //painter->rotate(90);

    int wheelSize = height() / 4;

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(Qt::white));

    // Draw back wheel
    painter->drawEllipse(QRectF(0, height() / 2 - wheelSize / 2, wheelSize, wheelSize));

    // Draw middle wheel
    painter->drawEllipse(QRectF(width() / 2 - wheelSize / 2, height() / 2 - wheelSize / 2, wheelSize, wheelSize));

    // Draw front wheel
    painter->drawEllipse(QRectF(width() - wheelSize, height() / 2 - wheelSize / 2, wheelSize, wheelSize));

    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidth(height() / 40);

    // Draw connecting line from back to middle wheel
    QPainterPath bmPath;
    bmPath.moveTo(wheelSize / 2, height() / 2);
    bmPath.lineTo(width() / 2, height() / 2);
    painter->strokePath(bmPath, pen);

    // Draw connecting line from middle to front wheel
    QPainterPath mfPath;
    mfPath.moveTo(width() / 2, height() / 2);
    mfPath.lineTo(width() - wheelSize / 2, height() / 2);
    painter->strokePath(mfPath, pen);
}

void HudOrientationSideImpl::setFrontPitch(float frontPitch) {
    _frontPitch = frontPitch;
    update();
}

float HudOrientationSideImpl::getFrontPitch() const {
    return _frontPitch;
}

void HudOrientationSideImpl::setRearPitch(float rearPitch) {
    _rearPitch = rearPitch;
    update();
}

float HudOrientationSideImpl::getRearPitch() const {
    return _rearPitch;
}


} // namespace MissionControl
} // namespace Soro

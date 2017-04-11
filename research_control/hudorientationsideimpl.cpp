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
#include <qmath.h>

namespace Soro {
namespace MissionControl {

HudOrientationSideImpl::HudOrientationSideImpl(QQuickItem *parent): AbstractHudOrientationImpl(parent)
{
    _frontPitch = _frontPitchZero = 400;
    _rearPitch = _rearPitchZero = 400;
}

float HudOrientationSideImpl::pitchToDegrees(float pitch, float pitchZero) {
    return 1.3 * -((pitch - (pitchZero - 500) - 100.0) * (180.0/800.0) - 90.0);
}

float HudOrientationSideImpl::degToRad(float deg) {
    return M_PI * deg / 180;
}

void HudOrientationSideImpl::paint(QPainter *painter) {
    painter->setRenderHint(QPainter::Antialiasing);

    int wheelSize = height() / 4;

    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidth(height() / 40);

    QPointF backCenter, middleCenter, frontCenter;

    backCenter.setX(width() / 2 - cos(degToRad(pitchToDegrees(_rearPitch, _rearPitchZero))) * (width() / 2 - wheelSize / 2));
    backCenter.setY(width() / 2 - sin(degToRad(pitchToDegrees(_rearPitch, _rearPitchZero))) * (width() / 2 - wheelSize / 2));

    frontCenter.setX(width() / 2 + cos(degToRad(pitchToDegrees(_frontPitch, _frontPitchZero))) * (width() / 2 - wheelSize / 2));
    frontCenter.setY(width() / 2 + sin(degToRad(pitchToDegrees(_frontPitch, _frontPitchZero))) * (width() / 2 - wheelSize / 2));

    middleCenter.setX(width() / 2);
    middleCenter.setY(height() / 2);

    // Draw connecting line from back to middle wheel
    QPainterPath bmPath;
    bmPath.moveTo(backCenter.x(), backCenter.y());
    bmPath.lineTo(middleCenter.x(), middleCenter.y());
    painter->strokePath(bmPath, pen);

    // Draw connecting line from middle to front wheel
    QPainterPath mfPath;
    mfPath.moveTo(middleCenter.x(), middleCenter.y());
    mfPath.lineTo(frontCenter.x(), frontCenter.y());
    painter->strokePath(mfPath, pen);

    painter->setPen(Qt::NoPen);

    // Draw back wheel
    painter->setBrush(QBrush(QColor("#2962FF")));
    painter->drawEllipse(QRectF(backCenter.x() - wheelSize / 2, backCenter.y() - wheelSize / 2, wheelSize, wheelSize));

    // Draw middle wheel
    painter->setBrush(QBrush(QColor("#00C853")));
    painter->drawEllipse(QRectF(middleCenter.x() - wheelSize / 2, middleCenter.y() - wheelSize / 2, wheelSize, wheelSize));

    // Draw front wheel
    painter->setBrush(QBrush(QColor("#d50000")));
    painter->drawEllipse(QRectF(frontCenter.x() - wheelSize / 2, frontCenter.y() - wheelSize / 2, wheelSize, wheelSize));
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

void HudOrientationSideImpl::setFrontPitchZero(float frontPitchZero) {
    _frontPitchZero = frontPitchZero;
    update();
}

float HudOrientationSideImpl::getFrontPitchZero() const {
    return _frontPitchZero;
}

void HudOrientationSideImpl::setRearPitchZero(float rearPitchZero) {
    _rearPitchZero = rearPitchZero;
    update();
}

float HudOrientationSideImpl::getRearPitchZero() const {
    return _rearPitchZero;
}

} // namespace MissionControl
} // namespace Soro

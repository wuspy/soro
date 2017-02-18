#include "hudpowerimpl.h"

namespace Soro {
namespace MissionControl {

HudPowerImpl::HudPowerImpl(QQuickItem *parent) : QQuickPaintedItem(parent)
{

}

int HudPowerImpl::wheelFLPower() const {
    return _wheelFLPower;
}

void HudPowerImpl::setWheelFLPower(int power) {
    _wheelFLPower = power;
    update();
}

int HudPowerImpl::wheelFRPower() const {
    return _wheelFRPower;
}

void HudPowerImpl::setWheelFRPower(int power) {
    _wheelFRPower = power;
    update();
}

int HudPowerImpl::wheelMLPower() const {
    return _wheelMLPower;
}

void HudPowerImpl::setWheelMLPower(int power) {
    _wheelMLPower = power;
    update();
}

int HudPowerImpl::wheelMRPower() const {
    return _wheelMRPower;
}

void HudPowerImpl::setWheelMRPower(int power) {
    _wheelMRPower = power;
    update();
}

int HudPowerImpl::wheelBLPower() const {
    return _wheelBLPower;
}

void HudPowerImpl::setWheelBLPower(int power) {
    _wheelBLPower = power;
    update();
}

int HudPowerImpl::wheelBRPower() const {
    return _wheelBRPower;
}

void HudPowerImpl::setWheelBRPower(int power) {
    _wheelBRPower = power;
    update();
}

int HudPowerImpl::baselinePower() const {
    return _baselinePower;
}

void HudPowerImpl::setBaselinePower(int power) {
    _baselinePower = power;
    update();
}

int HudPowerImpl::criticalPower() const {
    return _criticalPower;
}

void HudPowerImpl::setCriticalPower(int power) {
    _criticalPower = power;
    update();
}

float HudPowerImpl::wheelColorValue() const {
    return _wheelColorValue;
}

void HudPowerImpl::setWheelColorValue(float value) {
    _wheelColorValue = value;
    update();
}

float HudPowerImpl::wheelColorSaturation() const {
    return _wheelColorSaturation;
}

void HudPowerImpl::setWheelColorSaturation(float saturation) {
    _wheelColorSaturation = saturation;
    update();
}

float HudPowerImpl::wheelColorBaselineHue() const {
    return _wheelColorBaselineHue;
}

void HudPowerImpl::setWheelColorBaselineHue(float hue) {
    _wheelColorBaselineHue = hue;
    update();
}

float HudPowerImpl::wheelColorCriticalHue() const {
    return _wheelColorCriticalHue;
}

void HudPowerImpl::setWheelColorCriticalHue(float hue) {
    _wheelColorCriticalHue = hue;
    update();
}

float HudPowerImpl::clamp(float value) {
    if (value < 0) return 0;
    if (value > 1) return 1;
    return value;
}

void HudPowerImpl::paint(QPainter *painter) {
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidth(width() / 30);
    painter->setPen(pen);

    float hueRange = _wheelColorCriticalHue - _wheelColorBaselineHue;
    float powerRange = _criticalPower - _baselinePower;

    float flRatio = clamp((_wheelFLPower - _baselinePower) / powerRange);
    float frRatio = clamp((_wheelFRPower - _baselinePower) / powerRange);
    float mlRatio = clamp((_wheelMLPower - _baselinePower) / powerRange);
    float mrRatio = clamp((_wheelMRPower - _baselinePower) / powerRange);
    float blRatio = clamp((_wheelBLPower - _baselinePower) / powerRange);
    float brRatio = clamp((_wheelBRPower - _baselinePower) / powerRange);

    // Draw front axel
    QPainterPath frontAxelPath;
    frontAxelPath.moveTo(width() * 2 / 5 - pen.width(), height() / 8);
    frontAxelPath.lineTo(width() - width() * 2 / 5 + pen.width(), height() / 8);
    painter->drawPath(frontAxelPath);

    // Draw middle axel
    QPainterPath middleAxelPath;
    middleAxelPath.moveTo(width() * 2 / 5 - pen.width(), height() / 2);
    middleAxelPath.lineTo(width() - width() * 2 / 5 + pen.width(), height() / 2);
    painter->drawPath(middleAxelPath);

    // Draw back axel
    QPainterPath backAxelPath;
    backAxelPath.moveTo(width() * 2 / 5 - pen.width(), height() - height() / 8);
    backAxelPath.lineTo(width() - width() * 2 / 5 + pen.width(), height() - height() / 8);
    painter->drawPath(backAxelPath);

    // Draw frame
    QPainterPath framePath;
    framePath.moveTo(width() / 2, height() / 8);
    framePath.lineTo(width() / 2, height() - height() / 8);
    painter->drawPath(framePath);

    // Draw FL wheel
    QPainterPath flPath;
    flPath.moveTo(0, 0);
    flPath.lineTo(width() / 5, 0);
    flPath.lineTo(width() * 2 / 5, height() / 8);
    flPath.lineTo(width() / 5, height() / 4);
    flPath.lineTo(0, height() / 4);
    flPath.closeSubpath();
    painter->fillPath(flPath, QBrush(_wheelFLPower > 0 ? QColor::fromHsvF(_wheelColorBaselineHue + flRatio * hueRange, _wheelColorSaturation, _wheelColorValue) : Qt::darkGray));

    // Draw FR wheel
    QPainterPath frPath;
    frPath.moveTo(width(), 0);
    frPath.lineTo(width() - width() / 5, 0);
    frPath.lineTo(width() - width() * 2 / 5, height() / 8);
    frPath.lineTo(width() - width() / 5, height() / 4);
    frPath.lineTo(width(), height() / 4);
    frPath.closeSubpath();
    painter->fillPath(frPath, QBrush(_wheelFRPower > 0 ? QColor::fromHsvF(_wheelColorBaselineHue + frRatio * hueRange, _wheelColorSaturation, _wheelColorValue) : Qt::darkGray));

    // Draw ML wheel
    QPainterPath mlPath;
    mlPath.moveTo(0, height() * 3 / 8);
    mlPath.lineTo(width() / 5, height() * 3 / 8);
    mlPath.lineTo(width() * 2 / 5, height() / 2);
    mlPath.lineTo(width() / 5, height() * 5 / 8);
    mlPath.lineTo(0, height() * 5 / 8);
    mlPath.closeSubpath();
    painter->fillPath(mlPath, QBrush(_wheelMLPower > 0 ? QColor::fromHsvF(_wheelColorBaselineHue + mlRatio * hueRange, _wheelColorSaturation, _wheelColorValue) : Qt::darkGray));

    // Draw MR wheel
    QPainterPath mrPath;
    mrPath.moveTo(width(), height() * 3 / 8);
    mrPath.lineTo(width() - width() / 5, height() * 3 / 8);
    mrPath.lineTo(width() - width() * 2 / 5, height() / 2);
    mrPath.lineTo(width() - width() / 5, height() * 5 / 8);
    mrPath.lineTo(width(), height() * 5 / 8);
    mrPath.closeSubpath();
    painter->fillPath(mrPath, QBrush(_wheelMRPower > 0 ? QColor::fromHsvF(_wheelColorBaselineHue + mrRatio * hueRange, _wheelColorSaturation, _wheelColorValue) : Qt::darkGray));

    // Draw BL wheel
    QPainterPath blPath;
    blPath.moveTo(0, height());
    blPath.lineTo(width() / 5, height());
    blPath.lineTo(width() * 2 / 5, height() - height() / 8);
    blPath.lineTo(width() / 5, height() - height() / 4);
    blPath.lineTo(0, height() - height() / 4);
    blPath.closeSubpath();
    painter->fillPath(blPath, QBrush(_wheelBLPower > 0 ? QColor::fromHsvF(_wheelColorBaselineHue + blRatio * hueRange, _wheelColorSaturation, _wheelColorValue) : Qt::darkGray));

    // Draw BR wheel
    QPainterPath brPath;
    brPath.moveTo(width(), height());
    brPath.lineTo(width() - width() / 5, height());
    brPath.lineTo(width() - width() * 2 / 5, height() - height() / 8);
    brPath.lineTo(width() - width() / 5, height() - height() / 4);
    brPath.lineTo(width(), height() - height() / 4);
    brPath.closeSubpath();
    painter->fillPath(brPath, QBrush(_wheelBRPower > 0 ? QColor::fromHsvF(_wheelColorBaselineHue + brRatio * hueRange, _wheelColorSaturation, _wheelColorValue) : Qt::darkGray));
}

} // namespace MissionControl
} // namespace Soro

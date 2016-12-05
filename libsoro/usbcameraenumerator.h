#ifndef SORO_USBCAMERAENUMERATOR_H
#define SORO_USBCAMERAENUMERATOR_H

#include <QList>
#include "soro_global.h"

namespace Soro {

struct UsbCamera {
    QString name;
    QString vendorId;
    QString productId;
    QString serial;
    QString device;

    QString toString() const {
        QString str = "{";
        if (!name.isEmpty()) {
            str += name + ",";
        }
        if (!vendorId.isEmpty()) {
            str += "v" + vendorId + ",";
        }
        if (!productId.isEmpty()) {
            str += "d" + productId + ",";
        }
        if (!serial.isEmpty()) {
            str += "s" + serial + ",";
        }
        str += device + "}";
        return str;
    }
};

class LIBSORO_EXPORT UsbCameraEnumerator {

public:
    /* Enumerates all USB cameras connected. If successful, this will
     * return the number of cameras detected, otherwise -1 will be returned.
     */
    int loadCameras();

    const QList<UsbCamera*>& listDevices() const;

    /* Returns the first camera matching all of the specified properties. If no cameras matched, this will
     * return null.
     */
    const UsbCamera* find(QString name="", QString device="", QString vid="", QString pid="", QString serial="") const;

    ~UsbCameraEnumerator();

protected:
    void clearList();

private:
    QList<UsbCamera*> _cameras;
};

} // namespace Soro

#endif // SORO_USBCAMERAENUMERATOR_H

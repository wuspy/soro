#ifndef MEDIAFORMAT_H
#define MEDIAFORMAT_H

#include <QtCore>

namespace Soro {

struct MediaFormat {

    virtual ~MediaFormat() { }

    virtual QString toHumanReadableString() const=0;
    virtual QString createGstEncodingArgs() const=0;
    virtual QString createGstDecodingArgs() const=0;

    virtual bool isUseable() const=0;

    virtual QString serialize() const=0;
    virtual void deserialize(QString serial)=0;
};

} // namespace Soro

#endif // MEDIAFORMAT_H

#ifndef MBEDDATAPARSER_H
#define MBEDDATAPARSER_H

#include "mbedchannel.h"
#include "abstractdatarecorder.h"

namespace Soro {

/* This class is responsible for logging the data sent back by the research mbed, including
 * IMU and power consumption data. These values are stored in a binary logfile and timestamped;
 * additionally they can be accessed immediately by attaching to the newData() signal.
 */
class LIBSORO_EXPORT SensorDataRecorder : public AbstractDataRecorder
{
    Q_OBJECT
public:
    explicit SensorDataRecorder(QObject *parent = 0);

    enum DataTag {
        DATATAG_WHEELDATA_1 = 0,
        DATATAG_WHEELDATA_2,
        DATATAG_WHEELDATA_3,
        DATATAG_WHEELDATA_4,
        DATATAG_WHEELDATA_5,
        DATATAG_WHEELDATA_6,
        DATATAG_IMUDATA_1_X,
        DATATAG_IMUDATA_1_Y,
        DATATAG_IMUDATA_1_Z,
        DATATAG_IMUDATA_2_X,
        DATATAG_IMUDATA_2_Y,
        DATATAG_IMUDATA_2_Z
    };

private:
    QByteArray _buffer;

    void parseBuffer();
    bool parseNext(DataTag tag, int start);

public slots:
    void newData(const char* data, int len);

signals:
    void dataParsed(SensorDataRecorder::DataTag tag, float value);
};

} // namespace Soro

#endif // MBEDDATAPARSER_H

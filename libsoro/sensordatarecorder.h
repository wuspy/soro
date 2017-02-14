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

    static const char DATATAG_WHEELDATA_A = 'A';
    static const char DATATAG_WHEELDATA_B = 'B';
    static const char DATATAG_WHEELDATA_C = 'C';
    static const char DATATAG_WHEELDATA_D = 'D';
    static const char DATATAG_WHEELDATA_E = 'E';
    static const char DATATAG_WHEELDATA_F = 'F';
    static const char DATATAG_IMUDATA_1_X = 'X';
    static const char DATATAG_IMUDATA_1_Y = 'Y';
    static const char DATATAG_IMUDATA_1_Z = 'Z';
    static const char DATATAG_IMUDATA_2_X = 'P';
    static const char DATATAG_IMUDATA_2_Y = 'Q';
    static const char DATATAG_IMUDATA_2_Z = 'R';
    static const char DATATAG_ERROR = '?';

    explicit SensorDataRecorder(QObject *parent = nullptr);

    bool isValidTag(char c);

private:
    QByteArray _buffer;

    void parseBuffer(bool logErrors);

public slots:
    void newData(const char* data, int len);

signals:
    void dataParsed(char tag, int value);
    void parseError();
};

} // namespace Soro

#endif // MBEDDATAPARSER_H

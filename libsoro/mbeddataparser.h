#ifndef MBEDDATAPARSER_H
#define MBEDDATAPARSER_H

#include "mbedchannel.h"
#include "logger.h"

namespace Soro {
namespace Rover {

/* This class is responsible for logging the data sent back by the research mbed, including
 * IMU and power consumption data. These values are stored in a binary logfile and timestamped;
 * additionally they can be accessed immediately by attaching to the newData() signal.
 */
class MbedDataParser : public QObject
{
    Q_OBJECT
public:
    explicit MbedDataParser(MbedChannel *mbed, QObject *parent = 0);
    ~MbedDataParser();

    /* Starts logging data in the specified file, and calculates all timestamps offset from
     * the provided start time.
     */
    bool startLog(QString file, QDateTime loggedStartTime=QDateTime::currentDateTime());

    /* Stops logging, if it is currently active.
     */
    void stopLog();

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
    MbedChannel *_mbed = NULL;
    QByteArray _buffer;
    QFile *_file = NULL;
    QDataStream *_fileStream;

    // Keeps track of the time this logging began, which can be used to find the
    // timestamp of every measurement in the log file
    qint64 _logStartTime = 0;

    void parseBuffer();
    void parseNext(DataTag tag, int start);

private slots:
    void messageReceived(MbedChannel *mbed, const char* data, int len);

signals:
    void newData(DataTag tag, float value);
};

}
}

#endif // MBEDDATAPARSER_H

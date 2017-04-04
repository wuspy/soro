#ifndef SENSORDATAPARSER_H
#define SENSORDATAPARSER_H

#include "soro_global.h"
#include "mbedchannel.h"
#include "csvrecorder.h"

namespace Soro {

/* This class is responsible for logging the data sent back by the research mbed, including
 * IMU and power consumption data. These values are stored in a binary logfile and timestamped;
 * additionally they can be accessed immediately by attaching to the newData() signal.
 */
class LIBSORO_EXPORT SensorDataParser : public QObject
{
    Q_OBJECT

public:

    static const char DATATAG_WHEELPOWER_A = 'A';
    static const char DATATAG_WHEELPOWER_B = 'B';
    static const char DATATAG_WHEELPOWER_C = 'C';
    static const char DATATAG_WHEELPOWER_D = 'D';
    static const char DATATAG_WHEELPOWER_E = 'E';
    static const char DATATAG_WHEELPOWER_F = 'F';
    static const char DATATAG_IMUDATA_REAR_YAW = 'X';
    static const char DATATAG_IMUDATA_REAR_PITCH = 'Y';
    static const char DATATAG_IMUDATA_REAR_ROLL = 'Z';
    static const char DATATAG_IMUDATA_FRONT_YAW = 'P';
    static const char DATATAG_IMUDATA_FRONT_PITCH = 'Q';
    static const char DATATAG_IMUDATA_FRONT_ROLL = 'R';
    static const char DATATAG_ERROR = '?';

    class WheelPowerACsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel A Power"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_A) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerBCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel B Power"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_B) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerCCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel C Power"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_C) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerDCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel D Power"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_D) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerECsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel E Power"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_E) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerFCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel F Power"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_F) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuRearYawCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Rear Yaw"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_REAR_YAW) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuRearPitchCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Rear Pitch"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_REAR_PITCH) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuRearRollCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Rear Roll"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_REAR_ROLL) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuFrontYawCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Front Yaw"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_FRONT_YAW) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuFrontPitchCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Front Pitch"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_FRONT_PITCH) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuFrontRollCsvSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Front Roll"; }
                bool shouldKeepOldValues() const { return true; }
    //private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_FRONT_ROLL) CsvDataSeries::update(QVariant(value)); }
    };

    explicit SensorDataParser(QObject *parent=0);

    bool isValidTag(char c);
    QString getSeriesName() const;

    const WheelPowerACsvSeries* getWheelPowerASeries() const;
    const WheelPowerBCsvSeries* getWheelPowerBSeries() const;
    const WheelPowerCCsvSeries* getWheelPowerCSeries() const;
    const WheelPowerDCsvSeries* getWheelPowerDSeries() const;
    const WheelPowerECsvSeries* getWheelPowerESeries() const;
    const WheelPowerFCsvSeries* getWheelPowerFSeries() const;
    const ImuRearYawCsvSeries* getImuRearYawSeries() const;
    const ImuRearPitchCsvSeries* getImuRearPitchSeries() const;
    const ImuRearRollCsvSeries* getImuRearRollSeries() const;
    const ImuFrontYawCsvSeries* getImuFrontYawSeries() const;
    const ImuFrontPitchCsvSeries* getImuFrontPitchSeries() const;
    const ImuFrontRollCsvSeries* getImuFrontRollSeries() const;

private:
    QByteArray _buffer;
    WheelPowerACsvSeries _wheelPowerASeries;
    WheelPowerBCsvSeries _wheelPowerBSeries;
    WheelPowerCCsvSeries _wheelPowerCSeries;
    WheelPowerDCsvSeries _wheelPowerDSeries;
    WheelPowerECsvSeries _wheelPowerESeries;
    WheelPowerFCsvSeries _wheelPowerFSeries;
    ImuRearYawCsvSeries _imuRearYawSeries;
    ImuRearPitchCsvSeries _imuRearPitchSeries;
    ImuRearRollCsvSeries _imuRearRollSeries;
    ImuFrontYawCsvSeries _imuFrontYawSeries;
    ImuFrontPitchCsvSeries _imuFrontPitchSeries;
    ImuFrontRollCsvSeries _imuFrontRollSeries;

    void parseBuffer();

public slots:
    void newData(const char* data, int len);

signals:
    void dataParsed(char tag, int value);
    void parseError();
};

} // namespace Soro

#endif // SENSORDATAPARSER_H

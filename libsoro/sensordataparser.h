#ifndef MBEDDATAPARSER_H
#define MBEDDATAPARSER_H

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

    class WheelPowerASeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel A Power"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_A) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerBSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel B Power"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_B) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerCSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel C Power"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_C) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerDSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel D Power"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_D) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerESeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel E Power"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_E) CsvDataSeries::update(QVariant(value)); }
    };
    class WheelPowerFSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Wheel F Power"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_WHEELPOWER_F) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuRearYawSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Rear Yaw"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_REAR_YAW) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuRearPitchSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Rear Pitch"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_REAR_PITCH) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuRearRollSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Rear Roll"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_REAR_ROLL) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuFrontYawSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Front Yaw"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_FRONT_YAW) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuFrontPitchSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Front Pitch"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_FRONT_PITCH) CsvDataSeries::update(QVariant(value)); }
    };
    class ImuFrontRollSeries : public CsvDataSeries { friend class SensorDataParser;
    public:     QString getSeriesName() const { return "Front Roll"; }
    private:    void update(char tag, int value) { if (tag == DATATAG_IMUDATA_FRONT_ROLL) CsvDataSeries::update(QVariant(value)); }
    };

    explicit SensorDataParser(QObject *parent=0);

    bool isValidTag(char c);
    QString getSeriesName() const;

    const WheelPowerASeries* getWheelPowerASeries() const;
    const WheelPowerBSeries* getWheelPowerBSeries() const;
    const WheelPowerCSeries* getWheelPowerCSeries() const;
    const WheelPowerDSeries* getWheelPowerDSeries() const;
    const WheelPowerESeries* getWheelPowerESeries() const;
    const WheelPowerFSeries* getWheelPowerFSeries() const;
    const ImuRearYawSeries* getImuRearYawSeries() const;
    const ImuRearPitchSeries* getImuRearPitchSeries() const;
    const ImuRearRollSeries* getImuRearRollSeries() const;
    const ImuFrontYawSeries* getImuFrontYawSeries() const;
    const ImuFrontPitchSeries* getImuFrontPitchSeries() const;
    const ImuFrontRollSeries* getImuFrontRollSeries() const;

private:
    QByteArray _buffer;
    WheelPowerASeries _wheelPowerASeries;
    WheelPowerBSeries _wheelPowerBSeries;
    WheelPowerCSeries _wheelPowerCSeries;
    WheelPowerDSeries _wheelPowerDSeries;
    WheelPowerESeries _wheelPowerESeries;
    WheelPowerFSeries _wheelPowerFSeries;
    ImuRearYawSeries _imuRearYawSeries;
    ImuRearPitchSeries _imuRearPitchSeries;
    ImuRearRollSeries _imuRearRollSeries;
    ImuFrontYawSeries _imuFrontYawSeries;
    ImuFrontPitchSeries _imuFrontPitchSeries;
    ImuFrontRollSeries _imuFrontRollSeries;

    void parseBuffer(bool logErrors);

public slots:
    void newData(const char* data, int len);

signals:
    void dataParsed(char tag, int value);
    void parseError();
};

} // namespace Soro

#endif // MBEDDATAPARSER_H

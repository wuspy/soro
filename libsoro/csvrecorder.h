#ifndef CSVRECORDER_H
#define CSVRECORDER_H

#include <QDateTime>
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTimerEvent>

#include "soro_global.h"
#include "constants.h"

namespace Soro {

/* Abstract class to represent a data series.
 * Accepts QVariants for data
 */
class LIBSORO_EXPORT CsvDataSeries {
public:
    QVariant getValue() const;
    qint64 getValueAge() const;
    virtual QString getSeriesName() const=0;

protected:
    void update(QVariant value);

private:
    QVariant _value;
    qint64 _valueTime = 0;
};

/* Class to record data at a regular interval in a CSV formatted file
 */
class LIBSORO_EXPORT CsvRecorder : public QObject
{
    Q_OBJECT
public:
    CsvRecorder(QObject *parent=0);

    bool isRecording() const;
    qint64 getStartTime() const;

    void addColumn(const CsvDataSeries* series);
    void removeColumn(const CsvDataSeries* series);
    void clearColumns();
    void setUpdateInterval(int interval);

    int getUpdateInterval() const;
    const QList<const CsvDataSeries*>& getColumns() const;

public slots:
    /* Starts logging data in the specified file, and calculates all timestamps offset from
     * the provided start time.
     */
    bool startLog(QDateTime loggedStartTime);

    /* Stops logging, if it is currently active.
     */
    void stopLog();

signals:
    void logStarted(QDateTime loggedStartTime);
    void logStopped();

protected:
    void timerEvent(QTimerEvent *e);

private:
    QList<const CsvDataSeries*> _columns;
    int _updateTimerId = TIMER_INACTIVE;
    QTextStream *_fileStream = nullptr;
    QString _logDir;
    int _updateInterval;
    QFile *_file = nullptr;
    qint64 _logStartTime;
    bool _isRecording=false;
};

} // namespace Soro

#endif // CSVRECORDER_H

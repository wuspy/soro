#ifndef ABSTRACTDATARECORDER_H
#define ABSTRACTDATARECORDER_H

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QDataStream>

#include "soro_global.h"

namespace Soro {

/* Abstract class implemented by any class responsible for logging test data for the research
 * project
 *
 * All data is recorded in a big endian binary file
 */
class LIBSORO_EXPORT AbstractDataRecorder : public QObject
{
    Q_OBJECT

public:
    ~AbstractDataRecorder();

    bool isRecording() const;

    qint64 getStartTime() const;

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
    AbstractDataRecorder(QString logTag, QString logDirectory, QObject *parent=0);
    void addTimestamp();
    QDataStream *_fileStream = nullptr;

private:
    QString _logTag;
    QString _logDir;
    QFile *_file = nullptr;
    qint64 _logStartTime;
    bool _isRecording=false;
};

} // namespace Soro

#endif // ABSTRACTDATARECORDER_H

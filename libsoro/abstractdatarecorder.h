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
 */
class LIBSORO_EXPORT AbstractDataRecorder : public QObject
{
    Q_OBJECT

public:
    ~AbstractDataRecorder();

    /* Starts logging data in the specified file, and calculates all timestamps offset from
     * the provided start time.
     */
    bool startLog(QString file, QDateTime loggedStartTime=QDateTime::currentDateTime());

    /* Stops logging, if it is currently active.
     */
    void stopLog();

    bool isRecording();

protected:
    AbstractDataRecorder(QString logTag, QObject *parent=0);
    void addTimestamp();
    QDataStream *_fileStream = nullptr;

private:
    QString _logTag;
    QFile *_file = nullptr;
    qint64 _logStartTime;
    bool _isRecording=false;
};

} // namespace Soro

#endif // ABSTRACTDATARECORDER_H

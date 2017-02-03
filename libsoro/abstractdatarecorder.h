#ifndef ABSTRACTDATARECORDER_H
#define ABSTRACTDATARECORDER_H

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QDataStream>

namespace Soro {

/* Abstract class implemented by any class responsible for logging test data for the research
 * project
 */
class AbstractDataRecorder : public QObject
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

protected:
    AbstractDataRecorder(QString logTag, QObject *parent=0);
    inline bool canRecordData() { return _isRecording; }
    void recordData(QByteArray data);

private:
    QString _logTag;
    QFile *_file = NULL;
    QDataStream *_fileStream = NULL;
    qint64 _logStartTime;
    bool _isRecording=false;
};

} // namespace Soro

#endif // ABSTRACTDATARECORDER_H

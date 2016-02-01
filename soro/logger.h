#ifndef LOGGER_H
#define LOGGER_H

#include "soro_global.h"

/* A very simple logging implementation. Provides functionality to output
 * log data to a file as well signaling when messages are published.
 */
class SOROSHARED_EXPORT Logger: public QObject
{
    Q_OBJECT

private:
    static const QString LOG_TAG;
    static const QString _levelStrings[];
    static const QString _levelFormatters[];
    QFile* _file;
    QTextStream* _fileStream;
    void publish(qint32 level, QString tag, QString message);
public:
    Logger(QObject *parent);
    static const qint32 LEVEL_INFORMATION = 0;
    static const qint32 LEVEL_WARN = 1;
    static const qint32 LEVEL_ERROR = 2;
    bool setLogfile(QString dir);
    void closeLogfile();
    void i(QString tag, QString message);
    void w(QString tag, QString message);
    void e(QString tag, QString message);
signals:
    void onLogMessagePublished(qint32 level, QString tag, QString message);
    void onLogMessagePublished(QString messageFormatted);
};

#endif // LOGGER_H

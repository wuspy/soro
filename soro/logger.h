#ifndef LOGGER_H
#define LOGGER_H

#include "soro_global.h"

#define LOG_LEVEL_DEBUG 3
#define LOG_LEVEL_INFORMATION 2
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_ERROR 0

/* A very simple logging implementation. Provides functionality to output
 * log data to a file as well signaling when messages are published.
 *
 * This class is reentrant, because QTextStream is reentrant and mutexes are expensive.
 * Create separate logger instances for separate threads.
 */
class SOROSHARED_EXPORT Logger: public QObject
{
    Q_OBJECT

private:
    static const QString _levelFormatters[];
    QFile* _file;
    QTextStream* _fileStream;
    inline void publish(int level, QString tag, QString message);

public:
    Logger(QObject *parent);
    ~Logger();

    bool setLogfile(QString dir);
    void closeLogfile();
    void d(QString tag, QString message);
    void i(QString tag, QString message);
    void w(QString tag, QString message);
    void e(QString tag, QString message);
    int MaxLevel;
    bool RouteToQtLogger;

signals:
    void logMessagePublished(int level, QString tag, QString message);
    void logMessagePublished(QString messageFormatted);
};

#endif // LOGGER_H

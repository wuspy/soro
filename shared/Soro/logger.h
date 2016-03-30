#ifndef LOGGER_H
#define LOGGER_H

#define LOG_LEVEL_DEBUG 3
#define LOG_LEVEL_INFORMATION 2
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_DISABLED -1

#include <QtCore>

/* If a class has a Logger named _log, and a variable (or define)
 * named LOG_TAG, it can use these macros to more concisely
 * write log messages.
 * These macros also check to make sure _log isn't null, which
 * can also make it easier to add support for disabling the log
 * entirely.
 */
#define LOG_D(X) if (_log != NULL) _log->d(LOG_TAG, X)
#define LOG_I(X) if (_log != NULL) _log->i(LOG_TAG, X)
#define LOG_W(X) if (_log != NULL) _log->w(LOG_TAG, X)
#define LOG_E(X) if (_log != NULL) _log->e(LOG_TAG, X)

namespace Soro {

/* A very simple logging implementation. Provides functionality to output
 * log data to a file as well signaling when messages are published.
 *
 * This class is reentrant, because QTextStream is reentrant and mutexes are expensive.
 * Create separate logger instances for separate threads.
 */
class Logger: public QObject
{
    Q_OBJECT

private:
    static const QString _levelFormatters[];
    QFile* _file = NULL;
    QTextStream* _fileStream = NULL;
    inline void publish(int level, QString tag, QString message);

public:
    Logger(QObject *parent);
    ~Logger();

    bool setLogfile(QString fileName);
    void closeLogfile();
    void d(QString tag, QString message);
    void i(QString tag, QString message);
    void w(QString tag, QString message);
    void e(QString tag, QString message);
    int MaxLevel;
    int MaxQtLoggerLevel;
    bool RouteToQtLogger;

signals:
    void logMessagePublished(int level, QString tag, QString message);
    void logMessagePublished(QString messageFormatted);
};

}

#endif // LOGGER_H

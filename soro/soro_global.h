#ifndef SORO_GLOBAL_H
#define SORO_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtNetwork>

#if defined(SORO_LIBRARY)
#  define SOROSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SOROSHARED_EXPORT Q_DECL_IMPORT
#endif

//Log macros used by channel and watchdog
#define LOG_I(X) if (_log != NULL) _log->i(LOG_TAG, X)
#define LOG_W(X) if (_log != NULL) _log->w(LOG_TAG, X)
#define LOG_E(X) if (_log != NULL) _log->e(LOG_TAG, X)

#endif // SORO_GLOBAL_H

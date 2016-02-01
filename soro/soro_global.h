#ifndef SORO_GLOBAL_H
#define SORO_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtNetwork>

#define INT64_BYTES 8
#define INT32_BYTES 4
#define INT16_BYTES 2
#define INT8_BYTES 1

#define VERSION 1

#if defined(SORO_LIBRARY)
#  define SOROSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SOROSHARED_EXPORT Q_DECL_IMPORT
#endif

typedef int channelid;

#endif // SORO_GLOBAL_H

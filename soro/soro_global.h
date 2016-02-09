#ifndef SORO_GLOBAL_H
#define SORO_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtNetwork>

#if defined(SORO_LIBRARY)
#  define SOROSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SOROSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SORO_GLOBAL_H

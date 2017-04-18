#ifndef SORO_GLOBAL_H
#define SORO_GLOBAL_H

#include <QtCore/QtGlobal>

#if defined(SORO_COMMON_LIBRARY)
#  define LIBSORO_EXPORT Q_DECL_EXPORT
#else
#  define LIBSORO_EXPORT Q_DECL_IMPORT
#endif

// Force VideoFormat to use VAAPI encoders
#define USE_VAAPI_ENCODE

#endif // SORO_GLOBAL_H

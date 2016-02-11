#ifndef SOROUI_GLOBAL_H
#define SOROUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SOROUI_LIBRARY)
#  define SOROUISHARED_EXPORT Q_DECL_EXPORT
#else
#  define SOROUISHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SOROUI_GLOBAL_H

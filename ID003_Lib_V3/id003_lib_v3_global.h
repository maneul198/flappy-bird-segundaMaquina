#ifndef ID003_LIB_V3_GLOBAL_H
#define ID003_LIB_V3_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ID003_LIB_V3_LIBRARY)
#  define ID003_LIB_V3SHARED_EXPORT Q_DECL_EXPORT
#else
#  define ID003_LIB_V3SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ID003_LIB_V3_GLOBAL_H

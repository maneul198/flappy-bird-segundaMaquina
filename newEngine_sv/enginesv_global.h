#ifndef ENGINESV_GLOBAL_H
    #define ENGINESV_GLOBAL_H
    
    #include <QtCore/qglobal.h>
    
    #if defined(ENGINESV_LIBRARY)
        #define ENGINESVSHARED_EXPORT Q_DECL_EXPORT
    #else
        #define ENGINESVSHARED_EXPORT Q_DECL_IMPORT
    #endif
    
#endif // ENGINESV_GLOBAL_H

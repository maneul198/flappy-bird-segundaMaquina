/*
 * File:   KbiredUtilities.h
 * Author: sebastian
 *
 * Created on April 2, 2012, 4:04 PM
 */


#define ARM_DEVICE false

#define RELEASE_MODE true

#ifndef KBIREDUTILITIES_H
#define	KBIREDUTILITIES_H


#include <stdarg.h>
#include "stdio.h"
#include "string.h"
#include "syslog.h"

#define MSGSZ     128
#if ARM_DEVICE
#include "android/log.h"
#endif

#if ARM_DEVICE




#define LOGV(...)
#define LOGD(...)
#define LOGI(...)
#define LOGW(...)
#define LOGE(...)

// #define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "kbiredLibrary",KbiredUtilities::string_format_kbired(__VA_ARGS__));
// #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "kbiredLibrary",KbiredUtilities::string_format_kbired(__VA_ARGS__));
// #define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "kbiredLibrary",KbiredUtilities::string_format_kbired(__VA_ARGS__));
// #define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "kbiredLibrary",KbiredUtilities::string_format_kbired(__VA_ARGS__));
// #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "kbiredLibrary",KbiredUtilities::string_format_kbired(__VA_ARGS__));

#else

// #if !RELEASE_MODE
//     #define LOGV(...) printf("VERBOSE "); printf(__VA_ARGS__); printf("\n"); syslog(LOG_DEBUG | LOG_AUTHPRIV | LOG_CRON, "%s LIB VERSION %s ","VERBOSE ",LIBRARY_VERSION); syslog(LOG_DEBUG | LOG_AUTHPRIV | LOG_CRON, __VA_ARGS__)
//     #define LOGD(...) printf("DEBUG "); printf(__VA_ARGS__); printf("\n"); syslog(LOG_NOTICE | LOG_AUTHPRIV | LOG_CRON, "%s LIB VERSION %s ","DEBUG ",LIBRARY_VERSION); syslog(LOG_NOTICE | LOG_AUTHPRIV | LOG_CRON, __VA_ARGS__)
// #else
//     #define LOGV(...) syslog(LOG_DEBUG | LOG_AUTHPRIV | LOG_CRON, "%s LIB VERSION %s ","VERBOSE ",LIBRARY_VERSION); syslog(LOG_DEBUG | LOG_AUTHPRIV | LOG_CRON, __VA_ARGS__)
//     #define LOGD(...) syslog(LOG_NOTICE | LOG_AUTHPRIV | LOG_CRON, "%s LIB VERSION %s ","DEBUG ",LIBRARY_VERSION); syslog(LOG_NOTICE | LOG_AUTHPRIV | LOG_CRON, __VA_ARGS__)
// #endif

#define LOGV(...)
#define LOGD(...)

#define LOGI(...)
#define LOGW(...)
#define LOGE(...)

// #define LOGI(...) printf("INFO "); printf(__VA_ARGS__); printf("\n"); syslog(LOG_INFO | LOG_AUTHPRIV | LOG_CRON, "%s LIB VERSION %s ","INFO ",LIBRARY_VERSION); syslog(LOG_INFO | LOG_AUTHPRIV | LOG_CRON, __VA_ARGS__)
// #define LOGW(...) printf("WARN "); printf(__VA_ARGS__); printf("\n"); syslog(LOG_WARNING | LOG_AUTHPRIV | LOG_CRON, "%s LIB VERSION %s ","WARN ",LIBRARY_VERSION); syslog(LOG_WARNING | LOG_AUTHPRIV | LOG_CRON, __VA_ARGS__)
// #define LOGE(...) printf("ERROR "); printf(__VA_ARGS__); printf("\n"); syslog(LOG_ERR | LOG_AUTHPRIV | LOG_CRON, "%s LIB VERSION %s ","ERROR ",LIBRARY_VERSION); syslog(LOG_ERR | LOG_AUTHPRIV | LOG_CRON, __VA_ARGS__)

#endif

/* command status enumeration */
typedef enum
{
    PORT_CLOSED,
    PORT_OPEN,
    PORT_ERROR,
    SSP_REPLY_OK,
    SSP_PACKET_ERROR,
    SSP_CMD_TIMEOUT,
} PORT_STATUS;

typedef enum
{
    SMILEY_PROTOCOL,
    JCM_PROTOCOL,
} PROTOCOLS_IMPLEMENTED;

class KbiredUtilities
{
public:
    KbiredUtilities() { }

    static char* string_format_kbired(const char* string, ...);

    static void logd(const char* str, ...);

private:

};

#endif	/* KBIREDUTILITIES_H */


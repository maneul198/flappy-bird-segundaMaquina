/*
 * File:   KbiredUtilities.cpp
 * Author: sebastian
 *
 * Created on April 2, 2012, 4:04 PM
 */

#include <logs/KbiredUtilities.h>

char* KbiredUtilities::string_format_kbired(const char* lpOutputString, ...)
{
    va_list argptr;

    va_start(argptr, lpOutputString);

    char* OutMsg = new char[150];
    char* format = new char[150];

    for (int i = 0, j = 0; lpOutputString[i] != '\0'; i++)
    {
        format[j++] = lpOutputString[i];
        // If escape character
        if (lpOutputString[i] == '\\')
        {
            i++;
            continue;
        }
        // if not a substitutal character
        if (lpOutputString[i] != '%')
            continue;

        format[j++] = lpOutputString[++i];
        format[j] = '\0';
        switch (lpOutputString[i])
        {
            // string
        case 's':
        {
            char* s = va_arg(argptr, char *);
            sprintf(OutMsg, format, s);
            strcpy(format, OutMsg);
            j = strlen(format);
            strcat(format, " ");
            break;
        }
        // character
        case 'c':
        {
            char c = (char) va_arg(argptr, int);
            sprintf(OutMsg, format, c);
            strcpy(format, OutMsg);
            j = strlen(format);
            strcat(format, " ");
            break;
        }
        // integer
        case 'd':
        case 'i':
        {
            int d = va_arg(argptr, int);
            sprintf(OutMsg, format, d);
            strcpy(format, OutMsg);
            j = strlen(format);
            strcat(format, " ");
            break;
        }
        }
    }
    va_end(argptr);
    return OutMsg;
}

void KbiredUtilities::logd(const char* str, ...)
{
    openlog("Cabitech", LOG_PID|LOG_CONS, LOG_LOCAL0);

    va_list args;
    va_start(args, str);

    vsyslog(LOG_INFO, str, args);

    closelog();
}

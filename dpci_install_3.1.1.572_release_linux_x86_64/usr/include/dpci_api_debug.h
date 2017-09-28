/******************************************************************************
 *
 * $Id: dpci_api_debug.h 12910 2016-11-24 13:15:11Z james $
 *
 * Copyright 2003-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * DPCI debug support header file.
 *
 *****************************************************************************/

#ifndef _DPCI_API_DEBUG_H_
#define _DPCI_API_DEBUG_H_

#include "dpci_api_decl.h"
#include "dpci_api_types.h" // required for DPCI_DEBUG_.* macros

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
# ifdef _DEBUG
#  define DPCI_API_DEBUG
#  if _MSC_VER >= 1600 
#   define DPCI_DEBUG_FUNCRET(level, var, text, ...)	\
	do { \
		if (var == -1) \
			dpci_debug_error_sysmsg(level, "%s: " text "= %d", __FUNCTION__, __VA_ARGS__,(var)); \
		else \
			dpci_debug_info(level, "%s: " text "= %d\n", __FUNCTION__, __VA_ARGS__,(var)); \
	} while (0)
#  endif
# else
#  if _MSC_VER >= 1600 
#   define DPCI_DEBUG_FUNCRET(level, var, text, ...) do { } while(0)
#  endif
# endif
# define INLINE __forceinline 
#else
# ifdef DEBUG
#  define DPCI_API_DEBUG
#  define DPCI_DEBUG_FUNCRET(level, var, text, ...)	\
	do { \
		if (var == -1) \
			dpci_debug_error_sysmsg(level, "%s: " text "= %d", __func__, ##__VA_ARGS__,(var)); \
		else \
			dpci_debug_info(level, "%s: " text "= %d\n", __func__, ##__VA_ARGS__,(var)); \
	} while (0)
# else
#  define DPCI_DEBUG_FUNCRET(level, var, text, ...) do { } while(0)
# endif

/*
 * GCC info/manual pages say "extern __inline__" to ensure the function is
 * never compiled alone.  __inline__ is suggested for ISO C99 compatibility.
 */
# define INLINE extern __inline__
#endif

#include <stdarg.h>

#define STRING_OR_NULL(str)	((str) ? (str) : "[NULL]")

#ifdef DPCI_API_DEBUG
DPCI_API int APIENTRY dpci_debug_error_sysmsg(unsigned int level, const char *error_msg_format, ...);
DPCI_API int APIENTRY dpci_debug_error(unsigned int level, const char *error_msg_format, ...);
DPCI_API int APIENTRY dpci_debug_trace(unsigned int level, const char *format, ...);
DPCI_API int APIENTRY dpci_debug_warning(unsigned int level, const char *format, ...);
DPCI_API int APIENTRY dpci_debug_info(unsigned int level, const char *format, ...);
DPCI_API int APIENTRY dpci_debug(unsigned int level, const char *format, ...);
DPCI_API int APIENTRY dpci_vdebug(unsigned int level, const char *format, va_list ap);
#else
INLINE int dpci_debug_error_sysmsg(unsigned int level, const char *error_msg_format, ...)
{
	return 0;
}

INLINE int dpci_debug_error(unsigned int level, const char *error_msg_format, ...)
{
	return 0;
}

INLINE int dpci_debug_trace(unsigned int level, const char *format, ...)
{
	return 0;
}

INLINE int dpci_debug_warning(unsigned int level, const char *format, ...)
{
	return 0;
}

INLINE int dpci_debug_info(unsigned int level, const char *format, ...)
{
	return 0;
}

INLINE int dpci_debug(unsigned int level, const char *format, ...)
{
	return 0;
}

INLINE int dpci_vdebug(unsigned int level, const char *format, va_list ap)
{
	return 0;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DPCI_API_DEBUG_H_ */

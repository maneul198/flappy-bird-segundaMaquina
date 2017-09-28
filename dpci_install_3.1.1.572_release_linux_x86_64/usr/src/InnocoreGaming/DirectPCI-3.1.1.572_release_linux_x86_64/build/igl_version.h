/******************************************************************************
 *
 * $Id: igl_version.h 12464 2016-05-07 22:20:43Z aidan $
 *
 * Copyright 2008-2016 Advantech Co Ltd..
 * All rights reserved.
 *
 * Description:
 * Version macros for all IGL products
 *
 *****************************************************************************/

#ifndef _IGL_VERSION_H
#define _IGL_VERSION_H

#ifndef _VERSION_MAJOR
# error "_VERSION_MAJOR not set in project's version header file"
#endif
#ifndef _VERSION_MINOR
# error "_VERSION_MINOR not set in project's version header file"
#endif
#ifndef _VERSION_MICRO
# error "_VERSION_MICRO not set in project's version header file"
#endif

#ifdef WIN32
# include "autobuild.h"
#endif

#ifdef AUTO_BUILD_NO
# ifndef _VERSION_BUILD_STR
#  error "_VERSION_BUILD_STR not set in project's version header file"
# endif
# undef _USERNAME
#else
# undef _VERSION_BUILD_STR
# ifndef _USERNAME
#  define _USERNAME "_user"
# endif
# define _VERSION_BUILD_STR "_" __IGL_STRING(_USERNAME)
#endif

#if !defined (_VERSION_OS)
# ifdef WIN64
#  define _VERSION_OS	"win7"
# elif defined WIN32
#  define _VERSION_OS	"win32"
# elif defined(__linux)
#  define _VERSION_OS	"linux"
# else
#  error "Operating system not supported"
# endif
#endif

#if !defined(_VERSION_ARCH)
# ifdef WIN64
#  define _VERSION_ARCH x64
# elif defined WIN32
#  define _VERSION_ARCH i386
# else
#  error "CPU Architecture not supported"
# endif
#endif

#undef _VERSION_BUILD
#include "build.h"

#ifdef DEBUG
# define _DEBUG_INFIX "(DEBUG)"
#else
# define _DEBUG_INFIX ""
#endif

#define IGL_GET_VERSION_MAJOR(ver)	 (((ver) & 0xf0000000) >> 28)
#define IGL_GET_VERSION_MINOR(ver)	 (((ver) & 0x0ff00000) >> 20)
#define IGL_GET_VERSION_MICRO(ver)	 (((ver) & 0x000ff000) >> 12)
#define IGL_GET_VERSION_BUILD(ver)	 (((ver) & 0x00000fff))

#define _IGL_MAKE_VERSION(major, minor, micro, build) \
			(((major) << 28) | \
			((minor) << 20) | \
			((micro) << 12) | \
			(build))

#define __IGL_XSTRING(str)  #str
#define __IGL_STRING(str)  __IGL_XSTRING(str)

#define B__DATE__	__IGL_STRING(_B__DATE__)
#define B__TIME__	__IGL_STRING(_B__TIME__)

#define _VERSION_NUM	__IGL_STRING(_VERSION_MAJOR) "." \
				__IGL_STRING(_VERSION_MINOR) "." \
                               	__IGL_STRING(_VERSION_MICRO)
#define __VERSION_STRING	__IGL_STRING(_VERSION_MAJOR) "." \
				__IGL_STRING(_VERSION_MINOR) "." \
                               	__IGL_STRING(_VERSION_MICRO) "." \
				__IGL_STRING(_VERSION_BUILD)

#ifdef _VERSION_SPECIAL
#define _VERSION_STRING		__VERSION_STRING \
				__IGL_STRING(_VERSION_SPECIAL) 
#else
#define _VERSION_STRING		__VERSION_STRING
#endif

#if !defined(_VERSION_OS_ARCH)
# define VERSION_STRING	_VERSION_STRING	\
				_DEBUG_INFIX \
				_VERSION_BUILD_STR "_" \
				_VERSION_OS "_" \
				__IGL_STRING(_VERSION_ARCH)
#else
# define VERSION_STRING	_VERSION_STRING	\
				_DEBUG_INFIX \
				_VERSION_BUILD_STR "_" \
				__IGL_STRING(_VERSION_OS_ARCH)
#endif

#define	VERSION_CODE	_IGL_MAKE_VERSION(_VERSION_MAJOR, _VERSION_MINOR, _VERSION_MICRO, _VERSION_BUILD)

/*
 * Macros for Windows(tm) Resource Compiler files (.rc files).
 */
#define	COMPANY_NAME		"Advantech Innocore"
#define	COPYRIGHT_HOLDER	"Advantech Corporation Ltd."
#ifndef _PRODUCT_NAME
# error "_PRODUCT_NAME not set"
#endif

#define	_VERSION_COMMA	_VERSION_MAJOR,_VERSION_MINOR,_VERSION_MICRO
#define	RC_FILE_VERSION	_VERSION_COMMA,_VERSION_BUILD
#define	RC_PROD_VERSION	_VERSION_COMMA,_VERSION_BUILD
#define	RC_FileVersion	__IGL_STRING(_VERSION_MAJOR) "," \
				__IGL_STRING(_VERSION_MINOR) "," \
				__IGL_STRING(_VERSION_MICRO) "," \
				__IGL_STRING(_VERSION_BUILD)
#define	RC_ProductVersion VERSION_STRING
#define	RC_ProductName		COMPANY_NAME " " _PRODUCT_NAME
#define	RC_Copyright		"Copyright 2003-2016 " COPYRIGHT_HOLDER
#define	RC_Trademarks		COMPANY_NAME " Direct-PCI ConnectBus"
#define RC_BUILD_NAME		VERSION_STRING

#endif /* _IGL_VERSION_H */

/******************************************************************************
 *
 * $Id: dpci_api_decl.h 7141 2011-01-31 09:40:14Z emil $
 *
 * Copyright 2011-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * DPCI API header for common Windows/Linux declarations
 *
 *****************************************************************************/

#ifndef _DPCI_API_DECL_H_
#define _DPCI_API_DECL_H_

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef __GNUC__
	#define DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER) && (_MSC_VER >= 1300) //VC6 does not support deprecated
	#define DEPRECATED __declspec(deprecated)
#else
	//#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
	#define DEPRECATED
#endif

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DPCI_API_DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DPCI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WIN32
	#ifdef DPCI_API_DLL_EXPORTS
		#define DPCI_API __declspec(dllexport)
		#if (_MSC_VER >= 1300) //VC6 does not support deprecated
			#define DPCI_DEPRECATED_API __declspec(deprecated dllexport)
		#else
			#define DPCI_DEPRECATED_API DPCI_API
		#endif
	#elif DPCI_API_DLL_IMPORTS
		#define DPCI_API __declspec(dllimport)
		#if (_MSC_VER >= 1300) //VC6 does not support deprecated
			#define DPCI_DEPRECATED_API __declspec(deprecated dllimport)
		#else
			#define DPCI_DEPRECATED_API DPCI_API
		#endif
	#else
		#define DPCI_API 
		#define DPCI_DEPRECATED_API
	#endif
#else //Linux
	#define DPCI_API
	#undef APIENTRY
	#define APIENTRY
	#define DPCI_DEPRECATED_API DEPRECATED
#endif

#ifndef UNUSED_PARAMETER
# define UNUSED_PARAMETER(x) (x)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DPCI_API_DECL_H_ */


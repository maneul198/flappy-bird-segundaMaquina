/******************************************************************************
 *
 * $Id: dpci_error.h 11488 2015-01-31 23:25:03Z aidan $
 *
 * Copyright 2008-2015 Advantech Co. Ltd.
 * All rights reserved.
 *
 * Description:
 * DPCI Error API header file.
 *
 *****************************************************************************/

#ifndef _DPCI_ERROR_H_
#define _DPCI_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dpci_api_decl.h"

/*
 * Useful for cross-platform development...
 */
DPCI_API int			APIENTRY dpci_last_os_error_code(void);
DPCI_API const char *	APIENTRY dpci_last_os_error_string(void);
DPCI_API const char *	APIENTRY dpci_os_error_string(int);

#ifdef __cplusplus
}
#endif

#endif /* _DPCI_ERROR_H_ */





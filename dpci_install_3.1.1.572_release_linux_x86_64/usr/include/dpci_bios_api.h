/******************************************************************************
 *
 * $Id: dpci_bios_api.h 11488 2015-01-31 23:25:03Z aidan $
 *
 * Copyright 2011-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * DPCI BIOS API header file.
 *
 *****************************************************************************/

#ifndef _DPCI_BIOS_API_H_
#define _DPCI_BIOS_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dpci_api_decl.h"

/*
 * API to read system BIOS memory via dpci_core driver
 */
DPCI_API long	APIENTRY dpci_bios_dump(unsigned char *buff, 
					unsigned long* buff_size);
DPCI_API int	APIENTRY dpci_bios_size(unsigned long* buff_size);

#ifdef __cplusplus
}
#endif

#endif /* _DPCI_BIOS_API_H_ */

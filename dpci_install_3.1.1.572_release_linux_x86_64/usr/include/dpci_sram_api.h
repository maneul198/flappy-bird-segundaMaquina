/******************************************************************************
 *
 * $Id: dpci_sram_api.h 11488 2015-01-31 23:25:03Z aidan $
 *
 * Copyright 2003-2015 Advantech Co. Ltd.
 * All rights reserved.
 *
 * Description:
 * DPCI API header file for SRAM access.
 *
 *****************************************************************************/

#ifndef _DPCI_SRAM_API_H_
#define _DPCI_SRAM_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dpci_api_decl.h"
#include "dpci_error.h"

#ifdef __linux

/*
 * Device to open for mmap(2) and ioctls.
 */
#define	DPCI_SRAM_DEVICE	"/dev/sram0"

#endif

/*
 * Supported SRAM access calls.
 */
DPCI_API int	APIENTRY dpci_sram_read8	(unsigned int offset,
											 unsigned char *valuep);
DPCI_API int	APIENTRY dpci_sram_read16	(unsigned int offset,
											 unsigned short *valuep);
DPCI_API int	APIENTRY dpci_sram_read32	(unsigned int offset,
											 unsigned int *valuep);
DPCI_API int	APIENTRY dpci_sram_read		(unsigned int offset, 
											 void *buffer, 
											 unsigned int datalen);
DPCI_API int	APIENTRY dpci_sram_write8	(unsigned int offset,
											 unsigned char value);
DPCI_API int	APIENTRY dpci_sram_write16	(unsigned int offset,
											 unsigned short value);
DPCI_API int	APIENTRY dpci_sram_write32	(unsigned int offset,
											 unsigned int value);
DPCI_API int	APIENTRY dpci_sram_write	(unsigned int offset,
											 void *buffer, 
											 unsigned int datalen);
DPCI_API int	APIENTRY dpci_sram_size		(void);
DPCI_API void*	APIENTRY dpci_sram_map		(void);
DPCI_API int	APIENTRY dpci_sram_unmap	(void);

#ifdef __cplusplus
}
#endif

#endif /* _DPCI_SRAM_API_H_ */

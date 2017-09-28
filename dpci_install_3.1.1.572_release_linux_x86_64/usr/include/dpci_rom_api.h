/******************************************************************************
 *
 * $Id: dpci_rom_api.h 11488 2015-01-31 23:25:03Z aidan $
 *
 * Copyright 2003-2015 Advantech Co. Ltd.
 * All rights reserved.
 *
 * Description:
 * DPCI API header for ROM access.
 *
 *****************************************************************************/

#ifndef _DPCI_ROM_API_H_
#define _DPCI_ROM_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dpci_api_decl.h"
#include "dpci_error.h"

#ifdef __linux
/*
 * Device to open for mmap(2) and ioctls.
 */
#define	DPCI_ROM_DEVICE	"/dev/rom0"

#endif

/*
 * Supported ROM access calls.
 */
DPCI_API int	APIENTRY dpci_rom_read8	(unsigned int offset,
										 unsigned char *valuep);
DPCI_API int	APIENTRY dpci_rom_read16(unsigned int offset,
										 unsigned short *valuep);
DPCI_API int	APIENTRY dpci_rom_read32(unsigned int offset,
										 unsigned int *valuep);
DPCI_API int	APIENTRY dpci_rom_read	(unsigned int offset,
										 void *buffer,
										 unsigned int datalen);
DPCI_API int	APIENTRY dpci_rom_size	(void);
DPCI_API void*	APIENTRY dpci_rom_map	(void);
DPCI_API int	APIENTRY dpci_rom_unmap	(void);

#ifdef __cplusplus
}
#endif

#endif /* _DPCI_ROM_API_H */


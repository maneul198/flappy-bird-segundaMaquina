/******************************************************************************
 *
 * $Id: dpci_mem_ioctl.h 11905 2015-09-07 15:28:17Z aidan $
 *
 * Copyright 2003-2015 Advantech Co. Ltd.
 * All rights reserved.
 *
 * License:     GPL v2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Support: Advantech Innocore customers should send e-mail to this address:
 *
 *      support@advantech-innocore.com
 *
 * Users' own modifications to this driver are allowed but not supportable.
 *
 * Description:
 * DirectPCI header file for kernel-mode memory (SRAM/ROM) driver.
 *
 *****************************************************************************/

#ifndef _DPCI_MEM_H
#define _DPCI_MEM_H

#ifdef __cplusplus
extern "C"{
#endif

#include <linux/ioctl.h>
#include <linux/types.h>
#include <dpci_version.h>

#define DENSITRON_VENDOR_ID 		0x16cd
#define SRAM_DEVICE_ID		    	0x0101
#define SRAM16_DEVICE_ID	    	0x0102
#define ROM_DEVICE_ID		    	0x0103
#define ROM2_DEVICE_ID		    	0x0104

#define MEM_IOC_MAGIC			's'

/*
 * These are the current, supported SRAM I/O control commands.
 */
#define IOCTL_MEM_READ_BYTE	_IOWR(MEM_IOC_MAGIC, 1, struct mem_memloc)	
#define IOCTL_MEM_READ_WORD	_IOWR(MEM_IOC_MAGIC, 2, struct mem_memloc)	
#define IOCTL_MEM_READ_DWORD	_IOWR(MEM_IOC_MAGIC, 3, struct mem_memloc)	
#define IOCTL_MEM_WRITE_BYTE	_IOW(MEM_IOC_MAGIC, 4, struct mem_memloc)
#define IOCTL_MEM_WRITE_WORD	_IOW(MEM_IOC_MAGIC, 5, struct mem_memloc)
#define IOCTL_MEM_WRITE_DWORD	_IOW(MEM_IOC_MAGIC, 6, struct mem_memloc)
#define IOCTL_MEM_SIZE		_IOR(MEM_IOC_MAGIC, 7, int)
#define IOCTL_MEM_SET_DEBUG	_IOW(MEM_IOC_MAGIC, 8, int)
#define IOCTL_MEM_GET_DEBUG	_IOW(MEM_IOC_MAGIC, 9, int)

#define IOCTL_SRAM_ILLEGAL_CMD	10

#define MEM_MODULE_NAME			"dpci_mem"
#define MEM_DEVICE_NAME			"dpci_mem"
#define MEM_DEVICE_COUNT		2


struct mem_memloc 
{
   unsigned int offset;
   unsigned int value;
};

#ifdef __cplusplus
}
#endif

#endif /*end _DPCI_MEM_H*/

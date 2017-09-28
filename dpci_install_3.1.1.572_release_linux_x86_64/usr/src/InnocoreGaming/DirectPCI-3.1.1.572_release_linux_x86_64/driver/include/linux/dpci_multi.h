/******************************************************************************
 *
 * $Id: dpci_multi.h 11932 2015-09-14 11:25:47Z aidan $
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
 * Users' own modifications to this driver are not supported.
 *
 * Description:
 * Innocore DirectPCI core module for kernel-mode
 *
 *****************************************************************************/
#ifndef _DPCI_MULTI_H
#define _DPCI_MULTI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dpci_api_types.h"

/******************************************************************************/
/*
 * These macros define the default major, minor numbers and the total number of
 * DPCI devices
 */
#ifdef __KERNEL__
/*
 * These macro defines the size of the DCPI IO regions (see /proc/ioports)
 */
#define DPCI_IO_REGION_SIZE	0x0100
extern struct dpci_device *dpci_get_device(int device_no);
extern u8 dpci_ioboard_supported(struct dpci_device *ddp);
extern u8 dpci_get_ioboard_id(struct dpci_device *ddp);
extern u8 dpci_get_ioboard_rev(struct dpci_device *ddp);
extern u8 dpci_get_ioboard_intcfg(struct dpci_device *ddp);
extern u8 dpci_set_ioboard_intcfg(struct dpci_device *ddp, u8 set, u8 clear);
extern void dpci_enable_ioboard_intr(struct dpci_device *ddp);
extern void dpci_disable_ioboard_intr(struct dpci_device *ddp);
extern int dpci_get_uart_offsets(struct dpci_device *ddp, dpci_off_t *buf, int size);
extern int dpci_get_ioaddress(struct dpci_device *ddp);
extern u8 dpci_get_irq(struct dpci_device *ddp);
extern u8 dpci_get_reg_cached(struct dpci_device *ddp, dpci_off_t reg);
extern u8 dpci_get_reg(struct dpci_device *ddp, dpci_off_t reg);
extern u8 dpci_chg_reg(struct dpci_device *ddp, dpci_off_t reg, u8 set, u8 clear);
extern u8 dpci_set_reg(struct dpci_device *ddp, dpci_off_t reg, u8 val);
extern u8 dpci_io_readbyte(struct dpci_device *ddp,dpci_off_t offset);
extern u8 dpci_io_writebyte(struct dpci_device *ddp,dpci_off_t offset, u8 byte);
extern u16 dpci_io_readword(struct dpci_device *ddp,dpci_off_t offset);
extern u16 dpci_io_writeword(struct dpci_device *ddp,dpci_off_t offset, u16 word);
extern u32 dpci_io_readdword(struct dpci_device *ddp,dpci_off_t offset);
extern u32 dpci_io_writedword(struct dpci_device *ddp,dpci_off_t offset, u32 dword);
typedef irqreturn_t (*exint_handler_t)(struct dpci_device *ddp, void *private);
extern int dpci_set_exint_handler(struct dpci_device *ddp, exint_handler_t handler, void *private);
extern int dpci_clear_exint_handler(struct dpci_device *ddp, exint_handler_t handler, void *private);

extern int dpci_get_board_features(struct dpci_device *ddp, unsigned int *features_p);
#ifndef HAVE_1ROMSOCKET
# define HAVE_1ROMSOCKET	0x00100000
#endif
extern int dpci_get_nvram_config(struct dpci_device *ddp, resource_size_t *addr, resource_size_t *len);
extern int dpci_get_rom_config(struct dpci_device *ddp, resource_size_t *addr, resource_size_t *len);

#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif /*end __cplusplus*/
#endif /*end _DPCI_MULTI_H*/

/******************************************************************************
 *
 * $Id: dpci_core_ioctl.h 11959 2015-09-23 13:43:52Z aidan $
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
#ifndef _DPCI_CORE_IOCTL_H
#define _DPCI_CORE_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#include "dpci_api_types.h"
#include <linux/ioctl.h>
#include <linux/types.h>

/******************************************************************************/
/*
 * These macros define the commands that can be processed by ioctl fucntion
 * Those that begin with an underscore '_' are deprecated but are listed so
 * the numbers are not re-used.
 */
#define DPCI_IOC_MAGIC	'd'
#define OW_IOC_MAGIC	'W'

#define _IOCTL_DPCI_INT_MODIFY		_IOW(DPCI_IOC_MAGIC, 1, int)
#define IOCTL_DPCI_WD_ENABLE		_IOW(DPCI_IOC_MAGIC, 2, int)
#define IOCTL_DPCI_WD_RESET		_IO(DPCI_IOC_MAGIC, 3)
#define IOCTL_DPCI_ID_READEVENT	        _IOR(DPCI_IOC_MAGIC, 4, int)
#define IOCTL_DPCI_ID_NUMEVENTS		_IOR(DPCI_IOC_MAGIC, 5, struct idevent)
#define IOCTL_DPCI_ID_SETDATE           _IOW(DPCI_IOC_MAGIC, 6, struct idevent)
#define IOCTL_DPCI_ID_GETDATE		_IOR(DPCI_IOC_MAGIC, 7, struct idevent)
#define _IOCTL_DPCI_INT_ENABLE		_IO(DPCI_IOC_MAGIC, 8)
#define _IOCTL_DPCI_INT_DISABLE		_IO(DPCI_IOC_MAGIC, 9)
#define IOCTL_DPCI_ID_FWVERSION		_IOR(DPCI_IOC_MAGIC,10, int)
#define IOCTL_DPCI_IO_WRITE8		_IOW(DPCI_IOC_MAGIC,11, struct memloc)
#define IOCTL_DPCI_IO_WRITE16		_IOW(DPCI_IOC_MAGIC,12, struct memloc)
#define IOCTL_DPCI_IO_WRITE32		_IOW(DPCI_IOC_MAGIC,13, struct memloc)
#define IOCTL_DPCI_BAT_READ		_IOR(DPCI_IOC_MAGIC,17, int)
#define IOCTL_DPCI_DIS_READ		_IOR(DPCI_IOC_MAGIC,18, int)
#define IOCTL_DPCI_IO_READ8		_IOWR(DPCI_IOC_MAGIC,19, struct memloc)
#define IOCTL_DPCI_IO_READ16		_IOWR(DPCI_IOC_MAGIC,20, struct memloc)
#define IOCTL_DPCI_IO_READ32		_IOWR(DPCI_IOC_MAGIC,21, struct memloc)
#define _IOCTL_DPCI_IO_ISROMEXT		_IO(DPCI_IOC_MAGIC, 22)		/* not support in DPCI version 2.0 onwards*/
#define IOCTL_DPCI_IO_GETBOARDID	_IO(DPCI_IOC_MAGIC, 23)
#define IOCTL_DPCI_IO_GETBOARDREV	_IO(DPCI_IOC_MAGIC, 24)
#define _IOCTL_DPCI_E2_READ8		_IOWR(DPCI_IOC_MAGIC,25, struct e2loc)
#define	_IOCTL_DPCI_E2_WRITE8		_IOWR(DPCI_IOC_MAGIC,26, struct e2loc)
#define	__IOCTL_DPCI_IO_WAITINT		_IOWR(DPCI_IOC_MAGIC,27, struct waitint0)
#define	IOCTL_DPCI_ID_SETSLEEPTIME	_IOW(DPCI_IOC_MAGIC,28, int)
#define	IOCTL_DPCI_GET_VERSION		_IOR(DPCI_IOC_MAGIC,29, struct dpci_version)
#define	IOCTL_DPCI_ID_INTRUSIONSTATUS	_IO(DPCI_IOC_MAGIC, 30)
#define	__IOCTL_DPCI_ID_WAITEVENT	_IOW(DPCI_IOC_MAGIC, 30, int) /* bad */
#define	_IOCTL_DPCI_ID_WAITEVENT	_IOW(DPCI_IOC_MAGIC, 31, int)
#define	_IOCTL_DPCI_IO_WAITINT		_IOWR(DPCI_IOC_MAGIC,32, struct waitint)
#define IOCTL_DPCI_INT_MODIFY		_IOW(DPCI_IOC_MAGIC, 33, struct intcfg)
#define IOCTL_DPCI_IO_READP8		_IOWR(DPCI_IOC_MAGIC,34, struct memloc)
#define IOCTL_DPCI_IO_WRITEP8		_IOWR(DPCI_IOC_MAGIC,35, struct memloc)
#define IOCTL_DPCI_I2C_COMMAND		_IOWR(DPCI_IOC_MAGIC,36, struct dpci_i2c_cmdbuf)
#define _IOCTL_DPCI_E2_READBLK		_IOWR(DPCI_IOC_MAGIC,37, struct e2block)
#define	_IOCTL_DPCI_E2_WRITEBLK		_IOWR(DPCI_IOC_MAGIC,38, struct e2block)
#define	IOCTL_DPCI_DEBUG		_IOW(DPCI_IOC_MAGIC, 39, int)
#define	IOCTL_DPCI_I2C_GETNAME		_IOWR(DPCI_IOC_MAGIC, 40, struct getname)
#define IOCTL_DPCI_IO_NUMOPORTS		_IO(DPCI_IOC_MAGIC,41)
#define IOCTL_DPCI_IO_NUMIPORTS		_IOW(DPCI_IOC_MAGIC,42, int)
#define	IOCTL_DPCI_TS_WAITINTR		_IOW(DPCI_IOC_MAGIC,43, unsigned long)
#define	IOCTL_DPCI_TS_SIGNAL		_IOW(DPCI_IOC_MAGIC,44, int)
#define	IOCTL_DPCI_I2C_NUMBUSES		_IO(DPCI_IOC_MAGIC, 45)
#define	IOCTL_DPCI_CTR_GETNAME		_IOWR(DPCI_IOC_MAGIC, 46, struct dpci_ctrname)
#define	IOCTL_DPCI_CTR_GETVALUES	_IOWR(DPCI_IOC_MAGIC, 47, struct dpci_ctrvalues)
#define	IOCTL_DPCI_CTR_RESET		_IO(DPCI_IOC_MAGIC, 48)
#define	IOCTL_DPCI_ID_GET_SELECTED_EVENT _IOR(DPCI_IOC_MAGIC, 49, struct idevent)
#define	IOCTL_DPCI_ID_GETPROMBYTE	_IOW(DPCI_IOC_MAGIC, 50, int)
#define	IOCTL_DPCI_BAT_NUMBATTERIES	_IO(DPCI_IOC_MAGIC, 51)
#define	IOCTL_DPCI_BAT_GETNAME		_IOW(DPCI_IOC_MAGIC, 52, struct getname)
#define	IOCTL_DPCI_BAT_GETLEVEL		_IOWR(DPCI_IOC_MAGIC, 53, int)
#define	IOCTL_DPCI_BAT_SETERRORLEVEL	_IOW(DPCI_IOC_MAGIC, 54, struct battlvl)
#define	IOCTL_DPCI_BAT_SETCHECKPERIOD	_IOW(DPCI_IOC_MAGIC, 55, int)
#define	IOCTL_DPCI_BAT_GETSTATUS	_IOW(DPCI_IOC_MAGIC, 56, int)
#define	IOCTL_DPCI_DEBUG_DUMP_CACHED	_IO(DPCI_IOC_MAGIC, 57)
#define	IOCTL_DPCI_ID_DEBUG_CMD		_IOWR(DPCI_IOC_MAGIC, 58, struct dpci_idlp_cmdbuf)
#define IOCTL_DPCI_IO_CHANGEP8		_IOWR(DPCI_IOC_MAGIC, 59, struct params_change_port)
#define IOCTL_DPCI_IO_READ_OUTP8	_IOWR(DPCI_IOC_MAGIC, 60, struct memloc)
#define IOCTL_DPCI_IO_BOARD_SUPPORTED	_IO(DPCI_IOC_MAGIC, 61)
#define IOCTL_DPCI_IO_WAITPFD		_IOW(DPCI_IOC_MAGIC, 62, unsigned long)
#define	IOCTL_DPCI_GET_DEBUG		_IO(DPCI_IOC_MAGIC, 63)
#define	IOCTL_DPCI_BAT_GET_ERRORLEVEL	_IOWR(DPCI_IOC_MAGIC, 64, int)
#define IOCTL_DPCI_QMODE_GET		_IOR(DPCI_IOC_MAGIC, 65, struct quiet_mode)
#define IOCTL_DPCI_QMODE_SET		_IOWR(DPCI_IOC_MAGIC, 66, struct quiet_mode)
#define IOCTL_DPCI_IOWD_ENABLE		_IO(DPCI_IOC_MAGIC, 67)
#define IOCTL_DPCI_IOWD_DISABLE		_IO(DPCI_IOC_MAGIC, 68)
#define IOCTL_DPCI_IOWD_PAT		_IO(DPCI_IOC_MAGIC, 69)
#define IOCTL_DPCI_ID_SETDATE_PSW       _IOW(DPCI_IOC_MAGIC, 70, struct idevent_psw)
#define IOCTL_DPCI_ID_READEVENT_PSW	_IOWR(DPCI_IOC_MAGIC, 71, struct idevent_psw)
#define _IOCTL_DPCI_ID_WAITEVENT_PSW	_IOW(DPCI_IOC_MAGIC, 72, struct idwait_psw)
#define IOCTL_DPCI_ID_FWVERSIONFULL	_IOR(DPCI_IOC_MAGIC,73, int)
#define IOCTL_DPCI_WAIT_ALLEVENTS	_IOW(DPCI_IOC_MAGIC, 74, struct dpci_event_timeout)
#define IOCTL_DPCI_ID_WAITEVENT		_IOWR(DPCI_IOC_MAGIC, 75, struct idevent_timeout)
#define IOCTL_DPCI_WAIT_EVENTS		_IOWR(DPCI_IOC_MAGIC, 76, struct event_mask_timeout_t)
#define	IOCTL_DPCI_IO_WAITINT		_IOWR(DPCI_IOC_MAGIC,77, struct waitint_port)
#define IOCTL_DPCI_ID_WAITEVENT_PSW	_IOWR(DPCI_IOC_MAGIC, 78, struct idevent_timeout_psw)
#define	IOCTL_DPCI_IO_WAITINT32		_IOWR(DPCI_IOC_MAGIC,79, struct waitint_32)
#define IOCTL_DPCI_GPIO_READ_IP		_IOWR(DPCI_IOC_MAGIC, 80, struct memloc)
#define IOCTL_DPCI_GPIO_WRITE_OP	_IOWR(DPCI_IOC_MAGIC, 81, struct memloc)
#define IOCTL_DPCI_GPIO_READ_OP		_IOWR(DPCI_IOC_MAGIC, 82, struct memloc)
#define IOCTL_DPCI_BIOS_DUMP		_IOWR(DPCI_IOC_MAGIC, 83, struct bios_mem_param)
#define	IOCTL_DPCI_DEBUG_DUMP_EVENTWAIT	_IO(DPCI_IOC_MAGIC, 84)
#define	IOCTL_DPCI_CLOSE_EVENTS		_IO(DPCI_IOC_MAGIC, 85)
#define	IOCTL_DPCI_E130_ENABLE_BPS	_IOW(DPCI_IOC_MAGIC, 86, int)
#define	IOCTL_DPCI_E130_BPS_IS_ENABLED	_IO(DPCI_IOC_MAGIC, 87)

#define IOCTL_DPCI_ID_READEVENT_INSTANCE	_IOWR(DPCI_IOC_MAGIC, 88, struct idevent)
#define IOCTL_DPCI_ID_READCHECKSUM	        _IOR(DPCI_IOC_MAGIC, 89, int)
#define IOCTL_DPCI_HW_GETPROFILE	_IOR(DPCI_IOC_MAGIC, 90, struct dpci_hardware_profile)

/*
 * One-wire (OW) port ioctl commands.  write/read-byte are left here for
 * reference but are no longer implemented and were never used in user-land.
 */
#define	IOCTL_OW_LL_TOUCHRESET		_IOW(OW_IOC_MAGIC, 0, int)
#define	IOCTL_OW_LL_TOUCHBIT		_IOW(OW_IOC_MAGIC, 1, struct owdata)
#define	IOCTL_OW_LL_TOUCHBYTE		_IOW(OW_IOC_MAGIC, 2, struct owdata)
#define	_IOCTL_OW_LL_WRITEBYTE		_IOW(OW_IOC_MAGIC, 3, struct owdata)
#define	_IOCTL_OW_LL_READBYTE		_IOW(OW_IOC_MAGIC, 4, int)
#define	IOCTL_OW_LL_SPEED		_IOW(OW_IOC_MAGIC, 5, struct owdata)
#define	IOCTL_OW_LL_LINELEVEL		_IOW(OW_IOC_MAGIC, 6, struct owdata)
#define	IOCTL_OW_LL_PROGRAMPULSE	_IOW(OW_IOC_MAGIC, 7, int)
#define	IOCTL_OW_LL_HASPOWER		_IOW(OW_IOC_MAGIC, 8, int)
#define	IOCTL_OW_LL_HASOVERDRIVE	_IOW(OW_IOC_MAGIC, 9, int)
#define	IOCTL_OW_LL_HASPROGPULSE	_IOW(OW_IOC_MAGIC, 10, int)
#define	IOCTL_OW_LL_WRITEBYTEPOWER	_IOW(OW_IOC_MAGIC, 11, struct owdata)
#define	IOCTL_OW_LL_READBITPOWER	_IOW(OW_IOC_MAGIC, 12, struct owdata)
#define	IOCTL_OW_LL_GETNAME		_IOWR(OW_IOC_MAGIC, 13, struct getname)
#define	IOCTL_OW_SE_ACQUIRE		_IOW(OW_IOC_MAGIC, 14, int)
#define	IOCTL_OW_SE_RELEASE		_IOW(OW_IOC_MAGIC, 15, int)
#define	IOCTL_OW_SE_GETUSER		_IOW(OW_IOC_MAGIC, 16, int)
#define	IOCTL_OW_TR_BLOCK		_IOW(OW_IOC_MAGIC, 17, struct owblock)


/*
 * This is used for counters.
 */
struct dpci_ctrname
{
	int dc_ctr;
	char *dc_buf;
	int dc_buflen;
};

struct dpci_ctrvalues
{
	unsigned long *dc_buf;
	int dc_maxctrs;
	int dc_do_reset;
};

/*
 * Used for idlp wait-event command.
 */
struct idevent_timeout {
   struct idevent id_event;
   unsigned long timeout;
};

/*
 * Used for all dpci wait-event command.
 */

struct dpci_event_timeout {
   struct dpci_event event;
   unsigned long timeout;
};

struct event_mask_timeout_t {
	struct event_mask_t event_mask;
	struct dpci_event op_event;
	unsigned long timeout;
};

/*
 * Used for set-time commands when password is required
 */
struct idevent_psw {
   struct idevent id_event;
   char psw[256];
};

struct idevent_timeout_psw {
	struct idevent_timeout idevent_tm;
	char psw[256];
};

struct idwait_psw {
   unsigned long timeout;
   char psw[256];
};

#define SHA1_LEN	20

/*
 * Used for writing/reading i/o ports.  Value field is always a 32-bit unsigned
 * int - extra MSBs ignored accoring to size of access.
 */
struct memloc {
	dpci_off_t offset;
	unsigned int value;
};

/*
 * Used for changing bits on output port
 */
struct params_change_port {
	dpci_off_t offset;
	unsigned char set;
	unsigned char clear;
	unsigned char toggle;
};

/*
 * Used for writing/reading EEPROMS - 8/16/32 bits wide.
 */
#define	MFG_EEPROM_SIZE		256 /* bytes */
struct e2loc {
	unsigned int offset;
	unsigned int value;
};

/*
 * Used for handling block read/write operations to EEPROMs.
 */
struct e2block
{
	unsigned int	offset;		/* offset in target space */
	void		*addr;		/* user address of data */
	unsigned int	count;		/* count of bytes */
};

/*
 * Used for reading bios memory
 */
struct bios_mem_param
{
	unsigned long len;
	unsigned long offset;
};

/*
 * Used for interrupt configuration - this is the old port 0 only version
 */
struct intcfg0 {
	unsigned short int_mask;
	unsigned short pol_mask;
};

/*
 * Used for interrupt configuration - this is the newer all-ports version
 */
struct intcfg {
	int port;
   	unsigned char int_mask;
   	unsigned char pol_mask;
};

/*
 * Used for interrupt waiting - this is the old port 0 only version
 */
struct waitint0 {
	unsigned char int_mask;
	unsigned char edge_state;
	unsigned long timeout_ms;
};

/*
 * Used for interrupt waiting - this is the old port 0 only version
 */
struct waitint {
	int port;
	unsigned char int_mask;
	unsigned char edge_state;
	unsigned long timeout_ms;
};

/*
 * Used for interrupt waiting
 */
struct waitint_port {
	int port;
	unsigned char int_mask;
	unsigned char edge_state;
	unsigned char autoconfig_edge;
	unsigned long timeout_ms;
	struct dio_port_event op_data;
};

struct waitint_32 {
	unsigned long int_mask;
	unsigned long edge_state;
	unsigned long autoconfig_edge;
	unsigned long timeout;
	struct dio_event op_data;
};

/*
 * Used for getting names of various system ports, objects etc.
 */
#define	GETNAME_MAX_BUF	256
struct getname {
	unsigned int gn_portnum;
	char gn_buf[GETNAME_MAX_BUF];
};

/*
 * Used for one-wire (OW) port commands.
 */
struct owdata {
	int od_portnum;
	int od_data;
};

struct owblock {
	int ob_portnum;
	int ob_do_reset;
	int ob_datalen;
	char *ob_buffer;
};

#ifdef __cplusplus
}
#endif /*end __cplusplus*/
#endif /*end _DPCI_CORE_IOCTL_H  */

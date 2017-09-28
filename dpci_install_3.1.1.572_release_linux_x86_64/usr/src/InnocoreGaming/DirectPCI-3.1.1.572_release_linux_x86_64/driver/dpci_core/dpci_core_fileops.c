/******************************************************************************
 *
 * $Id: dpci_core_fileops.c 12233 2015-12-14 14:49:51Z aidan $
 *
 * Copyright 2003-2015 Advantech Co Limited.
 * All rights reserved.
 *
 * License:	GPL v2
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
 * DirectPCI core module for kernel-mode
 *
 *****************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/input.h>
#include <linux/cdev.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

/*include the file that defines the constants for the dpci-core driver.*/

#include <linux/dpci_core_ioctl.h>
#include <linux/dpci_multi.h>
#include "dpci_version.h"
#include "dpci_core_hw.h"
#include "dpci_core_types.h"
#include "dpci_core_priv.h"
#include "dpci_core_priv2.h"
#include "dpci_boards.h"

/*
 * DirectPCI counters.
 */
extern unsigned int all_interrupts;
extern unsigned int unassigned_interrupts;
extern unsigned int localio_interrupts;
extern unsigned int idlp_event_interrupts;
extern unsigned int idlp_ack_interrupts;
extern unsigned int exio_interrupts;
extern unsigned int digio_interrupts;
extern unsigned int ts_interrupts;
extern unsigned int pfd_interrupts;

struct dpci_counter dpci_counters[] =
{
	{&all_interrupts, "Number of all received interrupts"},
	{&unassigned_interrupts, "Number of unassigned interrupts"},
	{&localio_interrupts, "Number of on-board IO interrupts"},
	{&idlp_event_interrupts, "Number of IDLP interrupts"},
	{&idlp_ack_interrupts, "Number of IDLP data acknowledge interrupts"},
	{&exio_interrupts, "Number of external interrupts"},
	{&digio_interrupts, "Number of digital IO interrupts"},
	{&ts_interrupts, "Number of 80-0062 temperature sensor interrupts"},
	{&pfd_interrupts, "Number of power fail detection interrupts"},
	{NULL, NULL}
};


/*******************************************************************************
 *
 * Function:    dpci_dis_read()
 *
 * Parameters:  none
 *
 * Returns:     u8 - lower four bits have states of DIP switches.
 *
 * Description: Reads the DIP switches' states from REG1 and returns the data.
 *
 ******************************************************************************/
static u8 dpci_dis_read(struct dpci_device *ddp)
{
	u8 disstatus = 0;

	disstatus = dpci_io_readbyte(ddp, DPCI_REG1) & 0xF;
	return disstatus;
}


/*******************************************************************************
 *
 * Function:    dpci_bat_status()
 *
 * Parameters:  battno - battery number
 *
 * Returns:     >= 0 - battery status; <0 - error
 *
 * Description: Reads the status of the batteries present on the board.
 *
 ******************************************************************************/
static int dpci_bat_status(struct dpci_device *ddp, int battno)
{
	int status;

	if (battno < 0 ||
		battno >= ddp->mainboard->db_num_bat ||
		!ddp->mainboard->db_batteries[battno].dbat_desc ||
		disable_idlp)
	{
		return -ENXIO;
	}
	if (ddp->mainboard->db_batteries[battno].dbat_reg ==
		DPCI_BAT_REG_IDLP)
	{
		status = dpci_id_getbattstatus(ddp);
		if (status >= 0)
			status &= 1 << (ddp->mainboard->db_batteries[battno].dbat_idx - 1);
	}
	else
	{
		status = dpci_io_readbyte(ddp,
				ddp->mainboard->db_batteries[battno].dbat_reg) &
				ddp->mainboard->db_batteries[battno].dbat_idx;
		if (status > 0)
			status = 1 << battno;
	}
	return status;
}


/*******************************************************************************
 *
 * Function:    dpci_gpio_read_ip()
 *
 * Parameters:  none
 *
 * Returns:     <0 - error; >=0 - input state
 *
 * Description: Reads the input state of the gpio pins
 *
 ******************************************************************************/
int dpci_gpio_read_ip(struct dpci_device *ddp, struct memloc *mem_loc)
{
	switch (mem_loc->offset)
	{
		case 0:
			mem_loc->value = dpci_reg_get(ddp, DPCI_REG2) & 
					DPCI_REG2_GPIO0;
			break;
		case 1:
			mem_loc->value = dpci_reg_get(ddp, DPCI_REG2) & 
					DPCI_REG2_GPIO1;
			break;
		default:
			return -ENODEV;
	}
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_gpio_write_op()
 *
 * Parameters:  op_value - the output data to be written to the gpio pins.
 *
 * Returns:     <0 - error; 0 - success
 *
 * Description: Write data to gpio output pins.
 *
 ******************************************************************************/
int dpci_gpio_write_op(struct dpci_device *ddp, struct memloc mem_loc)
{	
	u8 set_clear;
	switch (mem_loc.offset)
	{
		case 0:	
			set_clear = DPCI_REG2_GPQ0;
			break;
		case 1:
			set_clear = DPCI_REG2_GPQ1;
			break;
		default:
			return -ENODEV;
	}
	if (mem_loc.value)
		dpci_reg_chg(ddp, DPCI_REG2, set_clear, 0, 0);
	else
		dpci_reg_chg(ddp, DPCI_REG2, 0, set_clear, 0);
	
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_gpio_read_op()
 *
 * Parameters:  none
 *
 * Returns:     <0 - error; >=0 - last output state
 *
 * Description: Reads the last output state of the gpio pins
 *
 ******************************************************************************/
int dpci_gpio_read_op(struct dpci_device *ddp, struct memloc *mem_loc)
{
	switch (mem_loc->offset)
	{
		case 0:
			mem_loc->value = dpci_reg_get(ddp, DPCI_REG2) & 
					DPCI_REG2_GPQ0;
			break;
		case 1:
			mem_loc->value = dpci_reg_get(ddp, DPCI_REG2) & 
					DPCI_REG2_GPQ1;
			break;
		default:
			return -ENODEV;
	}
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_bios_dump()
 *
 * Parameters:  b_buff - structure to hold BIOS memory read from the system
 *
 * Returns:     <0 - error; =0 -success
 *
 * Description: Reads BIOS memory from the system. BIOS size of defined by the 
 *		buff_size parameter of the b_buff structure.
 *
 ******************************************************************************/
int dpci_bios_dump(struct dpci_device *ddp, struct bios_mem_param *param)
{
	if (param->len < ddp->mainboard->bios_len)
	{
		param->len = ddp->mainboard->bios_len;
		return (-EINVAL);
	}

	param->offset = SYSTEM_MEMORY_END_ADDR - ddp->mainboard->bios_len;
	
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_ioctl()
 *
 * Parameters:  inode - the inode upon which the command is requested
 *		file - the file the user is using to access the inode
 *		cmd - the command being requested
 *		arg - additional data for the command.
 *
 * Returns:     status and or return data for the command.
 *
 * Description: Performs all i/o control operations specific to the DPCI device
 *		node.
 *
 ******************************************************************************/
#ifndef HAVE_UNLOCKED_IOCTL
static long dpci_ioctl(
			struct inode *inode,
#else
static long dpci_ioctl(
#endif
			struct file *file,
			unsigned int cmd,
			unsigned long arg)
{
	struct dpci_device *ddp = file->private_data;
	struct memloc memloc;
	struct params_change_port params_change_port;
	struct waitint waitint;
	struct waitint0 waitint0;
	struct waitint_port waitint_port;
	struct waitint_32 waitint_32;
	struct dpci_version dpci_version;
	struct intcfg intcfg;
	struct intcfg0 intcfg0;
	struct getname getname;
	struct battlvl battlvl;
	struct dpci_counter *dcp;
	struct dpci_ctrname ctrname;
	struct dpci_ctrvalues ctrvalues;
	struct dpci_event_timeout op_event_timeout;
	struct event_mask_t event_mask;
	struct event_mask_timeout_t event_mask_timeout;
	struct bios_mem_param b_param;
	int set, clear;
	int ret = 0;
	int status = 0;
	int counter;
	unsigned char newval;

	/*
	 * If the board I/O is disabled because of an IDLP problem then only
	 * allow a few calls and deny the rest.
	 */
	switch (cmd)
	{
	case IOCTL_DPCI_HW_GETPROFILE:
	case IOCTL_DPCI_IO_BOARD_SUPPORTED:
	case IOCTL_DPCI_IO_GETBOARDID:
	case IOCTL_DPCI_IO_GETBOARDREV:
	case IOCTL_DPCI_GET_VERSION:
	case IOCTL_DPCI_I2C_GETNAME:
	case IOCTL_DPCI_IO_NUMOPORTS:
	case IOCTL_DPCI_IO_NUMIPORTS:
	case IOCTL_DPCI_I2C_NUMBUSES:
	case IOCTL_DPCI_CTR_GETNAME:
	case IOCTL_DPCI_CTR_GETVALUES:
	case IOCTL_DPCI_CTR_RESET:
	case IOCTL_DPCI_BAT_NUMBATTERIES:
	case IOCTL_DPCI_BAT_GETNAME:
	case IOCTL_DPCI_GET_DEBUG:
	case IOCTL_DPCI_DEBUG:
	case IOCTL_DPCI_BIOS_DUMP:
	case IOCTL_DPCI_IO_READ8:
	case IOCTL_DPCI_IO_READ16:
	case IOCTL_DPCI_IO_READ32:
	case IOCTL_DPCI_IO_WRITE8:
	case IOCTL_DPCI_IO_WRITE16:
	case IOCTL_DPCI_IO_WRITE32:
	case IOCTL_OW_LL_GETNAME:
	case IOCTL_OW_SE_ACQUIRE:
	case IOCTL_OW_SE_RELEASE:
	case IOCTL_OW_SE_GETUSER:
		break;

	default: 
		if (ddp->disabled)
		{
			ret = -EACCES;
			goto done;
		}
	}

#if defined(USE_DGL_OW_API)
	/*
	 * Handle One-wire (OW/W1) commands separately.
	 */
	if (_IOC_TYPE(cmd) == OW_IOC_MAGIC)
	{
		return dpci_ow_ioctl(file, cmd, arg);
	}
#endif

	PRINT_IOC("(file=0x%p, cmd=0x%x, arg=0x%lx) pid=%d ddp=%p\n",
			file, cmd, arg, current->pid, ddp);

	if(!disable_idlp)
	{
		ret = dpci_id_ioctl(file, cmd, arg);
		if (ret != -EINVAL)
		{
			goto done;
		}
	}

	/*
	 * Clear ret which is now -EINVAL because of call to dpci_id_ioctl().
	 */
	ret = 0;
	switch(cmd)
	{
	case IOCTL_DPCI_GET_VERSION:
		dpci_version.version_code = DPCI_VERSION_CODE;
		strncpy(dpci_version.version_string,
			 DPCI_VERSION,
			 sizeof(dpci_version.version_string));
		dpci_version.hw_device_id = ddp->board_id->dpci_did;
		dpci_version.hw_device_rev = ddp->revision;
		ret = copy_to_user((void *)arg,
					&dpci_version,
					sizeof(dpci_version));
		break;

	case _IOCTL_DPCI_INT_MODIFY:
		if (copy_from_user(&intcfg0, (void *)arg, sizeof(intcfg0)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_io_modifyint(ddp, 0, intcfg0.int_mask,
					~intcfg0.int_mask,
					intcfg0.pol_mask);
		break;

	case IOCTL_DPCI_INT_MODIFY:
		/*
		 * This is the new version of INT_MODIFY which supports
		 * multiple ports.
		 */
		if (copy_from_user(&intcfg, (void *)arg, sizeof(intcfg)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_io_modifyint(ddp, intcfg.port, intcfg.int_mask,
					~intcfg.int_mask,
					intcfg.pol_mask);
		break;

	case IOCTL_DPCI_IO_NUMOPORTS:
		if (!ddp->reg_op)
		{
			ret = 0;
			break;
		}
		for (ret = 0; ret < MAX_OUTPUT_PORTS; ret++)
		{
			if (!ddp->reg_op[ret])
				break;
		}
		break;

	case IOCTL_DPCI_IO_NUMIPORTS:
		if (!ddp->reg_ip)
		{
			ret = 0;
			break;
		}
		for (ret = 0; ret < MAX_OUTPUT_PORTS; ret++)
		{
			if (arg)
			{
				if (!ddp->reg_ip_status[ret])
					break;
			}
			else
			{
				if (!ddp->reg_ip[ret])
					break;
			}
		}
		break;

	case IOCTL_DPCI_IO_WRITE8:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (!(debug & DEBUG_USERREGS))
		{
			ret = -EACCES;
			break;
		}
		dpci_reg_set(ddp, memloc.offset, memloc.value);
		ret = 0;
		break;

	case IOCTL_DPCI_IO_WRITE16:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (!(debug & DEBUG_USERREGS))
		{
			ret = -EACCES;
			break;
		}
		dpci_io_writeword(ddp, memloc.offset,memloc.value);
		ret = 0;
		break;

	case IOCTL_DPCI_IO_WRITE32:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (!(debug & DEBUG_USERREGS))
		{
			ret = -EACCES;
			break;
		}
		dpci_io_writedword(ddp, memloc.offset,memloc.value);
		ret = 0;
		break;

	case IOCTL_DPCI_IO_READ8:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (!(debug & DEBUG_USERREGS))
		{
			ret = -EACCES;
			break;
		}
		memloc.value = dpci_io_readbyte(ddp, memloc.offset);
		ret = copy_to_user((void *)arg, &memloc, sizeof(memloc));
		break;

	case IOCTL_DPCI_IO_READ16:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (!(debug & DEBUG_USERREGS))
		{
			ret = -EACCES;
			break;
		}
		memloc.value = dpci_io_readword(ddp, memloc.offset);
		ret = copy_to_user((void *)arg, &memloc, sizeof(memloc));
		break;

	case IOCTL_DPCI_IO_READ32:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (!(debug & DEBUG_USERREGS))
		{
			ret = -EACCES;
			break;
		}
		memloc.value = dpci_io_readdword(ddp, memloc.offset);
		ret = copy_to_user((void *)arg, &memloc, sizeof(memloc));
		break;

	case IOCTL_DPCI_IO_READP8:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if ((memloc.offset >= MAX_INPUT_PORTS) ||
			!ddp->reg_ip ||
			!ddp->reg_ip[memloc.offset])
		{
			ret = -ENODEV;
			break;
		}
		memloc.value = dpci_io_readbyte(ddp, ddp->reg_ip[memloc.offset]);
		PRINT_IO("IOCTL_DPCI_IO_READP8: port %d data = %02x\n",
			memloc.offset,
			memloc.value);
		ret = copy_to_user((void *)arg, &memloc, sizeof(memloc));
		break;

	case IOCTL_DPCI_IO_WRITEP8:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if ((memloc.offset >= MAX_OUTPUT_PORTS) ||
						!ddp->reg_op ||
						!ddp->reg_op[memloc.offset])
		{
			ret = -ENODEV;
			break;
		}
		/*
		 * Save the data written to the write-only register
		 */
		dpci_reg_set(ddp, ddp->reg_op[memloc.offset], memloc.value);
		PRINT_IO("IOCTL_DPCI_IO_WRITEP8: port %d data = %02x\n",
			memloc.offset,
			memloc.value);
		ret = 0;
		break;

	case IOCTL_DPCI_IO_CHANGEP8:
		if (copy_from_user(&params_change_port, (void *)arg, sizeof(params_change_port)))
		{
			ret = -EFAULT;
			break;
		}
		if ((params_change_port.offset >= MAX_OUTPUT_PORTS) ||
						!ddp->reg_op ||
						!ddp->reg_op[params_change_port.offset])
		{
			ret = -ENODEV;
			break;
		}

		newval = dpci_reg_chg(ddp, 
				      ddp->reg_op[params_change_port.offset], 
				      params_change_port.set, 
				      params_change_port.clear, 
				      params_change_port.toggle);
		PRINT_IO("IOCTL_DPCI_IO_CHANGEP8: port %d data = %02x\n",
			params_change_port.offset,
			newval);
		ret = 0;
		break;

	case IOCTL_DPCI_IO_READ_OUTP8:
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if ((memloc.offset >= MAX_OUTPUT_PORTS) ||
			!ddp->reg_op ||
			!ddp->reg_op[memloc.offset])
		{
			ret = -ENODEV;
			break;
		}
		memloc.value = ddp->val_regs[ddp->reg_op[memloc.offset]];
		PRINT_IO("IOCTL_DPCI_IO_READ_OUTP8: port %d data = %02x\n",
			memloc.offset,
			memloc.value);
		ret = copy_to_user((void *)arg, &memloc, sizeof(memloc));
		break;

	case IOCTL_DPCI_DIS_READ:
		if (ddp->features & HAVE_NODIPSW)
		{
			ret = -EINVAL;
			break;
		}
		ret = dpci_dis_read(ddp);
		break;

	case IOCTL_DPCI_QMODE_GET:
		if (ddp->features & HAVE_QUIETMODE)
		{
			struct quiet_mode qmode_status;

			ret = dpci_io_readbyte(ddp, DPCI_REG1);
			qmode_status.passthrough = (ret & DPCI_REG1_QMPT); 
			qmode_status.level = (ret & DPCI_REG1_QMLV);
			if (copy_to_user((void*)arg, &qmode_status, sizeof(qmode_status)))
			{
				ret = -EFAULT;
				break;
			}
			ret = 0;			
		}
		else
			ret = -EINVAL;
		break;

	case IOCTL_DPCI_QMODE_SET:
		if (ddp->features & HAVE_QUIETMODE)
		{
			struct quiet_mode qmode_status;

			if (copy_from_user(&qmode_status, (void *)arg, sizeof(qmode_status)))
			{
				ret = -EFAULT;
				break;
			}
			set = 0;
			clear = 0;
			if (qmode_status.passthrough == 0)
			{
				if (!(ddp->features & HAVE_QMODE_PASSTHRU))
				{
					ret = -EINVAL;
					break;
				}
				clear |= DPCI_REG1_QMPT;
			}
			else 
				set |= DPCI_REG1_QMPT;
			if (qmode_status.level == 0)
				clear |= DPCI_REG1_QMLV;
			else 
				set |= DPCI_REG1_QMLV;
			dpci_reg_chg(ddp, DPCI_REG1, set, clear, 0);
			ret = 0;
			/*
			 * Check whether we are on a board with CPLD f/w 
			 * version 0x80 and if the quiet mode status to set is 
			 * "OFF Set"
			 */
			if ((ddp->revision == 0x80) &&
				(qmode_status.passthrough == 1) && 
				(qmode_status.level == 1))
			{
				/* 
				 * Workaround for bug 95 - OE# bit in register 
				 * 0x1F does not clear on setting quiet mode 
				 * off.
				 */
				dpci_set_ioboard_intcfg(ddp, 
						0, 
						DPCI_IOBOARD_INTCFG_OE);
			}
		}
		else
			ret = -EINVAL;
		break;

	case IOCTL_DPCI_IO_BOARD_SUPPORTED:
		ret = dpci_ioboard_supported(ddp);
		break;

	case IOCTL_DPCI_IO_GETBOARDID:
		ret = dpci_get_ioboard_id(ddp);
		break;

	case IOCTL_DPCI_IO_GETBOARDREV:
		ret = dpci_get_ioboard_rev(ddp);
		break;

	case __IOCTL_DPCI_IO_WAITINT:
		if (copy_from_user(&waitint0, (void *)arg, sizeof(waitint0)))
		{
			ret = -EFAULT;
			break;
		}
		waitint_port.port = 0;
		waitint_port.int_mask = waitint0.int_mask;
		waitint_port.edge_state = waitint0.edge_state;
		waitint_port.autoconfig_edge = 0;
		waitint_port.timeout_ms = waitint0.timeout_ms;		
		
		ret = dpci_io_wait_int(ddp, &waitint_port, 0);
		if (ret > 0)
		{
			ret = waitint_port.op_data.change_mask;
		}
		break;

	case _IOCTL_DPCI_IO_WAITINT:
		if (copy_from_user(&waitint, (void *)arg, sizeof(waitint)))
		{
			ret = -EFAULT;
			break;
		}
		waitint_port.port = waitint.port;
		waitint_port.int_mask = waitint.int_mask;
		waitint_port.edge_state = waitint.edge_state;
		waitint_port.autoconfig_edge = 0;
		waitint_port.timeout_ms = waitint.timeout_ms;		
		ret = dpci_io_wait_int(ddp, &waitint_port, 0);
		if (ret > 0)
		{
			ret = waitint_port.op_data.change_mask;
		}
		break;

	case IOCTL_DPCI_IO_WAITINT:
		if (copy_from_user(&waitint_port, (void *)arg, sizeof(waitint_port)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_io_wait_int(ddp, &waitint_port, 1);
		if (ret == 1)
		{
			if (copy_to_user((void *)arg, 
					&waitint_port, 
					sizeof(waitint_port)))
			{
				ret = -EFAULT;
				break;
			}
		}
		break;

	case IOCTL_DPCI_IO_WAITINT32:
		if (copy_from_user(&waitint_32, (void *)arg, sizeof(waitint_32)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_io_wait_int32(ddp, &waitint_32);
		if (ret == 1)
		{
			if (copy_to_user((void *)arg, 
					&waitint_32, 
					sizeof(waitint_32)))
			{
				ret = -EFAULT;
				break;
			}
		}
		break;

	case IOCTL_DPCI_I2C_GETNAME:
		if (copy_from_user(&getname, (void *)arg, sizeof(getname)))
		{
			ret = -EFAULT;
			goto done;
		}
		if (getname.gn_portnum >= MAX_I2C_PORTS ||
			!ddp->i2c_ports[getname.gn_portnum].dip_desc)
		{
			ret = -ENXIO;
			goto done;
		}
		strncpy(getname.gn_buf,
			 ddp->i2c_ports[getname.gn_portnum].dip_desc,
			 sizeof(getname.gn_buf));
		if (copy_to_user((void *)arg, &getname, sizeof(getname)))
		{
			ret = -EFAULT;
			goto done;
		}
		ret = 0;
		break;

	case IOCTL_DPCI_I2C_NUMBUSES:
		for (ret = 0; ret < MAX_I2C_PORTS; ret++)
			if (!ddp->i2c_ports[ret].dip_desc)
				break;
		break;

	case IOCTL_DPCI_I2C_COMMAND:
		ret = user_i2c_command(ddp, arg);
		break;

	case IOCTL_DPCI_DEBUG:
		{
			int old_debug = debug;

			debug = arg;
			PRINT_INFO("debug level 0x%x -> 0x%x\n", old_debug, debug);
#ifndef DEBUG
			if (debug)
			{
				PRINT_WARN("debug=0x%X but this is not a DEBUG driver: "
						"please load the debug driver if you want debug output.\n", debug);
			}
#endif
			ret = 0;
		}
		break;

	case IOCTL_DPCI_GET_DEBUG:
		{
			PRINT_INFO("debug level = 0x%x\n",debug);
			ret = debug;
		}
		break;

	case IOCTL_DPCI_WAIT_ALLEVENTS:
		if (copy_from_user(&op_event_timeout, 
				(void *)arg, 
				sizeof(op_event_timeout)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_wait_allevents(ddp, &op_event_timeout);
		if (copy_to_user((void *)arg, 
				&op_event_timeout, 
				sizeof(op_event_timeout)))
		{
			ret = -EFAULT;
			goto done;
		}
		
		break;

	case IOCTL_DPCI_WAIT_EVENTS:
		if (copy_from_user(&event_mask_timeout, 
				(void *)arg, 
				sizeof(event_mask_timeout)))
		{
			ret = -EFAULT;
			break;
		}
		event_mask = event_mask_timeout.event_mask;
		
		ret = dpci_wait_events(ddp, 
				event_mask.er_type, 
				event_mask_timeout.timeout,
				1,
				&event_mask_timeout.op_event, 
				event_mask.int_mask, 
				event_mask.edge_state,
				event_mask.autoconfig_edge);
		if (copy_to_user((void *)arg, 
				&event_mask_timeout, 
				sizeof(event_mask_timeout)))
		{
			ret = -EFAULT;
			goto done;
		}
		
		break;

	case IOCTL_DPCI_IO_WAITPFD:
		ret = dpci_io_wait_pfd(ddp, arg);
		break;

	case IOCTL_DPCI_TS_WAITINTR:
		ret = dpci_ts_wait_alarm(ddp, arg);
		break;

	case IOCTL_DPCI_TS_SIGNAL:
		/*
		 * See if there's an I/O board II present.
		 */
		if ((!ddp->ioboard) ||
			(ddp->ioboard->db_board_id != DPCI_IOBOARD_ID_80_0062A))
		{
			ret = -ENXIO;
			break;
		}

		/*
		 * See if someone's already waiting for a signal.
		 */
		if (ddp->ts_pid)
		{
			PRINT_IO("IOCTL_DPCI_TS_SIGNAL: already in use by pid %d\n",
						ddp->ts_pid);
			ret = -EBUSY;
			goto done;
		}
		ddp->ts_pid = current->pid;
		ddp->ts_signal = arg;
		dpci_set_ioboard_intcfg(ddp, DPCI_IOBOARD62_INTCFG_TS, 0);
		break;

	case IOCTL_DPCI_CTR_GETNAME:
		if (copy_from_user(&ctrname, (void *)arg, sizeof(ctrname)))
		{
			ret = -EFAULT;
			goto done;
		}
		if (ctrname.dc_ctr >= -1 + sizeof(dpci_counters) / sizeof(struct dpci_counter))
		{
			ret = -ENXIO;
			goto done;
		}
		strncpy(ctrname.dc_buf,
			 dpci_counters[ctrname.dc_ctr].dc_desc,
			 ctrname.dc_buflen - 1);
		if (copy_to_user((void *)arg, &ctrname, sizeof(ctrname)))
		{
			ret = -EFAULT;
			goto done;
		}
		break;

	case IOCTL_DPCI_CTR_GETVALUES:
		if (copy_from_user(&ctrvalues, (void *)arg, sizeof(ctrvalues)))
		{
			PRINT_IOC("IOCTL_DPCI_CTR_GETNAME fault getting struct\n");
			ret = -EFAULT;
			goto done;
		}
		for (dcp = dpci_counters;
			 ctrvalues.dc_maxctrs-- && dcp->dc_counter;
			 dcp++)
		{
			if (copy_to_user((void *)ctrvalues.dc_buf++,
					 (void *)dcp->dc_counter,
					 sizeof(*dcp->dc_counter)))
			{
				ret = -EFAULT;
				goto done;
			}
		}
		if (!ctrvalues.dc_do_reset)
		 	break;

		/* DELIBERATE FALL THROUGH TO RESET-COUNTERS CODE */

	case IOCTL_DPCI_CTR_RESET:
		{
			unsigned long flags;

			local_irq_save(flags);
			local_irq_disable();
			for (dcp = dpci_counters; dcp->dc_counter; dcp++)
			{
				*(dcp->dc_counter) = 0;
			}
			local_irq_restore(flags);
		}
		break;

	case IOCTL_DPCI_BAT_READ:
		for (status = 0, counter = 0; counter < ddp->mainboard->db_num_bat; counter++)
		{
			if (!ddp->mainboard->db_batteries[counter].dbat_desc)
				break;
			ret = dpci_bat_status(ddp, counter);
			if (ret > 0)
				status |= 1 << counter;
		}
		ret = status;
		break;

	case IOCTL_DPCI_BAT_NUMBATTERIES:
		for (ret = 0; ret < ddp->mainboard->db_num_bat; ret++)
			if (!ddp->mainboard->db_batteries[ret].dbat_desc)
				break;
		break;

	case IOCTL_DPCI_BAT_GETNAME:
		if (copy_from_user(&getname, (void *)arg, sizeof(getname)))
		{
			ret = -EFAULT;
			goto done;
		}
		if (getname.gn_portnum >= ddp->mainboard->db_num_bat ||
			!ddp->mainboard->db_batteries[getname.gn_portnum].dbat_desc)
		{
			ret = -ENXIO;
			goto done;
		}
		strncpy(getname.gn_buf,
			 ddp->mainboard->db_batteries[getname.gn_portnum].dbat_desc,
			 sizeof(getname.gn_buf));
		if (copy_to_user((void *)arg, &getname, sizeof(getname)))
		{
			ret = -EFAULT;
			goto done;
		}
		ret = 0;
		break;

	case IOCTL_DPCI_BAT_GETLEVEL:
		if (arg < 0 ||
			arg >= ddp->mainboard->db_num_bat ||
			!ddp->mainboard->db_batteries[arg].dbat_desc ||
			disable_idlp)
		{
			ret = -ENXIO;
			goto done;
		}
		if (ddp->mainboard->db_batteries[arg].dbat_cal == 0)
		{
			ret = -EINVAL;
			goto done;
		}
		ret = dpci_id_getbattlevel(ddp, arg);
		if (ret >= 0)
		{
			ret *= ddp->mainboard->db_batteries[arg].dbat_cal;
			ret /= 10;
		}
		break;

	case IOCTL_DPCI_BAT_GET_ERRORLEVEL:
		if (arg < 0 ||
			arg >= ddp->mainboard->db_num_bat ||
			!ddp->mainboard->db_batteries[arg].dbat_desc ||
			disable_idlp)
		{
			ret = -ENXIO;
			goto done;
		}
		if (ddp->mainboard->db_batteries[arg].dbat_cal == 0)
		{
			ret = -EINVAL;
			goto done;
		}
		ret = dpci_id_getbatt_errorlevel(ddp, arg);
		if (ret >= 0)
		{
			ret *= ddp->mainboard->db_batteries[arg].dbat_cal;
			ret /= 10;
		}
		break;

	case IOCTL_DPCI_BAT_SETERRORLEVEL:
		if (copy_from_user(&battlvl, (void *)arg, sizeof(battlvl)))
		{
			ret = -EFAULT;
			goto done;
		}
		if (battlvl.battno < 0 ||
			battlvl.battno >= ddp->mainboard->db_num_bat ||
			!ddp->mainboard->db_batteries[battlvl.battno].dbat_desc ||
			disable_idlp)
		{
			ret = -ENXIO;
			goto done;
		}
		if (ddp->mainboard->db_batteries[battlvl.battno].dbat_cal == 0)
		{
			PRINT_IOC("no calibration!\n");
			ret = -EINVAL;
			goto done;
		}
		if (ddp->mainboard->db_batteries[battlvl.battno].dbat_reg !=
							DPCI_BAT_REG_IDLP)
		{
			PRINT_IOC("not on IDLP!\n");
			ret = -EINVAL;
			goto done;
		}
		PRINT_IOC("battlvl.battno=%d\n", battlvl.battno);
		PRINT_IOC("battlvl.millivolts=%d\n", battlvl.millivolts);

		/* The firmware accepts error voltage level above MIN_ERROR_VOLTAGE_LEVEL
		 * so we change any value below that to the MIN_ERROR_VOLTAGE_LEVEL
		 */
		if(battlvl.millivolts <= MIN_ERROR_VOLTAGE_LEVEL)
			battlvl.millivolts = MIN_ERROR_VOLTAGE_LEVEL;

		battlvl.millivolts *= 10;
		battlvl.millivolts /= ddp->mainboard->db_batteries[battlvl.battno].dbat_cal;
		ret = dpci_id_setbatterrorlevel(ddp,
						battlvl.battno,
						battlvl.millivolts);
		break;

	case IOCTL_DPCI_BAT_SETCHECKPERIOD:
		if(disable_idlp)
		{
			ret = -ENXIO;
			goto done;
		}

		for (ret = 0; ret < ddp->mainboard->db_num_bat; ret++)
			if (ddp->mainboard->db_batteries[ret].dbat_reg == DPCI_BAT_REG_IDLP)
				break;
		if (ret == ddp->mainboard->db_num_bat)
		{
			ret = -EINVAL;
			goto done;
		}
		ret = dpci_id_setbattcheckperiod(ddp, arg);
		break;

	case IOCTL_DPCI_BAT_GETSTATUS:
		ret = dpci_bat_status(ddp, arg);
		break;
	
	case IOCTL_DPCI_IOWD_ENABLE:
		ret = dpci_iowd_enable(ddp);
		break;

	case IOCTL_DPCI_IOWD_DISABLE:
		ret = dpci_iowd_disable(ddp);
		break;

	case IOCTL_DPCI_IOWD_PAT:
		ret = dpci_iowd_pat(ddp);
		break;
	
	case IOCTL_DPCI_GPIO_READ_IP:
		if (ddp->features & HAVE_NOGPIO)
		{
			ret = -EINVAL;
			break;
		}
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (memloc.offset >= MAX_GPIO_LINES)
		{
			ret = -ENODEV;
			break;
		}
		ret = dpci_gpio_read_ip(ddp, &memloc);
		if (ret == 0)
			ret = copy_to_user((void *)arg, 
					&memloc, sizeof(memloc));
		break;
	
	case IOCTL_DPCI_GPIO_WRITE_OP:
		if (ddp->features & HAVE_NOGPIO)
		{
			ret = -EINVAL;
			break;
		}
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (memloc.offset >= MAX_GPIO_LINES)
		{
			ret = -ENODEV;
			break;
		}
		ret = dpci_gpio_write_op(ddp, memloc);
		break;
	
	case IOCTL_DPCI_GPIO_READ_OP:
		if (ddp->features & HAVE_NOGPIO)
		{
			ret = -EINVAL;
			break;
		}
		if (copy_from_user(&memloc, (void *)arg, sizeof(memloc)))
		{
			ret = -EFAULT;
			break;
		}
		if (memloc.offset >= MAX_GPIO_LINES)
		{
			ret = -ENODEV;
			break;
		}
		ret = dpci_gpio_read_op(ddp, &memloc);
		if (ret == 0)
			ret = copy_to_user((void *)arg, 
					&memloc, sizeof(memloc));
		break;

	case IOCTL_DPCI_BIOS_DUMP:
		if (copy_from_user(&b_param, (void *)arg, sizeof(b_param)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_bios_dump(ddp, &b_param);
		if (copy_to_user((void *)arg, &b_param, sizeof(b_param)))
		{
			ret = -EFAULT;
			break;
		}
		
		break;
	
	case IOCTL_DPCI_DEBUG_DUMP_CACHED:
		PRINT_INFO("IOCTL_DPCI_DEBUG_DUMP_CACHED:\n");
		for (status = 0; status < GENIO_CUSTOMER_START; status += 8)
		{
			PRINT_INFO("%02x: %02x %02x %02x %02x %02x %02x %02x %02x\n",
					status,
					ddp->val_regs[status + 0],
					ddp->val_regs[status + 1],
					ddp->val_regs[status + 2],
					ddp->val_regs[status + 3],
					ddp->val_regs[status + 4],
					ddp->val_regs[status + 5],
					ddp->val_regs[status + 6],
					ddp->val_regs[status + 7]);
		}
		break;

	case IOCTL_DPCI_DEBUG_DUMP_EVENTWAIT:
		dpci_event_queue_dump(ddp);
		break;

	case IOCTL_DPCI_CLOSE_EVENTS:
		dpci_event_release_current_request(ddp, (pid_t) arg);
		break;

	case IOCTL_DPCI_E130_ENABLE_BPS:
		/*
		 * Check it's a DPX-E130.
		 */
		if (ddp->mainboard->db_board_id != DPX_E130)
		{
			ret = -EINVAL;
			break;
		}

		/*
		 * We need to use db_op_regs[0] + 2 because db_op_regs[2] is
		 * intentionally NULL so that users do not use the not-connected
		 * ports deliberately.
		 */
		if (arg)
		{
			dpci_reg_chg(ddp,
					ddp->mainboard->db_op_regs[0] + 2,
					1, 0, 0);
		}
		else
		{
			dpci_reg_chg(ddp,
					ddp->mainboard->db_op_regs[0] + 2,
					0, 1, 0);
		}
		break;

	case IOCTL_DPCI_E130_BPS_IS_ENABLED:
		/*
		 * Check it's a DPX-E130.
		 */
		if (ddp->mainboard->db_board_id != DPX_E130)
		{
			ret = -EINVAL;
			break;
		}
		ret = dpci_reg_get_saved(ddp, ddp->mainboard->db_op_regs[0] + 2);
		ret &= 1;
		break;

	case IOCTL_DPCI_HW_GETPROFILE:
		{
			if (copy_to_user((void *)arg, &(ddp->dhwp), sizeof(ddp->dhwp)))
			{
				ret = -EFAULT;
				break;
			}
			ret = 0;
		}
		break;

	default:
		PRINT_IOC("cmd 0x%x is invalid.\n", cmd);
		ret = -EINVAL;
		break;
	}

done:
	PRINT_IOC("(file=0x%p, cmd=0x%x, arg=0x%lx) = %d\n",
		file, cmd, arg, ret);

	return ret;
}


#ifdef HAVE_COMPAT_IOCTL
/*******************************************************************************
 *
 * Function:    dpci_compatioctl()
 *
 * Parameters:  file - the file the user is using to access the inode
 *		cmd - the command being requested
 *		arg - additioinal data for the command.
 *
 * Returns:     status and or return data for the command.
 *
 * Description: Performs all i/o control operations specific to the DPCI device
 *		node.  This is for 32-bit compatbility only and allows binaries
 *		compiled with 32-bit libdpci.so to work with 64-bit driver.
 *		We fudge the inode parameter to NULL; we can do this safely (I
 *		think) because none of our ioctl handlers yet use this.
 *
 ******************************************************************************/
static long dpci_compatioctl(struct file *file,
			unsigned int cmd,
			unsigned long arg)
{
#ifndef HAVE_UNLOCKED_IOCTL
	return dpci_ioctl(NULL, file, cmd, arg);
#else
	return dpci_ioctl(file, cmd, arg);
#endif
}
#endif


/*******************************************************************************
 *
 * Function:    dpci_open()
 *
 * Parameters:  inode - the inode upon which the open is requested
 *		file - the file the user is using to access the inode
 *
 * Returns:	0 - always
 *
 * Description: handles an open() request for the DPCI device node.  Nothing
 *		need be done here for the time being.  We DON'T enable input0
 *		interrupts here - we do it once somebody does a waitint or an
 *              input device open.
 *
 ******************************************************************************/
static int dpci_open(struct inode *inode, struct file *file)
{
	struct dpci_device *ddp;

	ddp = container_of(inode->i_cdev, struct dpci_device, cdev);
	PRINT_IOC("(inode=0x%p, file=0x%p) pid=%d ddp=%p\n", inode, file, current->pid, ddp);
	file->private_data = (void *)ddp;
	dpci_enable_digio_intr(ddp);
	ddp->flag_date_psw_active = 0;
	ddp->flag_event_psw_active = 0;
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_release()
 *
 * Parameters:  inode - the inode upon which the release is requested
 *		file - the file the user is using to access the inode
 *
 * Returns:	0 - always
 *
 * Description: handles a close() request for the DPCI device node.  We disable
 *		interrupts on the I/O board inputs if the input event device is
 *              not open too.
 *
 ******************************************************************************/
static int dpci_release(struct inode *inode, struct file *file)
{
	struct dpci_device *ddp = file->private_data;

	/*
	 * Ask that digital I/O interrupts are shutdown.  PIC interrupts are
	 * handled as needed and I/O board interrupts are not handled directly.
	 */
	PRINT_IOC("(inode=0x%p, file=0x%p) pid=%d ddp=%p\n", inode, file, current->pid, ddp);
	dpci_disable_digio_intr(ddp);
	dpci_ow_relinquish(ddp);
	dpci_event_release_requests(ddp);
	return 0;
}


struct file_operations dpci_fops =
{
	.owner	= THIS_MODULE,
#ifndef HAVE_UNLOCKED_IOCTL
	.ioctl	= dpci_ioctl,
#else
	.unlocked_ioctl	= dpci_ioctl,
#endif
#ifdef HAVE_COMPAT_IOCTL
	.compat_ioctl	= dpci_compatioctl,
#endif
	.open	= dpci_open,
	.release= dpci_release,
};

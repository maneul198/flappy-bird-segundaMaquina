/******************************************************************************
 *
 * $Id: dpci_core_ow.c 11905 2015-09-07 15:28:17Z aidan $
 *
 * Copyright 2007-2015 Advantech Co. Ltd.
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
 * Innocore DirectPCI core module for kernel-mode
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
#include "dpci_core_hw.h"

#include "dpci_core_priv.h"

#if defined(USE_DGL_OW_API)
/*******************************************************************************
 *
 * Function:    dpci_ow_get_port()
 *
 * Parameters:  p - the port number whose details are required.
 *
 * Returns:	struct dpci_ow_port * - the pointer to the port or NULL.
 *
 * Description: converts a logical port number to the actual port information
 *		structure for that port.
 *
 ******************************************************************************/
struct dpci_ow_port *dpci_ow_get_port(struct dpci_device *ddp, unsigned int p)
{
	if (p >= MAX_OW_PORTS)
		return NULL;
	if (!ddp->ow_ports[p].dowp_desc)
		return NULL;
	return &ddp->ow_ports[p];
}


static u8 dpci_ow_xfer_byte(struct dpci_ow_port *dowp, u8 obyte)
{
	int bit;
	u8 ibyte = 0;

	for (bit = 0x1; bit <=0x80; bit <<= 1)
	{
		if (dowp->dowp_touchbit(dowp, obyte & bit))
			ibyte |= bit;
	}
	PRINT_OW("wrote %02x got %02x\n", obyte, ibyte);
	return ibyte;
}


static int dpci_ow_xfer_block(struct dpci_ow_port *dowp, struct owblock *owbp)
{
	int i = 0;
	int ret = 0;
	char *buf = NULL;

	if (owbp->ob_do_reset)
	{
		ret = dowp->dowp_resetpulse(dowp);
		if (!ret)
			return -ENODEV;
	}

	buf = kmalloc(owbp->ob_datalen, GFP_KERNEL);
	if (!buf)
	{
		ret = -ENOMEM;
		goto done;
	}
	if (copy_from_user(buf, owbp->ob_buffer, owbp->ob_datalen))
	{
		ret = -EFAULT;
		goto done;
	}

	for (i = 0; i < owbp->ob_datalen; i++)
	{
		buf[i] = dpci_ow_xfer_byte(dowp, buf[i]);
	}
done:
	if (i && copy_to_user(owbp->ob_buffer, buf, owbp->ob_datalen))
	{
		ret = -EFAULT;
	}
	if (buf)
		kfree(buf);
	PRINT_OW("port %d: transferred %d bytes\n", owbp->ob_portnum, i);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_ow_ioctl()
 *
 * Parameters:  file - the file the user is using to access the inode
 *		cmd - the command being requested
 *		arg - additioinal data for the command.
 *
 * Returns:     status and or return data for the command.
 *
 * Description: Performs all i/o control operations specific to the DPCI device
 *		node.
 *
 *		The following error codes are used in returns; some are not
 *		used when the no other error code can occur (e.g. write-byte)
 *		but are used clearly when multiple causes of failure can occur
 *		(e.g. in write-byte-power etc.)
 *              -EINVAL: ioctl command not recognised.
 *              -EBUSY: port is in use by some other agent.
 *              -ENXIO: numbered port does not exist.
 *              -EACCES: cannot access device because no write permission.
 *              -EFAULT: something wrong with data (pointer) passed by user.
 *              -EIO: input data didn't match output data
 *		-ENOENT: power level not supported
 *		0/1: operation succeeded with given result
 *
 ******************************************************************************/
int dpci_ow_ioctl(struct file *file,
			unsigned int cmd,
			unsigned long arg)
{
	int ret = 0;
	struct dpci_ow_port *dowp = NULL;
	struct dpci_device *ddp = file->private_data;
	struct owdata owdata;
	struct owblock owblock;
	struct getname getname;
	int portno = -1;
	int bit;

	PRINT_IOC("(file=0x%p, cmd=0x%x, arg=0x%lx)\n", file, cmd, arg);

	/*
	 * Do some preliminary common checking of the port number
	 */
	switch (cmd)
	{
	case IOCTL_OW_LL_TOUCHRESET:
	case IOCTL_OW_LL_PROGRAMPULSE:
	case IOCTL_OW_LL_HASPOWER:
	case IOCTL_OW_LL_HASOVERDRIVE:
	case IOCTL_OW_LL_HASPROGPULSE:
	case IOCTL_OW_SE_RELEASE:
	case IOCTL_OW_SE_ACQUIRE:
	case IOCTL_OW_SE_GETUSER:
		portno = arg;
		if ((dowp = dpci_ow_get_port(ddp, portno)) == NULL)
		{
			ret = -ENXIO;
			goto done;
		}
		break;

	case IOCTL_OW_LL_TOUCHBIT:
	case IOCTL_OW_LL_TOUCHBYTE:
	case IOCTL_OW_LL_SPEED:
	case IOCTL_OW_LL_LINELEVEL:
	case IOCTL_OW_LL_WRITEBYTEPOWER:
	case IOCTL_OW_LL_READBITPOWER:
		if (copy_from_user(&owdata, (void *)arg, sizeof(owdata)))
		{
			ret = -EFAULT;
			goto done;
		}
		portno = owdata.od_portnum;
		if ((dowp = dpci_ow_get_port(ddp, portno)) == NULL)
		{
			ret = -ENXIO;
			goto done;
		}
		break;

	case IOCTL_OW_LL_GETNAME:
		break;

	case IOCTL_OW_TR_BLOCK:
		if (copy_from_user(&owblock, (void *)arg, sizeof(owblock)))
		{
			ret = -EFAULT;
			goto done;
		}
		portno = owblock.ob_portnum;
		if ((dowp = dpci_ow_get_port(ddp, portno)) == NULL)
		{
			ret = -ENXIO;
			goto done;
		}
		break;

	default:
		ret = -EINVAL;
		goto done;
	}

        /*
	 * For some commands, the port must be acquired.
	 */
	switch (cmd)
	{
	case IOCTL_OW_LL_TOUCHRESET:
	case IOCTL_OW_LL_PROGRAMPULSE:
	case IOCTL_OW_LL_TOUCHBIT:
	case IOCTL_OW_LL_TOUCHBYTE:
	case IOCTL_OW_LL_SPEED:
	case IOCTL_OW_LL_LINELEVEL:
	case IOCTL_OW_LL_WRITEBYTEPOWER:
	case IOCTL_OW_LL_READBITPOWER:
	case IOCTL_OW_SE_RELEASE:
	case IOCTL_OW_TR_BLOCK:
		if (dowp->dowp_acquired != current->pid)
		{
			PRINT_OW("Access requested on port %d by pid %d "
					"while pid %d has it\n",
					portno,
					current->pid,
					dowp->dowp_acquired);
			ret = -EBUSY;
			goto done;
		}
		break;
	}

	/*
	 * Now actually implement the command.
	 */
	switch (cmd)
	{
	case IOCTL_OW_LL_TOUCHRESET:
		ret = dowp->dowp_resetpulse(dowp);
		break;

	case IOCTL_OW_LL_TOUCHBIT:
		ret = dowp->dowp_touchbit(dowp, owdata.od_data);
		break;

	case IOCTL_OW_LL_TOUCHBYTE:
		ret = dpci_ow_xfer_byte(dowp, owdata.od_data);
		break;

	case IOCTL_OW_LL_SPEED:
		if (dowp->dowp_setspeed)
		{
			ret = dowp->dowp_setspeed(dowp, owdata.od_data);
		}
		else
		{
			// If the port doesn't support speed setting then
			// simply say we're always at normal speed.
			//
			ret = DOWP_SPEED_NORMAL;
		}
		break;

	case IOCTL_OW_LL_LINELEVEL:
		if (dowp->dowp_setlevel)
		{
			ret = dowp->dowp_setlevel(dowp, owdata.od_data);
		}
		else
		{
			// If the port doesn't support level setting then
			// say we're always at normal power level.
			//
			ret = DOWP_POWER_NORMAL;
		}
		break;

	case IOCTL_OW_LL_PROGRAMPULSE:
		if (!dowp->dowp_programpulse)
		{
			ret = -EINVAL;
			goto done;
		}
		ret = dowp->dowp_programpulse(dowp);
		break;

	case IOCTL_OW_LL_HASPOWER:
		ret = dowp->dowp_flags & DOWP_FLAGS_HAVE_POWER;
		break;

	case IOCTL_OW_LL_HASOVERDRIVE:
		ret = dowp->dowp_flags & DOWP_FLAGS_HAVE_ODRIVE;
		break;

	case IOCTL_OW_LL_HASPROGPULSE:
		ret = dowp->dowp_flags & DOWP_FLAGS_HAVE_PROGRAM;
		break;

	case IOCTL_OW_LL_WRITEBYTEPOWER:
		if (!(dowp->dowp_flags & DOWP_FLAGS_HAVE_POWER))
		{
			ret = -ENOENT;
			goto done;
		}
		ret = dpci_ow_xfer_byte(dowp, owdata.od_data);
		if (ret != owdata.od_data)
			ret = -EIO;
		else if (dowp->dowp_setlevel(dowp, DOWP_POWER_STRONG5) !=
							DOWP_POWER_STRONG5)
			ret = -ENOENT;
		else
			ret = 1;
		break;

	case IOCTL_OW_LL_READBITPOWER:
		if (!(dowp->dowp_flags & DOWP_FLAGS_HAVE_POWER))
		{
			ret = -ENOENT;
			goto done;
		}
		bit = dowp->dowp_touchbit(dowp, owdata.od_data);
		if ((bit && !owdata.od_data) || (!bit && owdata.od_data))
			ret = -EIO;
		else if (dowp->dowp_setlevel(dowp, DOWP_POWER_STRONG5) !=
							DOWP_POWER_STRONG5)
			ret = -ENOENT;
		else
			ret = 1;
		break;

	case IOCTL_OW_LL_GETNAME:
		if (copy_from_user(&getname, (void *)arg, sizeof(getname)))
		{
			ret = -EFAULT;
			goto done;
		}
		if ((dowp = dpci_ow_get_port(ddp, getname.gn_portnum)) == NULL)
		{
			ret = -ENXIO;
			goto done;
		}
		strncpy(getname.gn_buf, dowp->dowp_desc, sizeof(getname.gn_buf));
		if (copy_to_user((void *)arg, &getname, sizeof(getname)))
		{
			ret = -EFAULT;
			goto done;
		}
		ret = 0;
		break;

	case IOCTL_OW_SE_ACQUIRE:
		/*
		 * See if we can acquire this port.  If the port is currently
		 * acquired, then check the task for the PID we recorded in a
		 * previous acquisition request still exists.  If not then, we
		 * can hand over the portr straightaway.
		 */
		if (dowp->dowp_acquired)
		{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
			if (pid_task(find_get_pid(dowp->dowp_acquired), PIDTYPE_PID) != NULL)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
			if (find_task_by_vpid(dowp->dowp_acquired) != NULL)
#else
			if (find_task_by_pid(dowp->dowp_acquired) != NULL)
#endif
			{
				PRINT_OW("Acquisition of port %d by pid %d "
						"failed: pid %d has it\n",
						portno,
						current->pid,
						dowp->dowp_acquired);
				ret = 0;
				goto done;
			}
			PRINT_OW("Recovered port %d from previous acquisition "
					"by pid %d\n",
					portno,
					dowp->dowp_acquired);
		}
		dowp->dowp_acquired = current->pid;
		PRINT_OW("Port %d successfully acquired by pid %d\n",
				portno,
				dowp->dowp_acquired);
		/*
		 * Set the speed and level to reasonable defaults.
		 */
		if (dowp->dowp_setspeed)
			(void) dowp->dowp_setspeed(dowp, DOWP_SPEED_NORMAL);
		if (dowp->dowp_setlevel)
			(void) dowp->dowp_setlevel(dowp, DOWP_POWER_NORMAL);
		ret = 1;
		break;

	case IOCTL_OW_SE_RELEASE:
		/*
		 * Set the line speed to normal if allowed.  Ignore any errors.
		 */
		if (dowp->dowp_setspeed)
			(void) dowp->dowp_setspeed(dowp, DOWP_SPEED_NORMAL);
		if (dowp->dowp_setlevel)
			(void) dowp->dowp_setlevel(dowp, DOWP_POWER_NORMAL);
		PRINT_OW("Port %d successfully released by pid %d\n",
				portno,
				dowp->dowp_acquired);
		dowp->dowp_acquired = 0;
		ret = 1;
		break;

	case IOCTL_OW_SE_GETUSER:
		ret = dowp->dowp_acquired;
		break;

	case IOCTL_OW_TR_BLOCK:
		ret = dpci_ow_xfer_block(dowp, &owblock);
		break;

	default:
		ret = -EINVAL;
	}

done:
	PRINT_IOC("(file=0x%p, cmd=0x%x, arg=0x%lx) = %d\n",
		file, cmd, arg, ret);
	return ret;
}
#endif


#if defined(USE_DGL_OW_API) || defined(CONFIG_W1)
/*******************************************************************************
 *
 * Function:    dpci_ow_init_port()
 *
 * Parameters:  dowp * ptr  - to a struct dpci_ow_port.
 *
 * Returns:	int - 0 for success, otherwise -errno
 *
 * Description: Setup port access.  Write a one to the port and ensure it comes
 *              back as a one.  If not then we have a problem.
 *
 ******************************************************************************/
int dpci_ow_init_port(struct dpci_ow_port *dowp)
{
	u8 got;

	dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, dowp->dowp_wbit, 0, 0);
	udelay(5000);
	got = dpci_io_readbyte(dowp->dowp_ddp, dowp->dowp_reg);
	if (!(got & dowp->dowp_wbit) || !(got & dowp->dowp_rbit))
	{
		PRINT_DBG("wrote %02x, got back %02x\n",
			dpci_reg_get_saved(dowp->dowp_ddp, dowp->dowp_reg),
			got);
		return -EIO;
	}

	/*
	 * Reset speed to normal (not overdrive).
	 */
	DOWP_CONFIG(dowp) = 0;
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_ow_set_port_speed()
 *
 * Parameters:  data - void * ptr to a struct dpci_ow_port.
 *
 * Returns:	int - 0 for success, otherwise -errno
 *
 * Description: Setup port speed.
 *
 ******************************************************************************/
int dpci_ow_set_port_speed(struct dpci_ow_port *data, int speed)
{
	struct dpci_ow_port *dowp = (struct dpci_ow_port *)data;

	if (speed == DOWP_SPEED_NORMAL || speed == DOWP_SPEED_OVERDRIVE)
	{
		DOWP_CONFIG(dowp) &= ~DOW_CONFIG_SPEED_MASK;
		DOWP_CONFIG(dowp) |= speed & DOW_CONFIG_SPEED_MASK;
		PRINT_OW("set port speed to %s\n", (DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) ? "o-drive" : "normal");
	}
	return (DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK);
}


/*******************************************************************************
 *
 * Function:    dpci_ow_set_port_level()
 *
 * Parameters:  data - void * ptr to a struct dpci_ow_port.
 *
 * Returns:	int - 0 for success, otherwise -errno
 *
 * Description: Setup port speed.
 *
 ******************************************************************************/
int dpci_ow_set_port_level(struct dpci_ow_port *data, int level)
{
	struct dpci_ow_port *dowp = (struct dpci_ow_port *)data;

	if (level == DOWP_POWER_NORMAL || level == DOWP_POWER_STRONG5)
	{
		DOWP_CONFIG(dowp) &= ~DOW_CONFIG_LEVEL_MASK;
		DOWP_CONFIG(dowp) |= level & DOW_CONFIG_LEVEL_MASK;
		PRINT_OW("set port level to %s\n", level ? "strong" : "normal");
	}
	return (DOWP_CONFIG(dowp) & DOW_CONFIG_LEVEL_MASK);
}


/*******************************************************************************
 *
 * Function:    dpci_ow_reset_pulse()
 *
 * Parameters:  data - void * ptr to a struct dpci_ow_port.
 *              bit - zero or non-zero - the value to write.
 *
 * Returns:	presence bit
 *
 * Description: Send a reset pulse on a one-wire network.
 *              These timings are described at this URL.
 *
 *              http://www.maxim-ic.com/appnotes.cfm/appnote_number/126
 *
 ******************************************************************************/
u8 dpci_ow_reset_pulse(struct dpci_ow_port *data)
{
	struct dpci_ow_port *dowp = (struct dpci_ow_port *)data;
	int present;
	unsigned long flags;

	local_irq_save(flags);
	local_irq_disable();
	dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, dowp->dowp_wbit, 0, 0);
	udelay(OWTM_G_STD_US); // Timing 'G'
	dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, 0, dowp->dowp_wbit, 0);
	if ((DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) == DOWP_SPEED_OVERDRIVE)
		udelay(OWTM_H_OVR_US); // Timing 'H'
	else
		udelay(OWTM_H_STD_US);
	dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, dowp->dowp_wbit, 0, 0);
	if ((DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) == DOWP_SPEED_OVERDRIVE)
		ndelay(OWTM_I_OVR_NS); // Timing 'I'
	else
		udelay(OWTM_I_STD_US);
	if (dpci_io_readbyte(dowp->dowp_ddp, dowp->dowp_reg) & dowp->dowp_rbit)
		present = 0;
	else
		present = 1;
	if ((DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) == DOWP_SPEED_OVERDRIVE)
		udelay(OWTM_J_OVR_US); // Timing 'J'
	else
		udelay(OWTM_J_STD_US);
	local_irq_restore(flags);
	PRINT_OW("presence = %d\n", present);
	return present;
}


/*******************************************************************************
 *
 * Function:    dpci_ow_program_pulse()
 *
 * Parameters:  data - void * ptr to a struct dpci_ow_port.
 *
 * Returns:	0 if succesful, -EINVAL if not supported.
 *
 * Description: Send a program pulse on a one-wire network.
 *              These timings are described at this URL.
 *
 *              http://www.maxim-ic.com/appnotes.cfm/appnote_number/126
 *
 ******************************************************************************/
u8 dpci_ow_program_pulse(struct dpci_ow_port *data)
{
	int ret;
	struct dpci_ow_port *dowp = (struct dpci_ow_port *)data;
	unsigned long flags;

	if (dowp->dowp_pbit == 0)
	{
		ret = -EINVAL;
		goto done;
	}

	local_irq_save(flags);
	local_irq_disable();
	dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, dowp->dowp_pbit, 0, 0);
	udelay(OWTM_PROGPLS_US);
	dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, 0, dowp->dowp_pbit, 0);
	local_irq_restore(flags);
	ret = 0;
done:
	PRINT_OW("program pulse = %d\n", ret);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_ow_touch_bit()
 *
 * Parameters:  data - void * ptr to a struct dpci_ow_port.
 *              bit - zero or non-zero - the value to write.
 *
 * Returns:	nothing
 *
 * Description: Touch a single bit on the one-wire bus.  Write a 0 or a one; if
 *              a one was sent, then return the bit read back.
 *
 *              These timings are described at this URL.
 *
 *              http://www.maxim-ic.com/appnotes.cfm/appnote_number/126
 *
 ******************************************************************************/
u8 dpci_ow_touch_bit(struct dpci_ow_port *data, u8 obit)
{
	struct dpci_ow_port *dowp = (struct dpci_ow_port *)data;
	u8 ibit = 0;
	unsigned long flags;

	local_irq_save(flags);
	local_irq_disable();
	if (obit)
	{
		dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, 0, dowp->dowp_wbit, 0);
		if ((DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) == DOWP_SPEED_OVERDRIVE)
			ndelay(OWTM_A_OVR_NS); // Timing 'A'
		else
			udelay(OWTM_A_STD_US);
		dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, dowp->dowp_wbit, 0, 0);
		if ((DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) == DOWP_SPEED_OVERDRIVE)
			ndelay(OWTM_E_OVR_NS); // Timing 'E'
		else
			udelay(OWTM_E_STD_US);
		ibit = dpci_io_readbyte(dowp->dowp_ddp, dowp->dowp_reg) & dowp->dowp_rbit;
		if ((DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) == DOWP_SPEED_OVERDRIVE)
			udelay(OWTM_F_OVR_US); // Timing 'F'
		else
			udelay(OWTM_F_STD_US);
	}
	else
	{
		dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, 0, dowp->dowp_wbit, 0);
		if ((DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) == DOWP_SPEED_OVERDRIVE)
			ndelay(OWTM_C_OVR_NS); // Timing 'C'
		else
			udelay(OWTM_C_STD_US);
		dpci_reg_chg(dowp->dowp_ddp, dowp->dowp_reg, dowp->dowp_wbit, 0, 0);
		if ((DOWP_CONFIG(dowp) & DOW_CONFIG_SPEED_MASK) == DOWP_SPEED_OVERDRIVE)
			ndelay(OWTM_D_OVR_NS); // Timing 'D'
		else
			udelay(OWTM_D_STD_US);
	}
	PRINT_OW("wrote %d got %d\n", !!obit, !!ibit);
	local_irq_restore(flags);
	return ibit;
}


/*******************************************************************************
 *
 * Function:    dpci_ow_setup()
 *
 * Parameters:  dev - the input device being set up.
 *
 * Returns:	nothing
 *
 * Description: Setup Linux API W1 access.
 *
 ******************************************************************************/
static int dpci_ow_add_port(struct dpci_device *ddp,
				const struct dpci_ow_port *dowp0)
{
	int ret = 0;
	int p;
	struct dpci_ow_port *dowp;

	/*
	 * Find out which port to use.
	 */
	for (p = 0, dowp = ddp->ow_ports; p < MAX_OW_PORTS; p++, dowp++)
	{
		if (!dowp->dowp_desc)
			break;
	}
	if (p == MAX_OW_PORTS)
	{
		PRINT_ERR("OW port at reg. %02x: MAX_OW_PORTS exceeded.\n",
				dowp->dowp_reg);
		ret = -EBUSY;
		goto done;
	}

	/*
	 * Copy original port config structure.
	 */
	ddp->ow_ports[p] = *dowp0;
	ddp->ow_ports[p].dowp_ddp = ddp;

	dowp = &ddp->ow_ports[p];
	init_MUTEX(&dowp->dowp_sem);
	if (!dowp->dowp_touchbit || !dowp->dowp_resetpulse)
	{
		PRINT_ERR("OW port at reg. %02x: "
				"touchbit or reset method missing\n",
				dowp->dowp_reg);
		ret = -EINVAL;
		goto done;
	}

	/*
	 * If the port has an initialisation function then call it.  Should it
	 * fail then we abandon this port setup.
	 */
	if (dowp->dowp_init)
	{
		int ret;

		ret = dowp->dowp_init(dowp);
		if (ret != 0)
		{
			PRINT_ERR("couldn't initialise OW port at reg. %02x: "
					"err %d\n",
					dowp->dowp_reg,
					-ret);
			goto done;
		}
	}

#ifdef CONFIG_W1
	/*
	 * For Linux W1 config only the getbit and setbit ops are relevant at
	 * the moment.
	 */
	memset(&dowp->dowp_master, 0, sizeof(dowp->dowp_master));
	dowp->dowp_master->touch_bit = dowp->dowp_touchbit;
	dowp->dowp_master->data = dowp;
	ret = w1_add_master_device(&dowp->dowp_master);
	if (err)
	{
		PRINT_ERR("couldn't register W1 port at reg. %02x: err %d\n",
				p,
				dowp->dowp_reg,
				-ret);
		goto done;
	}
#endif
	ret = 0;
	PRINT_DBG("OW port #%d \"%s\" at reg %02x (w=%02x r=%02x)\n",
		p,
		dowp->dowp_desc,
		dowp->dowp_reg,
		dowp->dowp_wbit,
		dowp->dowp_rbit);
done:
	if (ret != 0)
	{
		memset(dowp, 0, sizeof(*dowp));
	}
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_ow_setup()
 *
 * Parameters:  dev - the input device being set up.
 *
 * Returns:	nothing
 *
 * Description: Setup OW port access - works for either API.
 *
 ******************************************************************************/
void dpci_ow_setup(struct dpci_device *ddp)
{
	int i;

	for (i = 0; ddp->mainboard->db_ow_ports[i].dowp_reg; i++)
	{
		dpci_ow_add_port(ddp, &ddp->mainboard->db_ow_ports[i]);
	}
	if (ddp->ioboard)
	{
		for (i = 0; ddp->ioboard->db_ow_ports[i].dowp_reg; i++)
		{
			dpci_ow_add_port(ddp, &ddp->ioboard->db_ow_ports[i]);
		}
	}
}


/*******************************************************************************
 *
 * Function:    dpci_ow_cleanup()
 *
 * Parameters:  dev - the input device being cleaned up.
 *
 * Returns:	nothing
 *
 * Description: Clean up one-wire ports - linux or DGL.  Not really needed for
 *              DGL ports but we do it anyway.
 *
 ******************************************************************************/
void dpci_ow_cleanup(struct dpci_device *ddp)
{
#ifdef CONFIG_W1
	int i;

	for (i = 0; i < MAX_OW_PORTS && ddp->ow_ports[i].dowp_reg; i++)
	{
		ret = w1_remove_master_device(&dowp->dowp_master);
	}
#endif
}


/*******************************************************************************
 *
 * Function:    dpci_ow_relinquish()
 *
 * Parameters:  dev - the input device being cleaned up.
 *
 * Returns:     nothing
 *
 * Description: Relinquish OW ports acquired by current thread and set level
 * 		and speed back to normal.
 *
 ******************************************************************************/
void dpci_ow_relinquish(struct dpci_device *ddp)
{
	int p;
	struct dpci_ow_port *dowp;

	for (p = 0; p < MAX_OW_PORTS; p++)
	{
		dowp = &ddp->ow_ports[p];
		if (dowp->dowp_acquired == current->pid)
		{
			dowp->dowp_acquired = 0;
			if (dowp->dowp_setspeed)
				(void) dowp->dowp_setspeed(dowp, DOWP_SPEED_NORMAL);
			if (dowp->dowp_setlevel)
				(void) dowp->dowp_setlevel(dowp, DOWP_POWER_NORMAL);
		}
	}
}

#endif

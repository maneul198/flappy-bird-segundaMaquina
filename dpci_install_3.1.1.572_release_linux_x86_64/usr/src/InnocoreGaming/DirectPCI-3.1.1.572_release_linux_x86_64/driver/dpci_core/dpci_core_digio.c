/******************************************************************************
 *
 * $Id: dpci_core_digio.c 11939 2015-09-14 22:31:13Z aidan $
 *
 * Copyright 2003-2015 Advantech Co. Ltd.
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
#include "dpci_core_hw.h"
#include "dpci_core_priv.h"


/*******************************************************************************
 *
 * Function:    dpci_enable_digio_intr()
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Enables digital I/O interrupts.  A count of requests is kept.
 *              Interrupts are only enabled on the first request.  If we have
 *              the EXINT bit supported in hardware, we set that, otherwise we
 *              simple enable I/O board interrupts since the (IO)INTEN bit is
 *              always controls digital I/O interrupts regardless of whether
 *              they're on board or not.  We also clear the interrupt enable
 *              bits for each input port so that the ports start from a known
 *              state.
 *
 ******************************************************************************/
void dpci_enable_digio_intr(struct dpci_device *ddp)
{
	if (!ddp->digio_users++)
	{
		int port;

		if (ddp->features & HAVE_IOINTONEXINT)
		{
			dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_EXINTEN, 0, 0);
		}
		else
		{
			dpci_enable_ioboard_intr(ddp);
		}

		/*
		 * Enable all I/O board interrupts.
		 */
		if (ddp->ioboard)
			dpci_set_ioboard_intcfg(ddp, DPCI_IOBOARD_INTCFG_IO, 0);

		/*
		 * Initialise the input interrupt register so as to disable
		 * all interrupts.
		 */
		if (ddp->reg_ip_status)
		{
			for (port = 0; ddp->reg_ip_status[port]; port++)
			{
				dpci_io_modifyint(ddp, port, 0, 0xff, 0);
			}
		}
	}
}


/*******************************************************************************
 *
 * Function:    dpci_disable_digio_intr()
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Disable I/O board interrupts.  Interrupts are only disabled on
 *		the request counter reaching zero again.  At this point we also
 *              clear the input register interrupt enable bits.
 *
 ******************************************************************************/
void dpci_disable_digio_intr(struct dpci_device *ddp)
{
	if (!--ddp->digio_users)
	{
		int port;

		if (ddp->features & HAVE_IOINTONEXINT)
		{
			dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_EXINTEN, 0);
		}
		else
		{
			dpci_disable_ioboard_intr(ddp);
		}

		/*
		 * Disable all interrupts from input ports.
		 */
		if (ddp->reg_ip_status)
		{
			if (ddp->ioboard)
				dpci_set_ioboard_intcfg(ddp, 0, DPCI_IOBOARD_INTCFG_IO);
			for (port = 0; ddp->reg_ip_status[port]; port++)
			{
				dpci_io_modifyint(ddp, port, 0, 0xff, 0);
			}
		}
	}
}


/*******************************************************************************
 *
 * Function:    dpci_io_modifyint()
 *
 * Parameters:  port - the port to modify
 *              set_mask - bit mask of interrupts to enable
 *              clr_mask - bit mask of interrupts to disable
 *		pol_mask - polarity mask governing type of edge for trigger
 *
 * Returns:     0 - for success
 *		1 - for failure if I/O card not present.
 *
 * Description: Configure the interrupt policy for handling an input port's
 *              changes.
 *
 ******************************************************************************/
int dpci_io_modifyint(struct dpci_device *ddp, int port,
				u8 int_setmask,
				u8 int_clrmask,
				u8 pol_mask)
{
	/*
	 * Check function preconditions
	 */
	if(port < 0 || port >= MAX_INPUT_PORTS)
	{
		PRINT_ERR("Invalid input port number!\n");
		return -EINVAL;
	}

	PRINT_IO("(%p, %d, %02x, %02x, %02x)\n",
		ddp, port, int_setmask, int_clrmask, pol_mask);

	if (!ddp->reg_ip)
	{
		PRINT_DBG("no input registers!\n");
		return -ENODEV;
	}

	/*
	 * Splice in the appropriate bits of masks.  Only bits that are 1 in
	 * int_setmask can be changed in the edge mask.
	 */
	if (ddp->features & HAVE_NEWDIGIOREGS)
	{
		u8 setmasko, setmaske;
		u8 clrmasko, clrmaske;
		u8 polmasko, polmaske;
		dpci_off_t rego, rege;

		if (!ddp->reg_ip_cfgeven || !ddp->reg_ip_cfgeven[port])
		{
			return -ENODEV;
		}
		rege = ddp->reg_ip_cfgeven[port];
		rego = ddp->reg_ip_cfgodd[port];
		setmasko = (int_setmask & 0xaa);
		setmasko |= (setmasko >> 1);
		setmaske = (int_setmask & 0x55);
		setmaske |= (setmaske << 1);
		clrmasko = ((int_setmask | int_clrmask) & 0xaa);
		clrmasko |= (clrmasko >> 1);
		clrmaske = ((int_setmask | int_clrmask) & 0x55);
		clrmaske |= (clrmaske << 1);
		polmasko = (~pol_mask) & 0xaa;
		polmasko |= (pol_mask & 0xaa) >> 1;
		polmasko &= setmasko;
		polmaske = (pol_mask & 0x55);
		polmaske |= ((~pol_mask) & 0x55) << 1;
		polmaske &= setmaske;
		PRINT_IO("setmask(o,e)=(%02x,%02x)\n", setmasko, setmaske);
		PRINT_IO("clrmask(o,e)=(%02x,%02x)\n", clrmasko, clrmaske);
		PRINT_IO("polmask(o,e)=(%02x,%02x)\n", polmasko, polmaske);
		dpci_reg_chg(ddp, rege, polmaske, clrmaske, 0);
		dpci_reg_chg(ddp, rego, polmasko, clrmasko, 0);
		PRINT_IO("IPP%d interrupt config odd=%02x even=%02x\n",
			port,
			dpci_reg_get_saved(ddp, rego),
			dpci_reg_get_saved(ddp, rege));
	}
	else
	{
		if (!ddp->reg_ip_enable[port])
		{
			return -ENODEV;
		}
		dpci_reg_chg(ddp, ddp->reg_ip_enable[port], int_setmask, int_clrmask, 0);
		dpci_reg_chg(ddp, ddp->reg_ip_pol[port], pol_mask & int_setmask, int_setmask, 0);
		PRINT_IO("IPP%d interrupt config ena=%02x pol=%02x\n",
			port,
			dpci_reg_get_saved(ddp, ddp->reg_ip_enable[port]),
			dpci_reg_get_saved(ddp, ddp->reg_ip_pol[port]));
	}
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_io_wait_int()
 *
 * Parameters:	waitint - the wait int parameter block from the user.
 *		cont_wait - should wait continue after one event
 *
 * Returns:     status and or return data for the command.
 *
 * Description: Performs all i/o control operations specific to the DPCI device
 *		node.
 *
 ******************************************************************************/
int dpci_io_wait_int(struct dpci_device *ddp, 
		struct waitint_port *waitint_port,
		int cont_wait)
{
        int ret;
        struct dpci_event op_event;
	int port;
        unsigned long int_mask;
        unsigned long edge_state;
        unsigned long autoconfig_edge;
	struct dio_event dio_event;

	port = waitint_port->port;
        int_mask = waitint_port->int_mask << (port * 8);
        edge_state = waitint_port->edge_state << (port * 8);
	autoconfig_edge = waitint_port->autoconfig_edge << (port * 8);
        ret = dpci_wait_events(ddp, 
			EVENT_DIG_IP, 
			waitint_port->timeout_ms,
			cont_wait, 
			&op_event, 
			int_mask, 
			edge_state,
			autoconfig_edge);
        if (ret < 0)
        {
                PRINT_DBG("interrupted by signal\n");
        }
	else if (ret == 0)
        {
                PRINT_DBG("timeout waiting for digital i/o interrupt\n");
        }
        else if (op_event.de_type == EVENT_DIG_IP)
        {
		dio_event = op_event.de_data.dio_event;
		waitint_port->op_data.change_mask = 
			(dio_event.change_mask >> (port * 8)) & 0xff;
		waitint_port->op_data.dio_value = 
			(dio_event.dio_value >> (port * 8)) & 0xff;
        }
	else
	{
		/*
		 * We should really not get here. We have been signalled on 
		 * an event that we were not waiting for. Log an error and 
		 * return error status
		 */
		PRINT_ERR("Unexpected event type signalled\n");
		PRINT_ERR("Waiting for Event type %d, got event type %d", 
			EVENT_DIG_IP, op_event.de_type);
		ret = -EFAULT;
	}
        PRINT_DBG("int wait return= 0x%02x\n", ret);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_io_wait_int32()
 *
 * Parameters:	waitint32 - the wait int parameter block from the user.
 *
 * Returns:     status and or return data for the command.
 *
 * Description: Performs all i/o control operations specific to the DPCI device
 *		node.
 *
 ******************************************************************************/
int dpci_io_wait_int32(struct dpci_device *ddp, struct waitint_32 *waitint32)
{
        int ret;
        struct dpci_event op_event;

        ret = dpci_wait_events(ddp, 
			EVENT_DIG_IP, 
			waitint32->timeout, 
			1,
			&op_event, 
			waitint32->int_mask, 
			waitint32->edge_state,
			waitint32->autoconfig_edge);
        if (ret < 0)
        {
                PRINT_DBG("interrupted by signal\n");
        }
	else if (ret == 0)
        {
                PRINT_DBG("timeout waiting for digital i/o interrupt\n");
        }
        else if (op_event.de_type == EVENT_DIG_IP)
        {
		waitint32->op_data = op_event.de_data.dio_event;
        }
	else
	{
		/*
		 * We should really not get here. We have been signalled on 
		 * an event that we were not waiting for. Log an error and 
		 * return error status
		 */
		PRINT_ERR("Unexpected event type signalled\n");
		PRINT_ERR("Waiting for Event type %d, got event type %d", 
			EVENT_DIG_IP, op_event.de_type);
		ret = -EFAULT;
	}
        PRINT_DBG("int wait return= 0x%02x\n", ret);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_io_wait_pfd()
 *
 * Parameters:  ddp - the device we are talking to
 *              arg - timeout
 *
 * Returns:     <0 - error; =0 - timeout; =1 - PFD interrupt occured.
 *
 * Description: Waits for a power-fail detect interrupt to occur in a
 *              during a specified timeout
 *
 ******************************************************************************/
int dpci_io_wait_pfd(struct dpci_device *ddp, unsigned long arg)
{
        int ret;
        struct dpci_event op_event;

        if (!(ddp->features & HAVE_POWERFAILDETECT))
        {
                return -ENXIO;
        }
        ret = dpci_wait_events(ddp, 
			EVENT_PFD, 
			arg, 
			0,
			&op_event, 
			0, 
			0,
			0);
        if (ret < 0)
        {
                PRINT_DBG("interrupted by signal\n");
        }
	else if (ret == 0)
        {
                PRINT_DBG("timeout waiting for PFD interrupt\n");
        }
        else if (op_event.de_type != EVENT_PFD)
	{
		/*
		 * We should really not get here. We have been signalled on 
		 * an event that we were not waiting for. Log an error and 
		 * return error status
		 */
		PRINT_ERR("Unexpected event type signalled\n");
		PRINT_ERR("Waiting for Event type %d, got event type %d", 
			EVENT_PFD, op_event.de_type);
		ret = -EFAULT;
	}
        PRINT_DBG("PFD wait return= 0x%02x\n", ret);
	return ret;
}

/*******************************************************************************
 *
 * Function:    dpci_ts_wait_alarm()
 *
 * Parameters:  ddp - the device we are talking to
 *              arg - timeout
 *
 * Returns:     <0 - error; =0 - timeout; =1 - PFD interrupt occured.
 *
 * Description: Waits for a temperature sensor alarm to occur in a
 *              during a specified timeout
 *
 ******************************************************************************/
int dpci_ts_wait_alarm(struct dpci_device *ddp, unsigned long arg)
{
        int ret;
        struct dpci_event op_event;

	/*
	 * See if there's an I/O board II present.
	 */
	if (!ddp->ioboard ||
		ddp->ioboard->db_board_id != DPCI_IOBOARD_ID_80_0062A)
	{
		return -ENXIO;
	}

        ret = dpci_wait_events(ddp, 
			EVENT_TS, 
			arg, 
			0,
			&op_event, 
			0, 
			0,
			0);
        if (ret < 0)
        {
                PRINT_DBG("interrupted by signal\n");
        }
	else if (ret == 0)
        {
                PRINT_DBG("timeout waiting for Temperature sensor alarm\n");
        }
        else if (op_event.de_type != EVENT_TS)
	{
		/*
		 * We should really not get here. We have been signalled on 
		 * an event that we were not waiting for. Log an error and 
		 * return error status
		 */
		PRINT_ERR("Unexpected event type signalled\n");
		PRINT_ERR("Waiting for Event type %d, got event type %d", 
			EVENT_TS, op_event.de_type);
		ret = -EFAULT;
	}
        PRINT_DBG("TS wait return= 0x%02x\n", ret);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_iowd_enable()
 *
 * Parameters:  ddp - the device instance
 *
 * Returns:     <0 - Error; 0 = Success
 *
 * Description: Enables the I/O watchdog on 80-0062 and 80-1003 ioboards.
 *
 ******************************************************************************/
int dpci_iowd_enable(struct dpci_device *ddp)
{
	if (ddp->ioboard)
	{
		/*
 		 * DPCI 116/117/C-series
 		 */
		u8 ret = 0;
		ret = dpci_set_ioboard_intcfg(ddp, 
					DPCI_IOBOARD_INTCFG_WDRC, 
					DPCI_IOBOARD_INTCFG_OE);
		return 0;
	}
	else
	{
		/*
 		 * DPX 112/S-series
 		 */
		return -ENXIO;
	}
}


/*******************************************************************************
 *
 * Function:    dpci_iowd_disable()
 *
 * Parameters:  ddp - the device instance
 *
 * Returns:     <0 - Error; 0 = Success
 *
 * Description: Disables the I/O watchdog on 80-0062 and 80-1003 ioboards.
 *
 ******************************************************************************/
int dpci_iowd_disable(struct dpci_device *ddp)
{
	if (ddp->ioboard)
	{
		/*
 		 * DPCI 116/117/C-series
 		 */
		u8 ret = 0;
		ret = dpci_set_ioboard_intcfg(ddp,
					0, 
					DPCI_IOBOARD_INTCFG_WDRC);
		return 0;
	}
	else
	{
		/*
 		 * DPX 112/S-series
 		 */
		return -ENXIO;
	}
}


/*******************************************************************************
 *
 * Function:    dpci_iowd_pat()
 *
 * Parameters:  ddp - the device instance
 *
 * Returns:     <0 - Error; 0 = Success
 *
 * Description: Pats(resets the WDT counter) the I/O watchdog on 80-0062 and i
 *		80-1003 ioboards.
 *
 ******************************************************************************/
int dpci_iowd_pat(struct dpci_device *ddp)
{
	if (ddp->ioboard)
	{
		/*
 		 * DPCI 116/117/C-series
 		 */
		dpci_io_readbyte(ddp, ddp->reg_ip[0]);
		return 0;
	}
	else
	{
		/*
 		 * DPX 112/S-series
 		 */
		return -ENXIO;
	}
}


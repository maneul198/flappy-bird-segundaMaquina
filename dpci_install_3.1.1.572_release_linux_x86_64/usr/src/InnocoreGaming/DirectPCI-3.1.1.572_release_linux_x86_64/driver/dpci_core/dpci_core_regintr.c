/******************************************************************************
 *
 * $Id: dpci_core_regintr.c 11905 2015-09-07 15:28:17Z aidan $
 *
 * Copyright 2003-2011 Advantech Corporation Limited.
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

unsigned int all_interrupts = 0;
unsigned int unassigned_interrupts = 0;
unsigned int localio_interrupts = 0;
unsigned int idlp_event_interrupts = 0;
unsigned int idlp_ack_interrupts = 0;
unsigned int exio_interrupts = 0;
unsigned int digio_interrupts = 0;
unsigned int ts_interrupts = 0;
unsigned int pfd_interrupts = 0;


/*******************************************************************************
 *
 * Function:    dpci_set_exint_handler()
 *
 * Parameters:  ddp - the device for which to set the handler
 *              handler - the interrupt to call.
 *
 * Returns:     0 for success, -EBUSY if interrupt already set.
 *
 * Description: Sets the interrupt handler to be called when an external
 *              interrupt (EXINT) occurs.  Only one handler may be set and an
 *		error occurs if an attempt is made to set more than one.
 *
 ******************************************************************************/
int dpci_set_exint_handler(struct dpci_device *ddp,
				exint_handler_t handler,
				void *private)
{
	if (ddp->exint_handler)
		return -EBUSY;
	ddp->exint_handler = handler;
	ddp->exint_handler_data = private;
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_set_exint_handler()
 *
 * Parameters:  ddp - the device for which to set the handler
 *              handler - the interrupt to call.
 *
 * Returns:     0 for success, -EBUSY if interrupt already set.
 *
 * Description: Sets the interrupt handler to be called when an external
 *              interrupt (EXINT) occurs.  Only one handler may be set and an
 *		error occurs if an attempt is made to set more than one.
 *
 ******************************************************************************/
int dpci_clear_exint_handler(struct dpci_device *ddp,
				exint_handler_t handler,
				void *private)
{
	if (ddp->exint_handler != handler && ddp->exint_handler_data != private)
		return -EINVAL;
	ddp->exint_handler = NULL;
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_ioboard_supported()
 *
 * Parameters:  none
 *
 * Returns:     u8 - the IO board support status. 1 - support; 0 - not supported
 *
 * Description: Returns the status of IO board support on the hardware
 *
 ******************************************************************************/
u8 dpci_ioboard_supported(struct dpci_device *ddp)
{
	if(ddp->features & HAVE_DENSITRON_IOEXP)
		return 1;
	else
		return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_get_ioboard_id()
 *
 * Parameters:  none
 *
 * Returns:     u8 - the ID byte for the I/O card.
 *
 * Description: Returns the ID byte as described, assuming the I/O card's 1st
 *		register behaves as the standard I/O board does (ID 0x03).
 *
 ******************************************************************************/
u8 dpci_get_ioboard_id(struct dpci_device *ddp)
{
	return dpci_io_readbyte(ddp, DPCI_IOBOARD_ID);
}


/*******************************************************************************
 *
 * Function:    dpci_get_ioboard_rev()
 *
 * Parameters:  none
 *
 * Returns:     u8 - the revision byte for the I/O card.
 *
 * Description: Returns the rev byte as described, assuming the I/O card's 2nd
 *		register behaves as the standard I/O board does (ID 0x03).
 *
 ******************************************************************************/
u8 dpci_get_ioboard_rev(struct dpci_device *ddp)
{
	return dpci_io_readbyte(ddp, DPCI_IOBOARD_REV);
}


/*******************************************************************************
 *
 * Function:    dpci_get_uart_offsets()
 *
 * Parameters:  buf - where to put the offsets of the uarts
 *              size - the size of the buffer (bytes).
 *
 * Returns:     int - the number of uarts.
 *
 * Description: returns the offsets of the uarts supported by the DirectPCI
 *              core I/O facility and any attached I/O expansion.
 *
 ******************************************************************************/
int dpci_get_uart_offsets(struct dpci_device *ddp, dpci_off_t *buf, int size)
{
	int count = 0;
	const struct dpci_board *dbp;

	if (ddp->ioboard)
		dbp = ddp->ioboard;
	else
		dbp = ddp->mainboard;
	for (count = 0; dbp->db_uart_regs[count] && count < MAX_UARTS; count++)
	{
	 *(buf++) = dbp->db_uart_regs[count];
		PRINT_DBG("Uart%d at offset %02x\n",
			count,
			dbp->db_uart_regs[count]);
	}

	return (count);
}


/*******************************************************************************
 *
 * Function:    dpci_get_ioaddress()
 *
 * Parameters:  none
 *
 * Returns:     int - the address of the DPCI chip's I/O segment.
 *
 * Description: returns the I/O address of the DPCI chip, as assigned to base
 * address 0 register in function 0 of the PCI device's configuration space.
 *
 ******************************************************************************/
int dpci_get_ioaddress(struct dpci_device *ddp)
{
	return ddp->address;
}


/*******************************************************************************
 *
 * Function:    dpci_get_irq()
 *
 * Parameters:  none
 *
 * Returns:     u8 - the interrupt line assigned to the DPCI chip.
 *
 * Description: Returns the interrupt number, which may be required by other
 *		drivers connected to the DirectPCI bus and which need to signal
 *		interrupts.
 *
 ******************************************************************************/
u8 dpci_get_irq(struct dpci_device *ddp)
{
	return ddp->irq_line;
}


/*******************************************************************************
 *
 * Function:    ip_irq_handler
 *
 * Parameters:  none.
 *
 * Returns:     nothing.
 *
 * Description: Handles an interrupt for IO reasons, i.e. input bit changed.
 *		If the input device is open then we report input events in the
 *		appropriate manner.  If some one is waiting on the interrupt
 *		wait queue than we wake that queue up.
 *
 ******************************************************************************/
irqreturn_t ip_irq_handler(struct dpci_device *ddp)
{
	int port;
	int status;
	int btn = BTN_0;
	u8 ip;
	int io_changed = 0;
	unsigned long dio_value = 0;
	unsigned long change_mask = 0;
	unsigned long ip_status = 0;
	u8 ip_status_8b;
	u8 int_mask_8b;

	PRINT_IRQ("ddp=%p\n", ddp);
	if (!ddp->reg_ip_status)
	{
		PRINT_ERR("input interrupt without an input device!\n");
		return IRQ_NONE;
	}
	for (port = 0;
		port < MAX_INPUT_PORTS && ddp->reg_ip_status[port];
		port++, btn += 8)
	{
		ip = dpci_io_readbyte(ddp, ddp->reg_ip[port]);
		dio_value |= (ip << (port * 8));
		status = dpci_io_readbyte(ddp, ddp->reg_ip_status[port]);
		if (!status)
		{
			continue;
		}
		io_changed = 1;
		PRINT_IO("input %d: status=%02x data=%02x\n", port, status, ip);

		/*
		 * Save the status away.
		 */
		ddp->val_ip_status[port] |= status;
		ip_status_8b = ddp->val_ip_status[port];
		ip_status |= (ip_status_8b << (port * 8));
		int_mask_8b = 
		(ddp->thread_event.er_param.int_mask >> (port * 8)) & 0xff;
		ddp->val_ip_status[port] &= ~int_mask_8b;

		if (ddp->features & HAVE_NEWDIGIOREGS)
		{
			/*
			 * We do this for new boards if the input device is not
			 * in use.
			 *
			 * For new DPX-S-series boards, the I/O interrupt
			 * registers are a little different.  The status is now
			 * cleared by resetting the registers to their original
			 * values.  We can do this by asking dpci_chg_reg not
			 * to actually change anything.
			 */
			dpci_off_t rege, rego;

			rege = ddp->reg_ip_cfgeven[port];
			rego = ddp->reg_ip_cfgodd[port];
			dpci_reg_chg(ddp, rege, 0, 0, 0);
			dpci_reg_chg(ddp, rego, 0, 0, 0);
		}
	}
	if (io_changed)
	{
		digio_interrupts++;
		ddp->thread_event.op_event.de_type |= EVENT_DIG_IP;

		change_mask = ip_status & ddp->thread_event.er_param.int_mask;
		ddp->thread_event.op_event.de_data.dio_event.dio_value
						 	= dio_value;
		ddp->thread_event.op_event.de_data.dio_event.change_mask
						 	= change_mask;
		ddp->thread_event.er_signal = SIGNAL_EVENT;
		//printk(KERN_INFO "ip=%08lx cm=%08lx\n", dio_value, change_mask);
		if (waitqueue_active(&ddp->thread_event.er_wqh))
		{
			wake_up_interruptible(&ddp->thread_event.er_wqh);
		}
		return IRQ_HANDLED;
	}
	PRINT_IRQ("not handled\n");
	return IRQ_NONE;
}

/*******************************************************************************
 *
 * Function:    ts_irq_handler
 *
 * Parameters:  none.
 *
 * Returns:     nothing.
 *
 * Description: Handles an interrupt for TS reasons.
 *				If some one is waiting on the interrupt wait queue than we wake
 *				that queue up. If someone requested an alarm signal we send that 
 *				signal too.
 ******************************************************************************/
static irqreturn_t ts_irq_handler(struct dpci_device *ddp)
{
	PRINT_IRQ("ddp=%p\n", ddp);
	if (!ddp->ioboard)
	{
		PRINT_ERR("ts interrupt without an ioboard!\n");
		return IRQ_NONE;
	}
	
	{
		//disable TS interrupts
		//maybe we don't needed here as already in er_disable_interrupts
		//but may need it for IOCTL_DPCI_TS_SIGNAL
		dpci_set_ioboard_intcfg(ddp, 0, DPCI_IOBOARD62_INTCFG_TS);
		
		//increment ts interrupt counter
		ts_interrupts++;
		
		//notify threads
		ddp->thread_event.op_event.de_type |= EVENT_TS;
		ddp->thread_event.er_signal = SIGNAL_EVENT;

		if (waitqueue_active(&ddp->thread_event.er_wqh))
		{
			wake_up_interruptible(&ddp->thread_event.er_wqh);
		}
		
		//send signal if IOCTL_DPCI_TS_SIGNAL was called
		if (ddp->ts_pid)
		{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
			kill_pid(find_vpid(ddp->ts_pid), ddp->ts_signal, 1);
#else
			kill_proc(ddp->ts_pid, ddp->ts_signal, 1);
#endif

			ddp->ts_pid = 0;
		}
		
		return IRQ_HANDLED;
	}
	
}

/******************************************************************************
 *
 * Function:    ioboard_80_1003_irq_handler
 *
 * Parameters:  none.
 *
 * Returns:     Nothing.
 *
 * Description: Handles an interrupt from the I/O board.
 *              COM port interrupts are handled automatically so we don't need
 *              to detect them here.  Given the number of interrupts that can
 *              come through this code, it would be good if we could route a
 *              separate interrupt controller/number to reduce the latencies.
 *
 *              We should be able to check the status of the INTCFG_IO bit in
 *              register #1f but it doesn't seem to be set on the boards I've
 *              tested until now.
 *
 ******************************************************************************/
irqreturn_t ioboard_80_1003_irq_handler(struct dpci_device *ddp, u8 reg0)
{
	irqreturn_t int_status;

	int_status = ip_irq_handler(ddp);
	if (int_status == IRQ_NONE)
		PRINT_IRQ("not handled\n");
	return int_status;
}


/******************************************************************************
 *
 * Function:    ioboard_80_0062_irq_handler
 *
 * Parameters:  none.
 *
 * Returns:     Nothing.
 *
 * Description: Handles an interrupt from the I/O board.
 *              COM port interrupts are handled automatically so we don't need
 *              to detect them here.  Given the number of interrupts that can
 *              come through this code, it would be good if we could route a
 *              separate interrupt controller/number to reduce the latencies.
 *
 ******************************************************************************/
irqreturn_t ioboard_80_0062_irq_handler(struct dpci_device *ddp, u8 reg0)
{
	int int_status;
	irqreturn_t irq_status = IRQ_NONE;

	PRINT_IRQ("ddp=%p\n", ddp);
	int_status = dpci_io_readbyte(ddp, DPCI_IOBOARD_INTSTAT);
	if (int_status & DPCI_IOBOARD_INTCFG_IO)
	{
		irq_status |= ip_irq_handler(ddp);
	}
	if (int_status & DPCI_IOBOARD62_INTCFG_TS)
	{
		irq_status |= ts_irq_handler(ddp);
	}
	if (irq_status == IRQ_NONE)
		PRINT_IRQ("not handled\n");
	return irq_status;
}


/******************************************************************************
 *
 * Function:    oneboard_irq_handler
 *
 * Parameters:  none.
 *
 * Returns:     IRQ_HANDLED or IRQ_NONE.
 *
 * Description: Handles an interrupt for one-board solutions such as the 116 or
 *              112.
 *
 ******************************************************************************/
irqreturn_t oneboard_irq_handler(struct dpci_device *ddp, u8 reg0)
{
	irqreturn_t irq_status = IRQ_NONE;
	int ioint = 0, exint = 0;

	PRINT_IRQ("ddp=%p\n", ddp);

	/*
	 * Revision 45 firmware (DPX116U) has a bug such that the IOINT bit
	 * never gets set; in this event we simply call the interrupt handler
	 * anyway.
	 */
	if (ddp->features & HAVE_EXINT0)
	{
		/*
		 * DPX-S410/S305 only has EXINT on bit 0.
		 */
		if (reg0 & DPCI_REG0_IOINT)
		{
			ioint++;
		}
		if (reg0 & DPCI_REG0_EXINT0)
		{
			exint++;
		}
	}
	else if ((ddp->features & HAVE_EXINTEN) &&
		(reg0 & DPCI_REG0_EXINT))
	{
		/*
		 * DPX-112 only has EXINTEN for on-board I/O.
		 */
		if (ddp->features & HAVE_IOINTONEXINT)
		{
			ioint++;
		}
		else
		{
			exint++;
		}
	}
	else if (reg0 & DPCI_REG0_IOINT ||
			ddp->revision == 0x45 ||
			ddp->revision == 0x61)
	{
		/*
		 * Other boards have IOINT for off-board or on-board Digital IO
		 */
		ioint++;
	}
#if 0
	else
	{
		dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_IOINTEN, 0);
		PRINT_ERR("Unexpected IO interrupt.\n");
		PRINT_ERR("The system hardware may be disabled or "
			"damaged or a software bug exists.\n");
		PRINT_ERR("Please report this error to "
			"support@advantech-innocore.com\n");
		irq_status = IRQ_HANDLED;
	}
#endif

	if (ioint)
	{
		irq_status |= ip_irq_handler(ddp);
		localio_interrupts++;
	}
	if (exint && (dpci_reg_get_saved(ddp, DPCI_REG0) & DPCI_REG0_EXINTEN))
	{
		exio_interrupts++;
		if (ddp->exint_handler)
			ddp->exint_handler(ddp, ddp->exint_handler_data);
		irq_status = IRQ_HANDLED;
	}
	if (irq_status == IRQ_NONE)
		PRINT_IRQ("not handled\n");
	return irq_status;
}


/******************************************************************************
 *
 * Function:    multiboard_irq_handler
 *
 * Parameters:  none.
 *
 * Returns:     IRQ_HANDLED or IRQ_NONE.
 *
 * Description: Handles an interrupt for two/three-board solutions such as the
 *              DPX-114/DPX-116/DPX-117 which can take an active back-plane or
 *              passive backplane + I/O board.
 *
 ******************************************************************************/
irqreturn_t multiboard_irq_handler(struct dpci_device *ddp, u8 reg0)
{
	irqreturn_t irq_status = IRQ_NONE;

	PRINT_IRQ("ddp=%p\n", ddp);

	/*
	 * See if it's an off board interrupt.
	 */
	if (reg0 & DPCI_REG0_IOINT)
	{
		exio_interrupts++;

		/*
		 * This covers I/O Board I/II expansion.
		 */
		if (ddp->ioboard && ddp->ioboard->db_irq_handler)
		{
			irq_status = IRQ_HANDLED;
			ddp->ioboard->db_irq_handler(ddp, reg0);
		}
		else
		{
			dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_IOINTEN, 0);
			PRINT_ERR("Unexpected IO interrupt.\n");
			PRINT_ERR("The system hardware may be disabled or "
				"damaged or a software bug exists.\n");
			PRINT_ERR("Please report this error to "
				"support@advantech-innocore.com\n");
		}
	}

	if (irq_status == IRQ_NONE)
		PRINT_IRQ("not handled\n");
	return irq_status;
}


/*******************************************************************************
 *
 * Function:    irq_handler
 *
 * Parameters:  irq - the interrupt line being handled
 *              dev_id - the device to which the irq relates (from request)
 *		regs - registers at interrupt time.
 *
 * Returns:     if device in use - IRQ_HANDLED.
 *              if device not in use - IRQ_NONE.
 *
 * Description: Handles an interrupt from the DPCI chip.  We check whence the
 *		interrupt originated and call either or both the PIC and IO
 *		interrupt handlers.
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
irqreturn_t irq_handler(int irq, void *dev_id)
#else
irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
	int int_status;
	irqreturn_t irq_status = IRQ_NONE;
	struct dpci_device *ddp = dev_id;

	all_interrupts++;
	int_status = dpci_reg_get(ddp, DPCI_REG0);
	PRINT_IRQ("REG0=0x%02x\n", int_status);
	if ((int_status & DPCI_REG0_LOGINT) &&
		(dpci_reg_get_saved(ddp, DPCI_REG0) & DPCI_REG0_PICINTEN))
	{
		irq_status = IRQ_HANDLED;
		idlp_event_interrupts++;
		idlp_event_irq_handler(ddp);
	}
	if ((ddp->features & HAVE_IDLPPLUS) &&
		(dpci_reg_get_saved(ddp, DPCI_REG0) & DPCI_REG0_ACKIEN) &&
		(int_status & DPCI_REG0_PICACK))
	{
		irq_status = IRQ_HANDLED;
		idlp_ack_interrupts++;
		idlp_ack_irq_handler(ddp);
	}
	if ((ddp->features & HAVE_POWERFAILDETECT) &&
		(dpci_reg_get_saved(ddp, DPCI_REG0) & DPCI_REG0_PFDIEN) &&
		((int_status & DPCI_REG0_PFD) == 0))
	{

		irq_status = IRQ_HANDLED;
		pfd_interrupts++;
		dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_PFDIEN, 0);
		ddp->thread_event.op_event.de_type |= EVENT_PFD;
		ddp->thread_event.er_signal = SIGNAL_EVENT;
		if (waitqueue_active(&ddp->thread_event.er_wqh))
		{
			wake_up_interruptible(&ddp->thread_event.er_wqh);
		}

		PRINT_IRQ("Power fail detect interrupt\n");
	}

	/*
	 * If the system board has its own interrupt handler then use that only.
	 */
	if (ddp->mainboard->db_irq_handler)
	{
		irq_status |= ddp->mainboard->db_irq_handler(ddp, int_status);
	}

	if (irq_status == IRQ_NONE)
	{
		PRINT_IRQ("not handled\n");
		unassigned_interrupts++;
	}
	return irq_status;
}


/*******************************************************************************
 *
 * Function:    dpci_get_ioboard_intcfg()
 *
 * Parameters:  none
 *
 * Returns:     u8 - the intcfg byte for the iocard (reg 1f)
 *
 * Description: Returns the interrupt configuration byte for the IO board.
 *
 ******************************************************************************/
u8 dpci_get_ioboard_intcfg(struct dpci_device *ddp)
{
	return dpci_reg_get(ddp, DPCI_IOBOARD_INTCFG);
}


/*******************************************************************************
 *
 * Function:    dpci_set_ioboard_intcfg()
 *
 * Parameters:  u8 set - bit mask of bits to set
 *		u8 clear - bit mask of bits to clear
 *
 * Returns:     u8 - the intcfg byte for the iocard (reg 1f)
 *
 * Description: Modifies the intcfg setting (register 1f) by ORing in the bits
 *		set in 'set' and clearing the bits set in 'clear'.  It returns
 *		the new setting.
 *
 ******************************************************************************/
u8 dpci_set_ioboard_intcfg(struct dpci_device *ddp, u8 set, u8 clear)
{
	return dpci_reg_chg(ddp, DPCI_IOBOARD_INTCFG, set, clear, 0);
}


/*******************************************************************************
 *
 * Function:    dpci_reg_set
 *
 * Parameters:  reg - the register to write
 *              u8 val - the value to set
 *
 * Returns:     u8 - the new setting byte for the register
 *
 * Description: Sets a given registers using the supplied val.  It returns the
 *              new setting.
 *
 ******************************************************************************/
u8 dpci_reg_set(struct dpci_device *ddp, dpci_off_t reg, u8 val)
{
	unsigned long flags;

	PRINT_REG0("(%p, 0x%x, 0x%x)\n", ddp, reg, val);
	
	spin_lock_irqsave(&ddp->val_regs_lock, flags);
	ddp->val_regs[reg] = val;
	dpci_io_writebyte(ddp, reg, ddp->val_regs[reg]);
	spin_unlock_irqrestore(&ddp->val_regs_lock, flags);
	return val;
}


/*******************************************************************************
 *
 * Function:    dpci_reg_get
 *
 * Parameters:  reg - the register to read
 *
 * Returns:     u8 - the value of the register.
 *
 * Description: Returns the actual value read back from the machine register.
 *
 ******************************************************************************/
u8 dpci_reg_get(struct dpci_device *ddp, dpci_off_t reg)
{
	return dpci_io_readbyte(ddp, reg);
}


/*******************************************************************************
 *
 * Function:    dpci_reg_get_saved
 *
 * Parameters:  reg - the register to read
 *
 * Returns:     u8 - the cached register value last written to the register.
 *
 * Description: Returns the cached value read back from storage
 *
 ******************************************************************************/
u8 dpci_reg_get_saved(struct dpci_device *ddp, dpci_off_t reg)
{
	return ddp->val_regs[reg];
}


/*******************************************************************************
 *
 * Function:    dpci_reg_chg
 *
 * Parameters:  reg - the register to write
 *              u8 set - bit mask of bits to set
 *		u8 clear - bit mask of bits to clear
 *		u8 xor - bit mask of bits to toggle
 *
 * Returns:     u8 - the new setting byte for the register
 *
 * Description: Modifies a given registers setting by ORing in the bits set in
 *		'set' and clearing the bits set in 'clear'.  It returns the new
 *		setting.  Bits are cleared first before setting.
 *
 ******************************************************************************/
u8 dpci_reg_chg(struct dpci_device *ddp, dpci_off_t reg, u8 set, u8 clear, u8 xor)
{
	u8 oldval, ret;
	unsigned long flags;

	spin_lock_irqsave(&ddp->val_regs_lock, flags);
	oldval = ddp->val_regs[reg];
	ddp->val_regs[reg] &= ~clear;
	ddp->val_regs[reg] |= set;
	ddp->val_regs[reg] ^= xor;
	ret = ddp->val_regs[reg];
	dpci_io_writebyte(ddp, reg, ddp->val_regs[reg]);
	PRINT_REG0("(%p, 0x%x, 0x%x, 0x%x, %02x): 0x%02x -> 0x%02x\n",
			ddp, reg, set, clear, xor, oldval, ddp->val_regs[reg]);
	spin_unlock_irqrestore(&ddp->val_regs_lock, flags);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_enable_ioboard_intr()
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Enables I/O board interrupts.  A count of requests is
 *              maintained.  Interrupts are only enabled on the first request.
 *
 ******************************************************************************/
void dpci_enable_ioboard_intr(struct dpci_device *ddp)
{
	if (!ddp->iobd_users++)
	{
		dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_IOINTEN, 0, 0);
	}
}


/*******************************************************************************
 *
 * Function:    dpci_disable_ioboard_intr()
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Disable I/O board interrupts.  Interrupts are only disabled on
 *		the request counter reaching zero again.
 *
 ******************************************************************************/
void dpci_disable_ioboard_intr(struct dpci_device *ddp)
{
	if (!--ddp->iobd_users)
	{
		dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_IOINTEN, 0);
	}
}


/*******************************************************************************
 *
 * Function:    dpci_io_readbyte()
 *
 * Parameters:  offset - the offset in I/O space from which to read
 *
 * Returns:     u8 - the data obtained.
 *
 * Description: Reads I/O memory at the specified address in DPCI I/O space and
 *		returns the data thus obtained.  If an I/O error occurs (PCI
 *		master or target aborts etc.) then we won't find out about it.
 *
 ******************************************************************************/
u8 dpci_io_readbyte(struct dpci_device *ddp, dpci_off_t offset)
{
	u8 byte;

	byte = inb(ddp->address + offset);
	PRINT_RA("read 0x%02x from offs 0x%02x\n", byte, offset);
	return byte;
}


/*******************************************************************************
 *
 * Function:    dpci_io_readword()
 *
 * Parameters:  offset - the offset in I/O space from which to read
 *
 * Returns:     u16 - the data obtained.
 *
 * Description: Reads I/O memory at the specified address in DPCI I/O space and
 *		returns the data thus obtained.  If an I/O error occurs (PCI
 *		master or target aborts etc.) then we won't find out about it.
 *
 ******************************************************************************/
u16 dpci_io_readword(struct dpci_device *ddp, dpci_off_t offset)
{
	u16 word;

	word = inw(ddp->address + offset);
	PRINT_RA("read 0x%04x from offs 0x%02x\n", word, offset);
	return word;
}


/*******************************************************************************
 *
 * Function:    dpci_io_readdword()
 *
 * Parameters:  offset - the offset in I/O space from which to read
 *
 * Returns:     u16 - the data obtained.
 *
 * Description: Reads I/O memory at the specified address in DPCI I/O space and
 *		returns the data thus obtained.  If an I/O error occurs (PCI
 *		master or target aborts etc.) then we won't find out about it.
 *
 ******************************************************************************/
u32 dpci_io_readdword(struct dpci_device *ddp, dpci_off_t offset)
{
	u32 dword;

	dword = inl(ddp->address + offset);
	PRINT_RA("read 0x%08x from offs 0x%02x\n", dword, offset);
	return dword;
}


/*******************************************************************************
 *
 * Function:    dpci_io_writebyte()
 *
 * Parameters:  offset - the offset in I/O space to write
 *		byte - the data to write
 *
 * Returns:     0 (zero).
 *
 * Description: Writes to I/O memory in the DPCI's I/O space.  The requested
 *		8-bit data is placed at the requested offset.
 *
 ******************************************************************************/
u8 dpci_io_writebyte(struct dpci_device *ddp, dpci_off_t offset, u8 byte)
{
	outb(byte, ddp->address + offset);
	PRINT_RA(": wrote 0x%02x to offs 0x%02x\n", byte, offset);
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_io_writeword()
 *
 * Parameters:  offset - the offset in I/O space to write
 *		word - the 16-bit data to write
 *
 * Returns:     0 (zero).
 *
 * Description: Writes to I/O memory in the DPCI's I/O space.  The requested
 *		16-bit data is placed at the requested offset.  Note that
 *		hardware limitations may make word-sized I/O access ineffective
 *		and that incorrect data can be written.
 *
 ******************************************************************************/
u16 dpci_io_writeword(struct dpci_device *ddp, dpci_off_t offset, u16 word)
{
	outw(word, ddp->address + offset);
	PRINT_RA("wrote 0x%04x to offs 0x%02x\n", word, offset);
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_io_writedword()
 *
 * Parameters:  offset - the offset in I/O space to write
 *		dword - the data to write
 *
 * Returns:     0 (zero).
 *
 * Description: Writes to I/O memory in the DPCI's I/O space.  The requested
 *		32-bit data is placed at the requested offset.  Note that
 *		hardware limitations may make word-sized I/O access ineffective
 *		and that incorrect data can be written.
 *
 ******************************************************************************/
u32 dpci_io_writedword(struct dpci_device *ddp, dpci_off_t offset, u32 dword)
{
	outl(dword, ddp->address + offset);
	PRINT_RA("wrote 0x%08x to offs 0x%02x\n", dword, offset);
	return 0;
}

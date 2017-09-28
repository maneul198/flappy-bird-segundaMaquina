/******************************************************************************
 *
 * $Id: dpci_serial.c 11906 2015-09-07 15:39:38Z aidan $
 *
 * Copyright 2005-2015 Advantech Co Ltd.
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
 * DirectPCI kernel-mode driver for serial I/O.  This is just a wrapper which
 * points the 8250 driver at the correct places in i/o memory.
 *
 *****************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/serial_8250.h>

#include <linux/dpci_multi.h>
#include "dpci_version.h"
#include "dpci_core_hw.h"

#define	SERIAL_UART_HZ		1843200


/*
 * For some reasons, these aren't in serial_8250.h.
 */
int serial8250_register_port(struct uart_port *);
void serial8250_unregister_port(int line);


/*
 * Support for debugging output.
 */
#define DPCI_DEVICE_NAME "dpci_serial"
#define PRINT_INFO(...) printk(KERN_INFO DPCI_DEVICE_NAME ": " __VA_ARGS__)
#define PRINT_ERR(...) printk(KERN_ERR DPCI_DEVICE_NAME ": " __VA_ARGS__)
#define PRINT_WARN(...) printk(KERN_WARNING DPCI_DEVICE_NAME ": " __VA_ARGS__)
int debug = 0;
#ifdef DEBUG
# define LOG_LEVEL	KERN_INFO
# define PRINT_DBG(fmt, ...) \
	{ \
		if (debug) \
		{ \
			printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
		} \
	}
#else
# define PRINT_DBG(...)
#endif

static int dpci_address = 0;
static int n_uarts;
static dpci_off_t uarts[MAX_UARTS];

static struct dpci_serial_port
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
	struct uart_8250_port up8250;
#else
	struct uart_port up;
#endif
	int line;
} dpci_serial_ports[MAX_UARTS];

static int __init
serial_init(void)
{
	int i;
	struct dpci_device *ddp;
	struct uart_port *up;

	PRINT_INFO("Advantech Innocore DirectPCI serial driver "
		"v%s $Revision: 11906 $\n",
		DPCI_VERSION); 
	PRINT_INFO("Advantech Innocore DirectPCI serial driver "
		"compiled " B__DATE__ ", " B__TIME__ "\n");
	PRINT_INFO("(C) 2005-2015, Advantech Co Ltd.\n");
	PRINT_DBG("debug=%d\n", debug);
#ifndef DEBUG
	if (debug)
	{
		PRINT_WARN("debug=0x%x but this is not a DEBUG driver: "
				"please load the debug driver if you want debug output.\n", debug);
	}
#endif

	ddp = dpci_get_device(0);
	if (!ddp)
	{
		printk(KERN_ERR
			"DirectPCI base I/O not available\n");
		return -ENODEV;
	}

	dpci_address = dpci_get_ioaddress(ddp);
	n_uarts = dpci_get_uart_offsets(ddp, uarts, sizeof(uarts));
	if (n_uarts == 0) {
		PRINT_ERR("no DirectPCI serial devices: "
			"check I/O board is inserted\n");
		return -ENODEV;
	}

	memset((void *)dpci_serial_ports, 0, sizeof(dpci_serial_ports));
	for (i = 0; i < n_uarts; i++)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
		up = &dpci_serial_ports[i].up8250.port;
		dpci_serial_ports[i].up8250.mcr_mask = ~0;
#else
		up = &dpci_serial_ports[i].up;
#endif
		/*
		 * Set up the port configuration.
		 */
		up->flags = UPF_SKIP_TEST | UPF_BOOT_AUTOCONF | UPF_SHARE_IRQ;
		up->irq = dpci_get_irq(ddp);
		up->uartclk = SERIAL_UART_HZ;
		up->iotype = UPIO_PORT;
		up->iobase = dpci_address + uarts[i];

		/*
		 * Register the port.
		 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
		dpci_serial_ports[i].line =
				serial8250_register_8250_port(&dpci_serial_ports[i].up8250);
#else
		dpci_serial_ports[i].line =
				serial8250_register_port(&dpci_serial_ports[i].up);
#endif
		if (dpci_serial_ports[i].line < 0)
		{
			PRINT_ERR("register serial port #%d at 0x%04x failed - error %d\n",
				i, 
				dpci_address + uarts[i],
				dpci_serial_ports[i].line);
			break;
		}
		PRINT_DBG("serial_init: registered port#%d at 0x%04x.\n",
			i,
			dpci_address + uarts[i]); 
	}

	/*
	 * If we couldn't install ALL the ports, then uninstall the ones we
	 * could and back all the way out.
	 */
	if (i != n_uarts)
	{
		while (i > 0)
		{
			i--;
			PRINT_DBG("serial_exit: unregistered port#%d at 0x%04x.\n", i,
				dpci_address + uarts[i]); 
			serial8250_unregister_port(dpci_serial_ports[i].line);
		}
		return -EBUSY;
	}
	dpci_enable_ioboard_intr(ddp);

	/*
	 * Enable COM interrupts.  This works for both 80-1003 and 80-0062
	 * I/O boards.
	 */
	dpci_set_ioboard_intcfg(ddp, DPCI_IOBOARD03_INTCFG_COMWD, 0);

	PRINT_DBG("Advantech Innocore DirectPCI serial driver loaded.\n");
	return 0;
}


static void __exit
serial_exit(void)
{
	int i;
	struct dpci_device *ddp;

	ddp = dpci_get_device(0);
	dpci_disable_ioboard_intr(ddp);
	dpci_set_ioboard_intcfg(ddp, 0, DPCI_IOBOARD03_INTCFG_COMWD);
	PRINT_DBG("serial_exit: disabled interrupts.\n"); 
	for (i = 0; i < n_uarts; i++)
	{
		PRINT_DBG("serial_exit: unregistered port#%d at 0x%04x.\n", i,
			dpci_address + uarts[i]); 
		serial8250_unregister_port(dpci_serial_ports[i].line);
	}

	PRINT_DBG("Advantech Innocore DirectPCI serial driver unloaded.\n"); 
	return;
}

MODULE_LICENSE("GPL");
module_init(serial_init);
module_exit(serial_exit);
MODULE_PARM_DESC(debug, "determines level of debugging output.");
module_param(debug, int, 0);

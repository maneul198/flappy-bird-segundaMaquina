/******************************************************************************
 *
 * $Id: dpci_core_idlp.c 12812 2016-09-05 11:59:41Z aidan $
 *
 * Copyright 2003-2014 Advantech Corporation Limited.
 * All rights reserved.
 *
 * License:	Advantech Innocore EULA
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
 * Advantech Innocore DirectPCI core module for kernel-mode
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

#ifdef IDLP_PSW
#include "idlppwd.h"
#endif

/*include the file that defines the constants for the dpci-core driver.*/

#include <linux/dpci_core_ioctl.h>
#include "dpci_core_hw.h"
#include "dpci_core_priv.h"

/*
 * Configuration flags
 *
 * INTERRUPT_DRIVEN_COMMS: if defined, all IDLP communication is driven from
 * the interrupt handler and polling is done solely for resynchronisation.
 *
 * IDLPPLUS_ACK_INTERRUPTS: this is half-polling mode.  Interrupts are used to
 * wake up the userland task when a byte (or the opportunity to write one) is
 * ready.  This is not an efficient mode to work in because the latency of
 * waking the task each time a byte comes in can be significant.
 *
 * If neither of the above is set then no interrupts are used and the driver
 * always waits around for the PICACK bit to be set.  This mode is not efficient
 * either but less inefficient that setting IDLPPLUS_ACK_INTERRUPTS.
 */
#define INTERRUPT_DRIVEN_COMMS
#undef IDLPPLUS_ACK_INTERRUPTS
#ifdef INTERRUPT_DRIVEN_COMMS
# undef IDLPPLUS_ACK_INTERRUPTS
#endif

/*
 * This is the time-out to wait for PICACK to go high in between a byte having
 * been read or written and the next opportunity to do so.  This is revised in
 * accordance with the calculation for IDLPPLUS_CMD_TIMEOUT_MS below, with the
 * longest command (select event, read event) taking 8 ms and requiring a data
 * exchange of 11 bytes over two commands - including waiting for the IDLP to
 * access its event storage EEPROM.
 */
#define IDLPPLUS_ACK_TIMEOUT_US	8000

/*
 * This is the total time for a command when INTERRUPT_DRIVEN_COMMS is defined.
 * In other cases, the per-byte time-out is in effect, which is basically the
 * same as IDLPPLUS_ACK_TIMEOUT_US.
 *
 * This time is based on the time taken for a DPX-S410 to read a full event log
 * using the select event API.  It took 1.43s of elapsed time for 255 events -
 * about 8ms for each event.  This is the slowest operation the IDLP can do -
 * even reading the entire IDPROM is quicker than this.  So, we can safely set
 * the timeout at about 20 milliseconds.
 */
#define IDLPPLUS_CMD_TIMEOUT_MS	20

/*
 * Event names array
 */
static DECLARE_EVENT_NAME_ARRAY(event_names);

int dpci_id_fwversion(struct dpci_device *ddp, unsigned int *fvp);
static int idlp_plus_command(struct dpci_device *, u8 , u8 *, int , u8 *, int);
static int idlp_plus_resync(struct dpci_device *ddp);
static int idlp_spi_command(struct dpci_device *, u8 , u8 *, int , u8 *, int);
static int idlp_spi_resync(struct dpci_device *ddp);

static int bcd_to_int(unsigned char *bcd, int bytes)
{
	int i = 0;

	while (bytes--)
	{
		i *= 100;
		i += (*bcd & 0xf) + (10 * ((*bcd) >> 4));
		bcd++;
	}
	return i;
}


static int int_to_bcd(int i, unsigned char *bcd, int bytes)
{
	int ret = 0;
	int b;

	for (b = bytes - 1; b >= 0; b--)
	{
		bcd[b] = i % 10;
		i /= 10;
		bcd[b] += 16 * (i % 10);
		i /= 10;
	}
	if (i)
		ret = -E2BIG;

	return ret;
}


static int ascii_to_int(unsigned char *ascii, int bytes)
{
	int i = 0;

	while (bytes--)
	{
		i *= 10;
		i += *ascii - '0';
		ascii++;
	}
	return i;
}


/*******************************************************************************
 *
 * Function:    dpci_id_setup()
 *
 * Parameters:  dev - the device being setup
 *
 * Returns:	nothing
 *
 * Description: Initialises resources for PIC access.
 *		We ensure the correct interrupts are enabled.
 *
 ******************************************************************************/
void dpci_id_setup(struct dpci_device *ddp)
{
	int error;

	/*
	 * Set up the PIC variables and log which version of the logging
	 * processor is in use.  Ensure the PIC starts in a reliable state:
 	 * set the slave select line high and disable PIC interrupts.
	 */
	init_MUTEX(&ddp->idlp_cmd_sem);
	spin_lock_init(&ddp->idlp_buf_spinlock);
	init_waitqueue_head(&ddp->idlp_data_wqh);
	if (ddp->features & HAVE_IDLPPLUS)
	{
		ddp->idlp_command = idlp_plus_command;
		if (ddp->features & HAVE_IDLP_QUICK)
		{
#ifdef DEBUG
			PRINT_INFO("IDLP-comms driver in quick mode.\n");
#endif
			ddp->idlpplus_mode = IDLP_PLUS_QUICK;
		}
		else
		{
			ddp->idlpplus_mode = IDLP_PLUS_INTR;
		}
		idlp_plus_resync(ddp);
	}
	else
	{
		ddp->idlp_command = idlp_spi_command;
		dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_SPISS, 0, 0);
		idlp_spi_resync(ddp);
	}
	dpci_id_intdisable(ddp);
	error = dpci_id_fwversion(ddp, &ddp->idlp_fwversion);
	if (error < 0)
	{
		PRINT_ERR("error %d talking to logging processor\n", -error);
	}
	else
	{
		if (ddp->idlp_fwversion < 0x100) // pre v54 firmware
		{
			PRINT_INFO("Intrusion detection/logging "
				"processor firmware rev. %d\n",
				ddp->idlp_fwversion);
		}
		else
		{
			PRINT_INFO("Intrusion detection/logging "
				"processor firmware rev. %d.%d (%d.%d)\n",
				IDLP_VERSION_MAJOR(ddp->idlp_fwversion),
				IDLP_VERSION_BUILD(ddp->idlp_fwversion),
				IDLP_VERSION_CONF1(ddp->idlp_fwversion),
				IDLP_VERSION_CONF2(ddp->idlp_fwversion));
		}
	}
	ddp->idevent_temp_flag = 0;
}


/*******************************************************************************
 *
 * Function:    dpci_id_cleanup()
 *
 * Parameters:  dev - the device being cleaned up
 *
 * Returns:	nothing
 *
 * Description: Shuts down the idlp and frees any claimed resources.
 *
 ******************************************************************************/
void dpci_id_cleanup(struct dpci_device *ddp)
{
	dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_PICINTEN, 0);
	dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_ACKIEN, 0);
	dpci_id_intdisable(ddp);
}


/*******************************************************************************
 *
 * Function:    idlp_spi_xferbyte
 *
 * Parameters:  data - the byte of data to send
 *
 * Returns:     u8 - the byte of data recieved from the slave
 *
 * Description: Sends a byte of data on the serial peripheral interconnect bus,
 *              at the same time clocking in a return byte from the same bus.
 *              Callers MUST take the ddp->idlp_cmd_sem mutex before calling.
 *
 ******************************************************************************/
static u8 idlp_spi_xferbyte(struct dpci_device *ddp, u8 data)
{
	int i = 0;
	u8 input = 0;
	u8 set, clear;
	unsigned long flags;

	local_irq_save(flags);
	local_irq_disable();
	dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_SPISS, 0, 0);
	dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_SPISS, 0);
	for (i = 128; i; i >>= 1)
	{
		/*
		 * Prepare output data.  Output the data to the port twice so
		 * the minimum delay required by the SPI slave is guaranteed.
		 */
		set = DPCI_REG0_SPISCK;
		if (data & i)
		{
			  set |= DPCI_REG0_SPISDO;
			  clear = 0;
		}
		else
		{
			  clear = DPCI_REG0_SPISDO;
		}
		dpci_reg_chg(ddp, DPCI_REG0, set, clear, 0);
		dpci_reg_chg(ddp, DPCI_REG0, set, clear, 0);
		dpci_reg_chg(ddp, DPCI_REG0, set, clear, 0);

		/*
		 * Bring clock low again.
		 */
		dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_SPISCK, 0);

		/*
		 * Shift in the data from the input line.  We can OR in the
		 * SPISDI bit directly because it's value is 1.
		 */
		input <<= 1;
		input |= (dpci_reg_get(ddp, DPCI_REG0) & DPCI_REG0_SPISDI);

		/*
		 * Write the clock-low word again for delay purposes.
		 */
		dpci_reg_chg(ddp, DPCI_REG0, 0, 0, 0);
	}

	PRINT_IDLP("output=%02x input=%02x\n", data, input);

	/*
	 * Bring SPISS high again.
	 */
	dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_SPISS, 0, 0);
	local_irq_restore(flags);

	/*
	 * Sleep for the minimum delay after each byte.  It's a long delay so
	 * don't use usleep as we keep all other tasks & interrupts from doing
	 * anything.  We don't use an interruptible time-out here beacuse that
	 * might hang the PIC or cause other commands to sent afterwards to be
	 * out of sync.
	 */
	wait_event_timeout(ddp->idlp_data_wqh, 0, SPI_BYTE_DELAY_MS);

	return input;
}


/*******************************************************************************
 *
 * Function:    idlp_spi_resync
 *
 * Parameters:  none
 *
 * Returns:     int - 0 for success, -EIO for failure.
 *
 * Description: sends 0xaa to the IDLP until the data comes back 0xaa at least
 *              MIN_IDLE_REPLIES times.  Returns 0 onces 0xaa starts coming back
 *              or -EIO if it never does within MAX_NONEIDLEREPLIES bytes.
 *
 ******************************************************************************/
static int idlp_spi_resync(struct dpci_device *ddp)
{
#define SPI_MIN_IDLE_REPLIES	5
#define SPI_MAX_NONDILE_REPLIES	20
	int i, ni;
	u8 byte;

	PRINT_IDLP("starting\n");
	for (i = 0, ni = 0; i < SPI_MIN_IDLE_REPLIES && ni < SPI_MAX_NONDILE_REPLIES; )
	{
		byte = idlp_spi_xferbyte(ddp, DPCI_IDLP_ACKNOWLEDGE);
		if (byte == DPCI_IDLP_ACKNOWLEDGE)
		{
			i++;
		}
		else
		{
			ni++;
			i = 0;
		}
	}
	if (ni == SPI_MAX_NONDILE_REPLIES)
	{
		PRINT_IDLP("not idle after %d bytes.\n", ni);
		return -EIO;
	}
	PRINT_IDLP("finally idle after %d bytes.\n", ni + i);
	return 0;
}


/*******************************************************************************
 *
 * Function:    idlp_spi_command
 *
 * Parameters:  cmd - the IDLP command to execute
 *              tdata - pointer to bytes to send.
 *              tlen - the number of bytes to send.
 *              rdata - pointer to bytes to receive.
 *              rlen - the number of bytes to receive.
 *
 * Returns:     the number of bytes unsent.
 *
 * Description: Sends a string of bytes on the SPI bus and checks the return
 *              data is always 0xaa.  If a byte returned is not 0xaa then the
 *              functions breaks off.  Hence a return of zero indicates all data
 *              was sent successfully.
 *
 ******************************************************************************/
static int idlp_spi_command(struct dpci_device *ddp, u8 cmd, u8 *tdata, int tlen, u8 *rdata, int rlen)
{
	int byteno;
	int ret = -EIO;
	int attempts;
	u8 byte;

	if (down_interruptible(&ddp->idlp_cmd_sem) != 0)
	{
		return -ERESTARTSYS;
	}
	for (attempts = 0; attempts < SPI_COMMAND_MAX_TRIES; attempts++)
	{
		PRINT_IDLP("command 0x%x attempt %d\n",
			cmd, attempts + 1);
		byte = idlp_spi_xferbyte(ddp, DPCI_IDLP_ACKNOWLEDGE);
		if (byte != DPCI_IDLP_ACKNOWLEDGE)
		{
			PRINT_IDLP("command 0x%x, attempt %d: return data "
				"(lead byte) 0x%02x not 0xaa\n",
				cmd, attempts + 1, byte);
			goto next;
		}
		if (idlp_spi_xferbyte(ddp, cmd) != DPCI_IDLP_ACKNOWLEDGE)
		{
			PRINT_IDLP("command 0x%x, attempt %d: return data "
				"(cmd byte) 0x%02x not 0xaa\n",
				cmd, attempts + 1, byte);
			goto next;
		}
		for (byteno = 0; byteno < tlen; byteno++)
		{
			if ((byte = idlp_spi_xferbyte(ddp, tdata[byteno])) != DPCI_IDLP_ACKNOWLEDGE)
			{
				PRINT_ERR("idlp_spi_command: "
					"command 0x%x, attempt %d: return data "
					"(byte %d) 0x%02x not 0xaa\n",
					cmd, attempts + 1, byteno, byte);
				goto next;
			}
		}
		for (byteno = 0; byteno < rlen; byteno++)
		{
			rdata[byteno] = idlp_spi_xferbyte(ddp, DPCI_IDLP_ACKNOWLEDGE);
			if (rdata[byteno] < 0x30 || rdata[byteno] > 0x39)
			{
				PRINT_ERR("idlp_spi_command: "
					"command 0x%x, attempt %d: return data "
					"(byte %d) 0x%02x not 0x30-0x39\n",
					cmd, attempts + 1, byteno,
					rdata[byteno]);
				goto next;
			}
		}
#if 0
		/*
		 * Send a final 0xaa to the PIC to see if it throws anything
		 * else back.  This is broken as of firmware rev.25 for IDLPs
		 * with an external eeprom for event log storage because the
		 * command to set the date renders the IDLP incommunicado for
		 * some 10-15ms.
		 */
		if (idlp_spi_xferbyte(ddp, DPCI_IDLP_ACKNOWLEDGE) != DPCI_IDLP_ACKNOWLEDGE)
		{
			goto next;
		}
#endif
		ret = 0;
		break;
next:
		wait_event_timeout(ddp->idlp_data_wqh, 0, SPI_COMMAND_DELAY_MS);
		if (attempts >= SPI_COMMAND_MAX_TRIES / 2)
		{
			idlp_spi_resync(ddp);
		}
	}
	up(&ddp->idlp_cmd_sem);
	PRINT_IDLP("command 0x%02x, %d attempts, ret=%d\n",
			cmd, attempts + 1, ret);
	return ret;
}


/*******************************************************************************
 *
 * Function:    idlp_plus_waitack
 *
 * Parameters:  ddp - DPCI device pointer
 *
 * Returns:     0 or -ve error code.
 *
 * Description: Waits for the PICACK bit to go high indicating the IDLP has
 * 		acknowledged a data byte sent and is ready to send or receive
 * 		the next byte.
 *
 *              This function can work in one of two ways determined at compile
 *              time.  The simple way is to undefine IDLPPLUS_ACK_INTERRUPTS
 *              which means the code simply loops continuously until PICACK is
 *              high or it runs out of time (30ms).  The other way is to define
 *              IDLPPLUS_ACK_INTERRUPTS (the default) which causes the wait for
 *              PICACK to be interrupt driven; in this case the process doing
 *              the IDLP access is put to sleep temporarily and woken up when
 *              the interrupt comes in or the timeout elapses.  This is more
 *              efficient generally because it allows other processes to be
 *              given execution time.
 *
 ******************************************************************************/
static int idlp_plus_waitack(struct dpci_device *ddp)
{
	int ret = -EIO;
#ifndef IDLPPLUS_ACK_INTERRUPTS
# define WAITACK_CHECK_PERIOD_US	1
	int timeout = IDLPPLUS_ACK_TIMEOUT_US;

	while (timeout)
	{
		if (dpci_reg_get(ddp, DPCI_REG0) & DPCI_REG0_PICACK)
		{
			ret = 0;
			break;
		}
		udelay(WAITACK_CHECK_PERIOD_US);
		timeout -= WAITACK_CHECK_PERIOD_US;
	}
	if (timeout == 0)
	{
		PRINT_ERR("idlp_plus_waitack: timeout reading byte from IDLP.\n");
	}
#else
	if (dpci_reg_get(ddp, DPCI_REG0) & DPCI_REG0_PICACK)
	{
		return 0;
	}
	dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_ACKIEN, 0, 0);
	wait_event_interruptible_timeout(ddp->idlp_data_wqh,
			dpci_reg_get(ddp, DPCI_REG0) & DPCI_REG0_PICACK,
			usecs_to_jiffies(IDLPPLUS_ACK_TIMEOUT_US));
	if (dpci_reg_get(ddp, DPCI_REG0) & DPCI_REG0_PICACK)
	{
		ret = 0;
	}
#endif
	return ret;
}


/*******************************************************************************
 *
 * Function:    idlp_plus_txbyte
 *
 * Parameters:  ddp - the device whose IDLP+ is to be contacted
 *              byte - the 8-bit data to send
 *
 * Returns:     int - 0 for success, -EIO for failure.
 *
 * Description: transmits a byte to the IDLP+.  It is necessary to wait for
 *              the IDLP+ to indicate it's ready to receive data.
 *
 ******************************************************************************/
static int idlp_plus_txbyte(struct dpci_device *ddp, u8 byte)
{
	int ret = -EIO;

	ret = idlp_plus_waitack(ddp);
	if (ret != 0)
		goto done;
	dpci_reg_set(ddp, DPCI_REGIDLPDATA, byte);
	ret = 0;
	PRINT_IDLP("output=%02x\n", byte);
done:
	return ret;
}


/*******************************************************************************
 *
 * Function:    idlp_plus_rxbyte
 *
 * Parameters:  ddp - the device whose IDLP+ is to be contacted
 *
 * Returns:     int - >=0 for success, -EIO for failure.
 *
 * Description: receives a byte from the IDLP+.  It is necessary to wait for
 *              the IDLP+ to indicate it's sent data. 
 *
 ******************************************************************************/
static int idlp_plus_rxbyte(struct dpci_device *ddp)
{
	u8 data;
	int ret = -EIO;

	ret = idlp_plus_waitack(ddp);
	if (ret != 0)
		goto done;
	data = dpci_io_readbyte(ddp, DPCI_REGIDLPDATA);
	ret = data;
	PRINT_IDLP("input=%02x\n", ret);
done:
	return ret;
}


/*******************************************************************************
 *
 * Function:    idlp_plus_resync
 *
 * Parameters:  none
 *
 * Returns:     int - 0 for success, -EIO for failure.
 *
 * Description: sends 0xaa to the IDLP until the data comes back 0xaa at least
 *              MIN_IDLE_REPLIES times.  Returns 0 onces 0xaa starts coming back
 *              or -EIO if it never does within MAX_NONEIDLEREPLIES bytes.
 *
 ******************************************************************************/
static int idlp_plus_resync(struct dpci_device *ddp)
{
#define PSP_MAX_RESYNC_ATTEMPTS	3
#define PSP_MAX_INPUT_LENGTH	7  /* readevent returns 7 bytes. */
	int attempts;
	int data;
	int ret;
	int i;

	/*
	 * Now do the main part of the resync with PICACK-checking.
	 */
	PRINT_IDLP("starting\n");
	for (attempts = 0; attempts < PSP_MAX_RESYNC_ATTEMPTS; attempts++)
	{
		/*
		 * Now send an AAh byte to persuade the IDLP to give up any command it
		 * might already be processing.
		 */
		PRINT_IDLP("Flushing IDLP output queue\n");
		dpci_reg_set(ddp, DPCI_REGIDLPDATA, DPCI_IDLP_ACKNOWLEDGE);
		PRINT_IDLP("output=%02x\n", DPCI_IDLP_ACKNOWLEDGE);
		for (i = 0; i < PSP_MAX_INPUT_LENGTH; i++)
		{
			ret = idlp_plus_rxbyte(ddp);
			if (ret < 0)
			{
				/*
				 * If the return is <0 then a time-out occurred so we
				 * break out now.  Otherwise we carry on, regardless of
				 * the data received.
				 */
				break;
			}
		}

		/*
		 * If we got this far and we have a PICACK signal logged in reg 0,
		 * then we are probably on f/w v59 or earlier, where the IDLP will
		 * always return an AAh everytime we read the data register.
		 */
		PRINT_IDLP("Resynchronising\n");
		udelay(10000);
		if ((dpci_io_readbyte(ddp, DPCI_REG0) & DPCI_REG0_PICACK) != 0)
		{
			data = DPCI_IDLP_CMD_CLEAR_WD;
		}
		else
		{
			data = DPCI_IDLP_ACKNOWLEDGE;
		}
		dpci_reg_set(ddp, DPCI_REGIDLPDATA, data);
		PRINT_IDLP("output=%02x\n", data);
		ret = idlp_plus_rxbyte(ddp);
		if (ret == -EIO)
		{
			PRINT_ERR("timeout reading byte from IDLP.\n");
			continue;
		}
		else if (ret == DPCI_IDLP_ACKNOWLEDGE)
		{
			break;
		}
		else
		{
			PRINT_IDLP("input=%02x expected 0xAA!\n",
					(unsigned char)ret & 0xff);
			continue;
		}
	}
	if (attempts == PSP_MAX_RESYNC_ATTEMPTS)
	{
		PRINT_ERR("not idle after %d attempts.\n", attempts);
		return -EIO;
	}
	PRINT_IDLP("finally idle after %d attempts.\n", attempts);
	return 0;
}


static int idlp_plus_command_quick(struct dpci_device *ddp, u8 cmd, u8 *tdata, int tlen, u8 *rdata, int rlen)
{
	int ret = 0;
	int byteno;

	if (idlp_plus_txbyte(ddp, cmd) != 0)
	{
		PRINT_IDLP("command 0x%x, couldn't send "
			"cmd byte.\n", cmd);
		ret = -EAGAIN;
		goto done;
	}

	/*
	 * Transmit the data to the IDLP+
	 */
	for (byteno = 0; byteno < tlen; byteno++)
	{
		if ((ret = idlp_plus_txbyte(ddp, tdata[byteno])) != 0)
		{
			PRINT_ERR("idlp_plus_command_quick: "
				"command 0x%x, byte %d: "
				"error %d\n",
				cmd, byteno, ret);
			ret = -EAGAIN;
			goto done;
		}
	}

	/*
	 * Now receive data from the IDLP+.  Check if an error condition
	 * is received.
	 */
	for (byteno = 0; byteno < rlen; byteno++)
	{
		ret = idlp_plus_rxbyte(ddp);
		if (ret < 0)
		{
			PRINT_ERR("idlp_plus_command_quick: "
				"command 0x%x, return data "
				"(byte %d) 0x%02x not valid\n",
				cmd, byteno, ret);
			ret = -EAGAIN;
			goto done;
		}

		/*
		 * Check for some possible error-code responses.
		 */
		if ((byteno == 0) &&
			(cmd != DPCI_IDLP_CMD_GET_INTRUSION_STATUS &&
			 cmd != DPCI_IDLP_CMD_GET_FW_VERSION_FULL &&
			 cmd != DPCI_IDLP_CMD_GET_PROM_BYTE))
		{
			switch (ret)
			{
			case DPCI_IDLP_ACKNOWLEDGE:
				PRINT_IDLP("Other failure getting IDLP data byte %d\n",
					byteno);
				ret = -EAGAIN;
				goto done;
			case DPCI_IDLP_ERR_INVAL_CMD:
				PRINT_IDLP("Command code rejected by IDLP\n");
				ret = -ENOTTY;
				goto done;
			case DPCI_IDLP_ERR_EXCESS_DATA:
				PRINT_ERR("IDLP signalled too many data bytes (%d) sent for cmd (0x%02x\n", byteno, cmd);
				ret = -EPROTO;
				goto done;
			case DPCI_IDLP_ERR_DATA_REQUIRED:
				PRINT_ERR("IDLP siganlled required data not supplied for command 0x%02x\n", cmd);
				ret = -EPROTO;
				goto done;
			case DPCI_IDLP_ERR_INVAL_DATA:
				PRINT_IDLP("Data rejected by IDLP at data byte %d\n", byteno);
				ret = -EINVAL;
				goto done;
			case DPCI_IDLP_ERR_EXECUTION:
				PRINT_IDLP("Command execution failed in IDLP\n");
				ret = -EAGAIN;
				goto done;
			}
		}
		rdata[byteno] = (u8)ret;
	}

	if (rlen == 0)
	{
		/*
		 * If no data bytes were to be received then the IDLP+
		 * will always transmit a final 0xaa.
		 */
		ret = idlp_plus_rxbyte(ddp);
		switch (ret)
		{
		case DPCI_IDLP_ERR_EXECUTION:
			PRINT_IDLP("Command execution failed in IDLP\n");
			ret = -EAGAIN;
			break;
		case DPCI_IDLP_ERR_INVAL_DATA:
			PRINT_IDLP("Data rejected by IDLP at data byte %d\n", byteno);
			ret = -EINVAL;
			break;
		case DPCI_IDLP_ERR_EXCESS_DATA:
			PRINT_ERR("IDLP signalled too many data bytes (%d) sent for cmd (0x%02x\n", byteno, cmd);
			ret = -EOVERFLOW;
			goto done;
		case DPCI_IDLP_ERR_DATA_REQUIRED:
			PRINT_ERR("IDLP siganlled required data not supplied for command 0x%02x\n", cmd);
			ret = -EPROTO;
			goto done;
		case DPCI_IDLP_ERR_INVAL_CMD:
			PRINT_IDLP("Command code rejected by IDLP\n");
			ret = -ENOTTY;
			break;
		case DPCI_IDLP_ACKNOWLEDGE:
			ret = 0;
			break;
		default:
			PRINT_ERR("Invalid trailer byte %02x - not AA\n", ret);
			ret = -EAGAIN;
			goto done;
		}
	}
	else
	{
		ret = 0;
	}
done:
	return ret;
}


/*******************************************************************************
 *
 * Function:    idlp_plus_command
 *
 * Parameters:  cmd - the IDLP command to execute
 *              tdata - pointer to bytes to send.
 *              tlen - the number of bytes to send.
 *              rdata - pointer to bytes to receive.
 *              rlen - the number of bytes to receive.
 *
 * Returns:     the number of bytes unsent.
 *
 * Description: Sends a string of bytes on the SPI bus and checks the return
 *              data is always 0xaa.  If a byte returned is not 0xaa then the
 *              functions breaks off.  Hence a return of zero indicates all data
 *              was sent successfully.
 *
 ******************************************************************************/
static int idlp_plus_command(struct dpci_device *ddp, u8 cmd, u8 *tdata, int tlen, u8 *rdata, int rlen)
{
	int attempts;
	int ret = -EIO;

	if (down_interruptible(&ddp->idlp_cmd_sem) != 0)
	{
		return -ERESTARTSYS;
	}
	for (attempts = 0; attempts < IDLP_COMMAND_MAX_TRIES; attempts++)
	{
		PRINT_IDLP("command 0x%x attempt %d\n", cmd, attempts + 1);

		/*
		 * If idlpplus_mode is IDLP_MODE_QUICK then we do a quick
		 * (polled) method.
		 */
		if (ddp->idlpplus_mode == IDLP_PLUS_QUICK)
		{
			ret = idlp_plus_command_quick(ddp, cmd, tdata, tlen, rdata, rlen);
			switch (ret)
			{
			case -EINVAL:
			case -ENOTTY:
			case -EPROTO:
			case -EOVERFLOW:
			case 0:
				goto done;
			default:
				goto next;
			}
			break;
		}

		/*
		 * If idlp_plus_mode is IDLP_MODE_INTR then we do the regular
		 * method.
		 */
		ddp->idlp_cmd = cmd;
		ddp->idlp_tx_buf = tdata;
		ddp->idlp_tx_len = tlen;
		ddp->idlp_rx_buf = rdata;
		ddp->idlp_rx_len = rlen;
		if (rlen != 0)
		{
			ddp->idlp_aa_len = 0;
		}
		else
		{
			ddp->idlp_aa_len = 1;
		}
		ddp->idlp_data_idx = -1;
		ddp->idlp_irq_status = IIS_WORKING;
		while (ddp->idlp_irq_status == IIS_WORKING)
		{
			dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_ACKIEN, 0, 0);
			wait_event_interruptible_timeout(ddp->idlp_data_wqh,
					ddp->idlp_irq_status != IIS_WORKING,
					msecs_to_jiffies(IDLPPLUS_CMD_TIMEOUT_MS));
			dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_ACKIEN, 0);
recheck_irq_status:
			switch (ddp->idlp_irq_status)
			{
			case IIS_WORKING:
				if (dpci_reg_get(ddp, DPCI_REG0) & DPCI_REG0_PICACK)
				{
					PRINT_IDLP("timeout waiting for IDLP data byte %d - able to restart\n", ddp->idlp_data_idx);
					dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_ACKIEN, 0, 0);
					idlp_ack_irq_handler(ddp);
					goto recheck_irq_status;
				}
				PRINT_IDLP("timeout waiting for IDLP data byte %d\n",
					ddp->idlp_data_idx);
				goto next;
			case IIS_REJECTED_CMD:
				PRINT_IDLP("Command rejected by IDLP at data byte %d\n",
					ddp->idlp_data_idx);
				ret = -ENOTTY;
				goto done;
			case IIS_REJECTED_DATA:
				PRINT_IDLP("Data rejected by IDLP at data byte %d\n",
					ddp->idlp_data_idx);
				ret = -EINVAL;
				goto done;
			case IIS_EXEC_ERROR:
				PRINT_IDLP("IDLP-specific failure at data byte %d\n",
					ddp->idlp_data_idx);
				ret = -EREMOTEIO;
				goto done;
			case IIS_TOO_LITTLE_DATA:
				PRINT_ERR("IDLP siganlled required data not supplied for command 0x%02x\n",
					ddp->idlp_cmd);
				ret = -EPROTO;
				goto done;
			case IIS_TOO_MUCH_DATA:
				PRINT_ERR("IDLP signalled too many data bytes (%d) sent for cmd (0x%02x\n",
					ddp->idlp_cmd,
					ddp->idlp_data_idx);
				ret = -EOVERFLOW;
				goto done;
			case IIS_FAILED:
				PRINT_IDLP("Other failure waiting for IDLP data byte %d\n",
					ddp->idlp_data_idx);
				ret = -EIO;
				goto next;
			case IIS_COMPLETED:
				ret = 0;
				goto done;
			default:
				PRINT_ERR("IDLP work queue in state %d\n", ddp->idlp_irq_status);
			}
		}
		ret = 0;
		break;
next:
		idlp_plus_resync(ddp);
	}
	if (attempts == IDLP_COMMAND_MAX_TRIES)
	{
		ret = -EIO;
	}
done:
	up(&ddp->idlp_cmd_sem);
	PRINT_IDLP("command 0x%02x, %d attempts, ret=%d\n",
			cmd, attempts + 1, ret);
	return ret;
}


/*******************************************************************************
 *
 * Function:    idlp_ack_irq_handler
 *
 * Parameters:  none.
 *
 * Returns:     nothing.
 *
 * Description: Handles an IDLP data acknowledge interrupt.  We kill the
 *              interrupt source first.
 *
 ******************************************************************************/
void idlp_ack_irq_handler(struct dpci_device *ddp)
{
#ifdef INTERRUPT_DRIVEN_COMMS
	unsigned long flags;
#endif

	PRINT_IRQ("IDLP data ack interrupt!\n");
#ifdef INTERRUPT_DRIVEN_COMMS
	spin_lock_irqsave(&ddp->idlp_buf_spinlock, flags);
	if (ddp->idlp_irq_status != IIS_WORKING)
	{
		PRINT_ERR("Unexpected IDLP data ack interrupt: "
				"idlp_irq_status=%d, wait queue=%s!\n",
				ddp->idlp_irq_status,
				waitqueue_active(&ddp->idlp_data_wqh) ?
					"active" : "inactive");
		goto done_unlock;
	}
	if (ddp->idlp_data_idx == -1)
	{
		dpci_reg_set(ddp, DPCI_REGIDLPDATA, ddp->idlp_cmd);
		PRINT_IDLP("output[cmd]=%02x\n", ddp->idlp_cmd);
	}
	else if (ddp->idlp_data_idx < ddp->idlp_tx_len)
	{
		dpci_reg_set(ddp, DPCI_REGIDLPDATA,
					ddp->idlp_tx_buf[ddp->idlp_data_idx]);
		PRINT_IDLP("output[%d]=%02x\n",
				ddp->idlp_data_idx,
				ddp->idlp_tx_buf[ddp->idlp_data_idx]);
	}
	else if (ddp->idlp_data_idx < ddp->idlp_tx_len + ddp->idlp_rx_len)
	{
		u8 data;

		data = dpci_io_readbyte(ddp, DPCI_REGIDLPDATA);
		PRINT_IDLP("input[%d]=%02x\n",
				ddp->idlp_data_idx - ddp->idlp_tx_len, data);
		if ((ddp->idlp_data_idx == ddp->idlp_tx_len) &&
			(ddp->idlp_cmd != DPCI_IDLP_CMD_GET_INTRUSION_STATUS &&
			 ddp->idlp_cmd != DPCI_IDLP_CMD_GET_FW_VERSION_FULL &&
			 ddp->idlp_cmd != DPCI_IDLP_CMD_GET_PROM_BYTE))
		{
			switch (data)
			{
			case DPCI_IDLP_ACKNOWLEDGE:
				ddp->idlp_irq_status = IIS_FAILED;
				goto done_unlock;
			case DPCI_IDLP_ERR_INVAL_CMD:
				ddp->idlp_irq_status = IIS_REJECTED_CMD;
				goto done_unlock;
			case DPCI_IDLP_ERR_EXCESS_DATA:
				ddp->idlp_irq_status = IIS_TOO_MUCH_DATA;
				goto done_unlock;
			case DPCI_IDLP_ERR_DATA_REQUIRED:
				ddp->idlp_irq_status = IIS_TOO_LITTLE_DATA;
				goto done_unlock;
			case DPCI_IDLP_ERR_INVAL_DATA:
				ddp->idlp_irq_status = IIS_REJECTED_DATA;
				goto done_unlock;
			case DPCI_IDLP_ERR_EXECUTION:
				ddp->idlp_irq_status = IIS_EXEC_ERROR;
				goto done_unlock;
			}
		}
		ddp->idlp_rx_buf[ddp->idlp_data_idx - ddp->idlp_tx_len] = data;
	}
	else if (ddp->idlp_aa_len == 1)
	{
		/*
		 * If a command has no receive data then we always expect to
		 * get one AAh byte back.
		 */
		u8 data;

		data = dpci_io_readbyte(ddp, DPCI_REGIDLPDATA);
		PRINT_IDLP("trailer=%02x\n", data);
		switch (data)
		{
		case DPCI_IDLP_ERR_EXECUTION:
			ddp->idlp_irq_status = IIS_EXEC_ERROR;
			goto done_unlock;
		case DPCI_IDLP_ERR_EXCESS_DATA:
			ddp->idlp_irq_status = IIS_TOO_MUCH_DATA;
			goto done_unlock;
		case DPCI_IDLP_ERR_DATA_REQUIRED:
			ddp->idlp_irq_status = IIS_TOO_LITTLE_DATA;
			goto done_unlock;
		case DPCI_IDLP_ERR_INVAL_DATA:
			ddp->idlp_irq_status = IIS_REJECTED_DATA;
			goto done_unlock;
		case DPCI_IDLP_ERR_INVAL_CMD:
			ddp->idlp_irq_status = IIS_REJECTED_CMD;
			goto done_unlock;
		case DPCI_IDLP_ACKNOWLEDGE:
			break;
		default:
			ddp->idlp_irq_status = IIS_FAILED;
			PRINT_ERR("Invalid trailer byte %02x - not AA\n", data);
				break;
			goto done_unlock;
		}
	}
	ddp->idlp_data_idx++;
	if (ddp->idlp_data_idx ==
		ddp->idlp_tx_len + ddp->idlp_rx_len + ddp->idlp_aa_len)
	{
		ddp->idlp_irq_status = IIS_COMPLETED;
	}
done_unlock:
	if (ddp->idlp_irq_status != IIS_WORKING)
	{
		dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_ACKIEN, 0);
		if (waitqueue_active(&ddp->idlp_data_wqh))
			wake_up_interruptible(&ddp->idlp_data_wqh);
	}
	spin_unlock_irqrestore(&ddp->idlp_buf_spinlock, flags);
#elif defined(IDLPPLUS_ACK_INTERRUPTS)
	dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_ACKIEN, 0);
	if (waitqueue_active(&ddp->idlp_data_wqh))
		wake_up_interruptible(&ddp->idlp_data_wqh);
	else
		PRINT_ERR("Unexpected IDLP data ack interrupt!\n");
#else
	dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_ACKIEN, 0);
	PRINT_ERR("Unexpected IDLP data ack interrupt!\n");
#endif
}


/*******************************************************************************
 *
 * Function:    idlp_event_irq_handler
 *
 * Parameters:  none.
 *
 * Returns:     nothing.
 *
 * Description: Handles an interrupt for PIC reasons, i.e. an event was logged.
 *
 ******************************************************************************/
void idlp_event_irq_handler(struct dpci_device *ddp)
{
	PRINT_IRQ("IDLP event log interrupt!\n");

	/*
	 * Disable interrupts until we can shut the PIC up.  We always disable
	 * interrupts even in the unlikely event we get called when there's no-
	 * one waiting for an event interrupt; in which case, we leave a trace
	 * in the log.
	 *
	 * If we can disable PIC interrupts seperately from IO interrupts then
	 * we just disable PIC interrupts - otherwise, we have to disable the
	 * whole lot.
	 */
	ddp->thread_event.er_signal = SIGNAL_EVENT;
	dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_PICINTEN, 0);
	ddp->thread_event.op_event.de_type |= EVENT_IDLP;
	if (waitqueue_active(&ddp->thread_event.er_wqh))
	{
		/*
		 * Wake up anyone waiting for a PIC event.
		 */
		wake_up_interruptible(&ddp->thread_event.er_wqh);
	}
#if 0
	else
	{
		/*
		 * If we got a logging interrupt without expecting one then
		 * the chances are the PIC is somehow out of operation.
		 * Ouptut a message describing the issues.  If there's not a
		 * seperate PICINTEN bit then disabling interrupts will also
		 * disable serial interrupts.  Warn about this too.
		 */
		PRINT_ERR("Unexpected log interrupt.\n");
		PRINT_ERR("The intrusion and logging processor may "
				"be disabled or damaged.\n");
		PRINT_ERR("Please report this error to "
			"support@advantech-innocore.com\n");
	}
#endif
	return;
}


/*******************************************************************************
 *
 * Function:    dpci_id_wdenable()
 *
 * Parameters:  value - number of seconds for the watch-dog to timeout; a value
 *		of 0 disables the watchdog.
 *
 * Returns:     0.
 *
 * Description: converts the value to a 3-digit string and transmits it over
 *              the spi bus, prefixed by an AA,06 sequence.
 *
 ******************************************************************************/
int dpci_id_wdenable(struct dpci_device *ddp, int value)
{
	char outstr[4];
	int datalen;
	int ret;

	PRINT_DBG("(%d)\n", value);
	if (value < 0 )
	{
		return (-EINVAL);
	}
	if (ddp->features & HAVE_IDLPPLUS)
	{
		if ((((ddp->idlp_fwversion & 0xff) < 57) && (value > 255)) ||
			(((ddp->idlp_fwversion & 0xff) >= 57) && (value > 9999)))
		{
			return (-EINVAL);
		}
		
		datalen = 2;
		int_to_bcd(value, outstr, 2);
	}
	else
	{
		datalen = 3;
		sprintf(outstr,"%.3d",value);
	}

	/* Send dummy and then command */
	if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_SET_WD_TIMEOUT, outstr, datalen, 0, 0)) < 0)
		return ret;
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_id_wdreset()
 *
 * Parameters:  none
 *
 * Returns:     0 - always.
 *
 * Description: Sends a command (AA,07) over the SPI bus to reset the watchdog.
 *
 ******************************************************************************/
int dpci_id_wdreset(struct dpci_device *ddp)
{
	int ret = 0;
	/* Function to reset the watchdog */
	PRINT_DBG("\n");
	if (ddp->features & HAVE_CLRWDT)
	{
		/* Toggle CLRWDT bit on REG1 */
		dpci_reg_chg(ddp, DPCI_REG1, DPCI_REG1_CLRWDT, 0, 0);
		wait_event_timeout(ddp->clrwdt_wqh, 0, CLRWDT_DELAY_MS);
		dpci_reg_chg(ddp, DPCI_REG1, 0, DPCI_REG1_CLRWDT, 0);
	}
	else
	{
		/* send CLRWDT command to IDLP - on S and C series */
		ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_CLEAR_WD, 0, 0, 0, 0);
	}
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_id_intdisable()
 *
 * Parameters:  none
 *
 * Returns:     0 - always.
 *
 * Description: Sends a command (AA,05) over the SPI bus to disable interrupts
 *		when an event occurs.
 *
 ******************************************************************************/
int dpci_id_intdisable(struct dpci_device *ddp)
{
	PRINT_DBG("\n");
	/*
	 * Check idlp_command is valid first: if not just bail out.
	 */
	if (!ddp->idlp_command)
		return -ENOTSUPP;
	return ddp->idlp_command(ddp, DPCI_IDLP_CMD_INT_DISABLE, 0, 0, 0, 0);
}


/*******************************************************************************
 *
 * Function:    dpci_id_intenable()
 *
 * Parameters:  none
 *
 * Returns:     0 - always.
 *
 * Description: Sends a command (AA,04) over the SPI bus to enable interrupts
 *		when an event occurs.
 *
 ******************************************************************************/
int dpci_id_intenable(struct dpci_device *ddp)
{
	PRINT_DBG("\n");
	return ddp->idlp_command(ddp, DPCI_IDLP_CMD_INT_ENABLE, 0, 0, 0, 0);
}


/*******************************************************************************
 *
 * Function:    dpci_id_setdate()
 *
 * Parameters:  idevent - pointer to event containing new date information
 *
 * Returns:     0 - always.
 *
 * Description: formats the date in the idevent according to the specification
 *		required by the logging processor.  Then sends the formatted
 *		data to the processor prefixed by the AA,02 sequence.
 *
 ******************************************************************************/
int dpci_id_setdate(struct dpci_device *ddp, struct idevent *idevent)
{
	u8 tempstr[13];
	int nbytes;
	int ret;

#ifdef IDLP_PSW
	if (!ddp->flag_date_psw_active)
		return ERROR_PSW_REQ;
#endif
	PRINT_DBG("\n");

	/*
	 * Check the new time and date fields are in correct ranges.
	 */
	if ((idevent->sec >= 60) ||
		(idevent->min >= 60) ||
		(idevent->hour >= 24) ||
		(idevent->day < 1) ||
		(idevent->day > 31) ||
		(idevent->month > 12) ||
		(idevent->month < 1) ||
		(idevent->year > 99))
	{
		PRINT_DBG("invalid date %04d/%02d/%02d %02d:%02d:%02d\n",
			idevent->year + 2000,
			idevent->month,
			idevent->day,
			idevent->hour,
			idevent->min,
			idevent->sec);
		return -EINVAL;
	}

	/*
	 * Format the time/date for the intrusion logging processor and
	 * send it out.
	 */
	if (ddp->features & HAVE_IDLPPLUS)
	{
		int_to_bcd(idevent->year, &tempstr[0], 1);
		int_to_bcd(idevent->month, &tempstr[1], 1);
		int_to_bcd(idevent->day, &tempstr[2], 1);
		int_to_bcd(idevent->hour, &tempstr[3], 1);
		int_to_bcd(idevent->min, &tempstr[4], 1);
		int_to_bcd(idevent->sec, &tempstr[5], 1);
		nbytes = 6;
	}
	else
	{
		sprintf(tempstr,
			"%.2d%.2d%.2d%.2d%.2d%.2d",
			idevent->year,
			idevent->month,
			idevent->day,
			idevent->hour,
			idevent->min,
			idevent->sec);
		nbytes = 12;
	}
	if ((ret = ddp->idlp_command(ddp,
				DPCI_IDLP_CMD_SET_DATE_TIME,
				tempstr, nbytes,
				0, 0)) < 0)
	{
		return ret;
	}

	/*
	 * Log this date/time change to the system log for auditing reasons.
	 */
	PRINT_INFO("IDLP date set to %04d/%02d/%02d %02d:%02d:%02d\n",
		idevent->year + 2000,
		idevent->month,
		idevent->day,
		idevent->hour,
		idevent->min,
		idevent->sec);
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_id_setdate_psw()
 *
 * Parameters:  idevent - pointer to event containing new date information and 
 * 		password.
 *
 * Returns:     0 - always.
 *
 * Description: Checks the password entered by the user and takes action 
 * 		accordingly
 *
 ******************************************************************************/
int dpci_id_setdate_psw(struct dpci_device *ddp, struct idevent_psw *idevent_psw)
{
#ifdef IDLP_PSW
	if (memcmp(idevent_psw->psw, idlp_date_psw, SHA1_LEN) == 0)
	{
		/* 
		 * Password accepted - flag this instance. This process need
		 * not enter the password again.
		 */
		ddp->flag_date_psw_active = 1;
#endif
		return dpci_id_setdate(ddp, &idevent_psw->id_event);
#ifdef IDLP_PSW
	}
	else 
		return ERROR_PSW_INVALID;
#endif
}


/*******************************************************************************
 *
 * Function:    dpci_id_numevents()
 *
 * Parameters:  none
 *
 * Returns:     number of unread events stored by logging processor.
 *
 * Description: Talks over SPI to the logging processor and obtains the number
 *              of remaining events that haven't been read by the host.  The
 *		data is returned in ASCII format and has to be converted to
 *		integer format correctly.
 *
 ******************************************************************************/
int dpci_id_numevents(struct dpci_device *ddp)
{
	int ret;
	u8 cmd, i[3] = {'0', '0', '0'};
	int nbytes;

	PRINT_DBG("\n");
	if (ddp->features & HAVE_IDLPPLUS)
	{
		cmd = DPCI_IDLP_CMD_GET_NUM_EVENTS;
		nbytes = 2;
	}
	else if (ddp->idlp_fwversion < 25)
	{
		cmd = DPCI_IDLP_CMD_GET_NUM_EVENTS;
		nbytes = 2;
	}
	else
	{
		cmd = DPCI_IDLP_CMD_GET_NUM_EVENTS_3B;
		nbytes = 3;
	}
	ret = ddp->idlp_command(ddp, cmd, 0, 0, &i[3 - nbytes], nbytes);
	if (ret != 0)
		return ret;

	if (ddp->features & HAVE_IDLPPLUS)
	{
		ret = bcd_to_int(&i[1], 2);
	}
	else
	{
		ret = ascii_to_int(i, 3);
	}
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_id_getdate()
 *
 * Parameters:  idevent - pointer to structure into which to place date.
 *
 * Returns:     0 - always
 *
 * Description: Obtains the date from the logging processor and formats it into
 *		the event pointed to by idevent.
 *
 ******************************************************************************/
int dpci_id_getdate(struct dpci_device *ddp, struct idevent *idevent)
{
	u8 timedate[12];
	int ret;

	PRINT_DBG("\n");

	if (ddp->features & HAVE_IDLPPLUS)
	{
		if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_DATE_TIME, 0, 0, timedate, 6)) < 0)
			return ret;
		idevent->year = bcd_to_int(&timedate[0], 1);
		idevent->month = bcd_to_int(&timedate[1], 1);
		idevent->day = bcd_to_int(&timedate[2], 1);
		idevent->hour = bcd_to_int(&timedate[3], 1);
		idevent->min = bcd_to_int(&timedate[4], 1);
		idevent->sec = bcd_to_int(&timedate[5], 1);
	}
	else
	{
		if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_DATE_TIME, 0, 0, timedate, 12)) < 0)
			return ret;
		idevent->year = ascii_to_int(&timedate[0], 2);
		idevent->month = ascii_to_int(&timedate[2], 2);
		idevent->day = ascii_to_int(&timedate[4], 2);
		idevent->hour = ascii_to_int(&timedate[6], 2);
		idevent->min = ascii_to_int(&timedate[8], 2);
		idevent->sec = ascii_to_int(&timedate[10], 2);
	}
	PRINT_DBG("got date %04d/%02d/%02d %02d:%02d:%02d\n",
		idevent->year + 2000,
		idevent->month,
		idevent->day,
		idevent->hour,
		idevent->min,
		idevent->sec);
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_id_readevent()
 *
 * Parameters:  *idevent - event structure pointer
 *
 * Returns:     0.
 *
 * Description: Reads ab event from the intrusion/logging PIC and places it in
 *		the structure pointed to by idevent.
 *
 ******************************************************************************/
int dpci_id_readevent(struct dpci_device *ddp, struct idevent *idevent)
{
	u8 evtimedate[14];
	int ret;

#ifdef IDLP_PSW
	if (!ddp->flag_event_psw_active)
		return ERROR_PSW_REQ;
#endif
	/*
	 * This is only for backward compatibility for old ID_WAITEVENTS. 
	 * This is a readevent following a success in idlp_wait_event
	 */
	if (ddp->idevent_temp_flag)
	{
		*idevent = ddp->idevent_temp;
		ddp->idevent_temp_flag = 0;
	}

	if (ddp->features & HAVE_IDLPPLUS)
	{
		if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_EVENT, 0, 0, evtimedate, 7)) < 0)
			return ret;
		idevent->eventcode = bcd_to_int(&evtimedate[0], 1);
		idevent->year = bcd_to_int(&evtimedate[1], 1);
		idevent->month = bcd_to_int(&evtimedate[2], 1);
		idevent->day = bcd_to_int(&evtimedate[3], 1);
		idevent->hour = bcd_to_int(&evtimedate[4], 1);
		idevent->min = bcd_to_int(&evtimedate[5], 1);
		idevent->sec = bcd_to_int(&evtimedate[6], 1);
	}
	else
	{
		if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_EVENT, 0, 0, evtimedate, 14)) < 0)
			return ret;
		idevent->eventcode = ascii_to_int(&evtimedate[0], 2);
		idevent->year = ascii_to_int(&evtimedate[2], 2);
		idevent->month = ascii_to_int(&evtimedate[4], 2);
		idevent->day = ascii_to_int(&evtimedate[6], 2);
		idevent->hour = ascii_to_int(&evtimedate[8], 2);
		idevent->min = ascii_to_int(&evtimedate[10], 2);
		idevent->sec = ascii_to_int(&evtimedate[12], 2);
	}

	/*
	 * Log this event to the system log for auditing reasons.
	 */
	if (idevent->eventcode != ID_EVENT_NONE)
	{
		PRINT_INFO("EVENT on %04d/%02d/%02d at "
			"%02d:%02d:%02d: #%d - %s\n",
			idevent->year + 2000,
			idevent->month,
			idevent->day,
			idevent->hour,
			idevent->min,
			idevent->sec,
			idevent->eventcode,
			(idevent->eventcode >= sizeof(event_names) / sizeof(char *)) ? "unknown event" : event_names[idevent->eventcode]);
	}
	else
	{
		PRINT_DBG("No event available.\n");
	}
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_id_readlastevent()
 *
 * Parameters:  *idevent - event structure pointer
 *
 * Returns:     0.
 *
 * Description: Reads the last event from the intrusion/logging PIC and places it in
 *		the structure pointed to by idevent.
 *
 ******************************************************************************/
int dpci_id_readlastevent(struct dpci_device *ddp, 
							int event_no, 
							struct idevent *idevent)
{
	u8 evtimedate[7];
	u8 eventcode = (u8)event_no;
	int ret;

	if (!(ddp->features & HAVE_IDLPPLUS))
	{
		return -EINVAL;
	}

	PRINT_IDLP("int_evtno=%d\n", event_no);
	if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_LAST_EVENT, &eventcode, 1, evtimedate, 7)) < 0)
		return ret;

	idevent->eventcode = bcd_to_int(&evtimedate[0], 1);
	idevent->year = bcd_to_int(&evtimedate[1], 1);
	idevent->month = bcd_to_int(&evtimedate[2], 1);
	idevent->day = bcd_to_int(&evtimedate[3], 1);
	idevent->hour = bcd_to_int(&evtimedate[4], 1);
	idevent->min = bcd_to_int(&evtimedate[5], 1);
	idevent->sec = bcd_to_int(&evtimedate[6], 1);

	/*
	 * Log this event to the system log for auditing reasons.
	 */
	if (idevent->eventcode != ID_EVENT_NONE)
	{
		PRINT_INFO("EVENT code #%d last logged: %04d/%02d/%02d at "
			"%02d:%02d:%02d: #%d - %s\n",
			event_no,
			idevent->year + 2000,
			idevent->month,
			idevent->day,
			idevent->hour,
			idevent->min,
			idevent->sec,
			idevent->eventcode,
			(idevent->eventcode >= sizeof(event_names) / sizeof(char *)) ? "unknown event" : event_names[idevent->eventcode]);
	}
	else
	{
		PRINT_INFO("EVENT code #%d last logged: no record\n",
			event_no);
	}
	return 0;
}

/*******************************************************************************
 *
 * Function:    dpci_id_readchecksum()
 *
 * Parameters:  none
 *
 * Returns:     number of checksum stored by logging processor.
 *
 * Description: Reads checksum from the intrusion/logging PIC.
 *
 ******************************************************************************/
int dpci_id_readchecksum(struct dpci_device *ddp, unsigned int *pcs)
{
	int ret;

	u8 evtcs[] = {0, 0};
	if (!(ddp->features & HAVE_IDLPPLUS))
	{
		return -EINVAL;
	}

	if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_CHECKSUM, 0, 0, evtcs, 2)) < 0)
		return ret;

	*pcs = (evtcs[0] | (evtcs[1] << 8));

	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_id_readevent_psw()
 *
 * Parameters:  *idevent_psw - pointer to structure that holds the password and
 * 				the buffer for idlp event
 *
 * Returns:     0.
 *
 * Description: Checks the password provided by the user and takes action 
 * 		accordingly.
 *
 ******************************************************************************/
int dpci_id_readevent_psw(struct dpci_device *ddp, struct idevent_psw *idevent_psw)
{
#ifdef IDLP_PSW
	if (memcmp(idevent_psw->psw, idlp_event_psw, SHA1_LEN) == 0)
	{
		/* 
		 * Password accepted - flag this instance. This process need
		 * not enter the password again.
		 */
		ddp->flag_event_psw_active = 1;
#endif
		return dpci_id_readevent(ddp, &idevent_psw->id_event);
#ifdef IDLP_PSW
	}
	else 
		return ERROR_PSW_INVALID;
#endif
}


/*******************************************************************************
 *
 * Function:    dpci_id_get_selected_event()
 *
 * Parameters:  *idevent - event structure pointer
 *
 * Returns:     0.
 *
 * Description: Reads ab event from the intrusion/logging PIC and places it in
 *		the structure pointed to by idevent.
 *
 ******************************************************************************/
int dpci_id_get_selected_event(struct dpci_device *ddp,
				int event_no,
				struct idevent *idevent)
{
	u8 bcd_evtno[2];
	u8 evtimedate[7];
	int ret;

	if (!(ddp->features & HAVE_IDLPPLUS))
	{
		return -EINVAL;
	}
	int_to_bcd(event_no, bcd_evtno, sizeof(bcd_evtno));
	PRINT_IDLP("int_evtno=%d bcd_evtno=%02x%02x\n", event_no, bcd_evtno[0], bcd_evtno[1]);
	if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_SELECT_EVENT, bcd_evtno, 2, 0, 0)) < 0)
		return ret;
	if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_SELECTED_EVENT, 0, 0, evtimedate, 7)) < 0)
		return ret;
	idevent->eventcode = bcd_to_int(&evtimedate[0], 1);
	idevent->year = bcd_to_int(&evtimedate[1], 1);
	idevent->month = bcd_to_int(&evtimedate[2], 1);
	idevent->day = bcd_to_int(&evtimedate[3], 1);
	idevent->hour = bcd_to_int(&evtimedate[4], 1);
	idevent->min = bcd_to_int(&evtimedate[5], 1);
	idevent->sec = bcd_to_int(&evtimedate[6], 1);

	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_id_fwversion()
 *
 * Parameters:  none
 *
 * Returns:     u8 - the version of the firmware on the logging processor.
 *
 * Description: Sends an AA,08 command to the logging processor and reads back
 *		two bytes comprising the firmware ID.  The data is returned as
 *		two ASCII characters which have to be converted to integers
 *		correctly by subtracting the ASCII code for 0 (48,0x30).
 *
 ******************************************************************************/
int dpci_id_fwversion(struct dpci_device *ddp, unsigned int *fvp)
{
	u8 fwversion[6];
	u8 fw_ver;
	u8 build_no;
	u8 config_byte;
	int ret;

	if (ddp->features & HAVE_IDLPPLUS)
	{
		if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_FW_VERSION_FULL, 0, 0, fwversion, 4)) < 0)
		{
			if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_FW_VERSION, 0, 0, fwversion, 1)) < 0)
				return ret;
			fwversion[1] = 0;
			fwversion[2] = 0;
			fwversion[3] = 0;
		}
		fwversion[0] = bcd_to_int(fwversion, 1);
		*fvp = (fwversion[0] | (fwversion[1] << 8) | (fwversion[2] << 16) | (fwversion[3] << 24));
	}
	else
	{
		if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_FW_VERSION, 0, 0, fwversion, 2)) < 0)
			return ret;
		fw_ver = ascii_to_int(fwversion, 2);

		if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_FW_BUILD_NUM, 0, 0, fwversion, 6)) < 0)
		{
			*fvp = fw_ver;
		}
		else
		{
			build_no = ascii_to_int(fwversion, 3);
			config_byte = ascii_to_int(&(fwversion[3]), 3);		
			fwversion[0] = fw_ver;
			fwversion[1] = build_no;
			fwversion[2] = 0;
			fwversion[3] = config_byte;
			*fvp = (fwversion[0] | (fwversion[1] << 8) | (fwversion[2] << 16) | (fwversion[3] << 24));
		}
	}
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_id_setsleeptime()
 *
 * Parameters:  sleepcode - code determing period for sleep.
 *
 * Returns:     0 for success
 *
 * Description: Sets the sleeping period between checks of the intrusion detec-
 *              tion lines.  The allowable times are 125ms, 250ms, 500ms or 1s.
 *
 ******************************************************************************/
int dpci_id_setsleeptime(struct dpci_device *ddp, int sleepcode)
{
	u8 usleepcode = (u8)sleepcode;

	/*
	 * PICs with firmware versions < 10 do not support this command.
	 * However, the PIC just accepts it anyway so we have to flag the
	 * errors here.
	 */
	if (ddp->idlp_fwversion < 10)
		return -EINVAL;

	/*
	 * The PIC also just blithely accepts duff sleep-code values without
	 * much argument so we filter those out too.
	 */
	switch (sleepcode)
	{
	case INTRUS_SLEEP_1SEC:
	case INTRUS_SLEEP_500MSEC:
	case INTRUS_SLEEP_250MSEC:
	case INTRUS_SLEEP_125MSEC:
		break;
	default:
		return -EINVAL;
	}

	/*
	 * We can now send the command.
	 */
	return ddp->idlp_command(ddp, DPCI_IDLP_CMD_SET_SLEEP_TIME, &usleepcode, 1, 0, 0);
}


/*******************************************************************************
 *
 * Function:    dpci_id_intrusionstatus()
 *
 * Parameters:  none
 *
 * Returns:     the intrusion status bit-mask.
 *
 * Description: Sends an 0A command to the logging processor and reads back
 *		two bytes comprising the firmware ID.  The data is returned as
 *		two ASCII characters which have to be converted to integers
 *		correctly by subtracting the ASCII code for 0 (48,0x30) and a
 *              bit of bit shifting.
 *
 ******************************************************************************/
int dpci_id_intrusionstatus(struct dpci_device *ddp)
{
	u8 bytes[2];
	int nbytes;
	int status;
	int ret;

	/*
	 * PICs with firmware versions < 24 do not support this command.
	 * However, the PIC just accepts it anyway so we have to flag the
	 * errors here.
	 */
	if (ddp->idlp_fwversion < 24)
		return -EINVAL;

	if (ddp->features & HAVE_IDLPPLUS)
	{
		nbytes = 1;
	}
	else
	{
		nbytes = 2;
	}

	if ((ret = ddp->idlp_command(ddp,
				DPCI_IDLP_CMD_GET_INTRUSION_STATUS,
				0, 0,
				bytes, nbytes)) < 0)
		return ret;

	if (ddp->features & HAVE_IDLPPLUS)
		status = (bytes[0] & 0x87) | ((bytes[0] & 0x70) >> 1) | ((bytes[0] & 0x08) << 3);
	else
		status = (bytes[0] & 0x7) | ((bytes[1] & 0x7) << 3);
	(void) dpci_id_wdreset(ddp);
	return status;
}


/*******************************************************************************
 *
 * Function:    dpci_id_get_prom_byte()
 *
 * Parameters:  ddp - the device to contact
 *              offs - the offset from which to recieve the byte.
 *
 * Returns:     >= 0 for valid data, <0 for errors
 *
 * Description: Reads a byte from the system's ID prom.
 *
 ******************************************************************************/
static int dpci_id_get_prom_byte(struct dpci_device *ddp, int offs)
{
	int ret = ret;
	u8 data[2];

	if (!(ddp->features & HAVE_IDLPPLUS) || offs >= 128 || offs < 0)
	{
		return -EINVAL;
	}
	data[0] = offs;
	if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_SELECT_PROM_BYTE, data, 1, 0, 0)) < 0)
		return ret;
	if ((ret = ddp->idlp_command(ddp, DPCI_IDLP_CMD_GET_PROM_BYTE, 0, 0, data, 2)) < 0)
		return ret;
	if (data[1] != DPCI_IDLP_ACKNOWLEDGE)
	{
		PRINT_ERR("2nd return byte from IDLP should be 0xaa but got %02x\n", data[1]);
		ret = -EIO;
	}
	else
	{
		ret = data[0];
	}
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_id_setbattcheckperiod()
 *
 * Parameters:  ddp - the device to contact
 *              code - the checking period code.
 *
 * Returns:     >= 0 for valid data, <0 for errors
 *
 * Description: Instructs the IDLP how often to check battery levels.
 *
 ******************************************************************************/
int dpci_id_setbattcheckperiod(struct dpci_device *ddp, int code)
{
	u8 data;

	if (!(ddp->features & HAVE_IDLPPLUS))
	{
		return -EINVAL;
	}
	data = code;
	return ddp->idlp_command(ddp,
				DPCI_IDLP_CMD_SET_BATT_CHECK_PERIOD,
				&data,
				1,
				0,
				0);
}


/*******************************************************************************
 *
 * Function:    dpci_id_getbattstatus()
 *
 * Parameters:  ddp - the device to contact
 *              code - the checking period code.
 *
 * Returns:     >= 0 for valid data, <0 for errors
 *
 * Description: Reads the battery status word from the IDLP.
 *
 ******************************************************************************/
int dpci_id_getbattstatus(struct dpci_device *ddp)
{
	u8 data;
	int ret;

	if (!(ddp->features & HAVE_IDLPPLUS))
	{
		return -EINVAL;
	}
	if ((ret = ddp->idlp_command(ddp,
				DPCI_IDLP_CMD_GET_BATT_STATUS,
				0,
				0,
				&data,
				1)) < 0)
	{
		return ret;
	}
	return data;
}


/*******************************************************************************
 *
 * Function:    dpci_id_getbattlevel()
 *
 * Parameters:  ddp - the device to contact
 *              batt - the battery whose level is to be obtained.
 *
 * Returns:     >= 0 units of voltage (about 17mV), <0 for errors
 *
 * Description: Reads the battery level from the IDLP.
 *
 ******************************************************************************/
int dpci_id_getbattlevel(struct dpci_device *ddp, int batt)
{
	u8 data[4];
	int units;
	int len;
	int ret;

	if(ddp->idlp_fwversion < 44)
		len = 2;
	else
		len = 4;

	if ((ret = ddp->idlp_command(ddp,
				DPCI_IDLP_CMD_GET_BATT1_VOLTAGE + batt,
				0,
				0,
				data,
				len)) < 0)
		return ret;
	units = bcd_to_int(data, 2);
	return units;
}

/*******************************************************************************
 *
 * Function:    dpci_id_getbatt_errorlevel()
 *
 * Parameters:  ddp - the device to contact
 *              batt - the battery whose error level is to be obtained.
 *
 * Returns:     >= 0 units of voltage (about 18.4mV), <0 for errors
 *
 * Description: Reads the battery error level from the IDLP.
 *
 ******************************************************************************/
int dpci_id_getbatt_errorlevel(struct dpci_device *ddp, int batt)
{
	u8 data[4];
	int units;
	int len;
	int ret;

	if(ddp->idlp_fwversion < 44)
		return -1;
	else
		len = 4;

	if ((ret = ddp->idlp_command(ddp,
				DPCI_IDLP_CMD_GET_BATT1_VOLTAGE + batt,
				0,
				0,
				data,
				len)) < 0)
		return ret;
	units = bcd_to_int(&data[2], 2);
	return units;
}

/*******************************************************************************
 *
 * Function:    dpci_id_setbatterrorlevel()
 *
 * Parameters:  ddp - the device to contact
 *              batt - the battery whose level is to be obtained.
 *              units - the numbers of units below which the battery level is
 *                      considered to be in error.
 *
 * Returns:     >= 0 for valid data, <0 for errors
 *
 * Description: Reads the battery level from the IDLP.
 *
 ******************************************************************************/
int dpci_id_setbatterrorlevel(struct dpci_device *ddp, int batt, int units)
{
	u8 data[2];

	int_to_bcd(units, data, 2);
	PRINT_IDLP("batt=%d units=%d bcd_data=%02x%02x\n",
			batt, units, data[0], data[1]);
	return ddp->idlp_command(ddp,
				DPCI_IDLP_CMD_SET_BATT1_ERROR_LEVEL + batt,
				data,
				2,
				0,
				0);
}

/*******************************************************************************
 *
 * Function:    dpci_id_wait_event()
 *
 * Parameters:  ddp - the device to contact
 *              arg - the timeout value passed-in by the user.
 *		cont_wait - should the wait continue after one event
 *
 * Returns:     = 0 for timeout, <0 for errors, =1 for interrupt received
 *
 * Description: Waits for an IDLP event to occur for a specified timeout.
 *
 ******************************************************************************/
int dpci_id_wait_event(struct dpci_device *ddp, 
		struct idevent_timeout *idevent_tm,
		int cont_wait)
{
	int ret = 0;
	struct dpci_event op_event;

#ifdef IDLP_PSW
	if (!ddp->flag_event_psw_active)
		return ERROR_PSW_REQ;
#endif
	ret = dpci_wait_events(ddp,
			EVENT_IDLP,
			idevent_tm->timeout,
			cont_wait,
			&op_event,
			0,
			0,
			0);
	if (ret < 0)
	{
		PRINT_DBG("interrupted by signal \n");
	}
	else if (ret == 0)
	{
		PRINT_DBG("Timeout waiting on IDLP event \n");
	}
	else if (op_event.de_type != EVENT_IDLP)
	{
		PRINT_ERR("Unexpected event type signalled \n");
		PRINT_ERR("Waiting for Event type %d, got Event type %d\n",
			EVENT_IDLP, op_event.de_type);
		ret = -EFAULT;
	}
	else
	{
		idevent_tm->id_event = op_event.de_data.idlp_event;
	}
	PRINT_IOC("event wait return=%d\n", ret);
	
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_id_wait_event_psw()
 *
 * Parameters:  ddp - the device to contact
 *              idwait_psw_buf - the buffer containing the timeout value and 
 *              password passed-in by the user.
 *		cont_wait - should the wait continue after one event
 *
 * Returns:     = 0 for timeout, <0 for errors, =1 for interrupt received
 *
 * Description: Waits for an IDLP event to occur for a specified timeout.
 *
 ******************************************************************************/
int dpci_id_wait_event_psw(struct dpci_device *ddp, 
			struct idevent_timeout_psw *idevent_tm_psw,
			int cont_wait)
{
#ifdef IDLP_PSW
	if (memcmp(idevent_tm_psw.psw, idlp_event_psw, SHA1_LEN) == 0)
	{
		/* 
		 * Password accepted - flag this instance. This process need
		 * not enter the password again.
		 */
		ddp->flag_event_psw_active = 1;
#endif
		return dpci_id_wait_event(ddp, 
				&idevent_tm_psw->idevent_tm, cont_wait);
#ifdef IDLP_PSW
	}
	else 
		return ERROR_PSW_INVALID;
#endif
}


/*******************************************************************************
 *
 * Function:    idlp_ioctl()
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
 ******************************************************************************/
int dpci_id_ioctl(struct file *file,
			unsigned int cmd,
			unsigned long arg)
{
	struct dpci_device *ddp = file->private_data;
	struct dpci_idlp_cmdbuf cmdbuf;
	struct idevent buf;
	struct idevent_timeout idevent_tm;
	struct idevent_psw buf_psw;
	struct idevent_timeout_psw idevent_tm_psw;
	int event_no;
	int offs;
	int ret = 0;
	struct idwait_psw idwait_psw_buf;

	PRINT_IOC("(file=0x%p, cmd=0x%x, arg=0x%lx)\n", file, cmd, arg);

	switch(cmd)
	{
	case IOCTL_DPCI_WD_ENABLE:
		ret = dpci_id_wdenable(ddp, arg);
		break;

	case IOCTL_DPCI_WD_RESET:
		ret = dpci_id_wdreset(ddp);
		break;

	case IOCTL_DPCI_ID_NUMEVENTS:
		/*
		 * The old 2.6 driver had GETEVENTS and READEVENT confused such
		 * that the READEVENT ioctl would return the count of events in
		 * the intrusion logging processor.  We make sure support the
		 * "broken" behaviour for the old code so that old binaries
		 * continue to function correctly.
		 */
		ret = dpci_id_numevents(ddp);
		break;

	case IOCTL_DPCI_ID_READCHECKSUM:
		{
			unsigned int cs;
			ret = dpci_id_readchecksum(ddp, &cs);
			if (ret == 0)
			{
				ret = cs & 0xffff;
			}
		break;
		}
	case IOCTL_DPCI_ID_READEVENT:
		ret = dpci_id_readevent(ddp, &buf);
		if (ret != 0)
			break;
		ret = copy_to_user((void *)arg, &buf, sizeof(buf));
		break;

	case IOCTL_DPCI_ID_READEVENT_INSTANCE:
		if (copy_from_user(&event_no, (void *)arg, sizeof(event_no)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_id_readlastevent(ddp, event_no, &buf);
		if (ret != 0)
			break;
		ret = copy_to_user((void *)arg, &buf, sizeof(buf));
		break;

	case IOCTL_DPCI_ID_READEVENT_PSW:
		if (copy_from_user(&buf_psw, (void *)arg, sizeof(buf_psw)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_id_readevent_psw(ddp, &buf_psw);
		if (ret != 0)
			break;
		ret = copy_to_user((void *)arg, &buf_psw, sizeof(buf_psw));
		break;

	case IOCTL_DPCI_ID_SETDATE:
		if (copy_from_user(&buf, (void *)arg, sizeof(buf)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_id_setdate(ddp, &buf);
		break;

	case IOCTL_DPCI_ID_SETDATE_PSW:
		if (copy_from_user(&buf_psw, (void *)arg, sizeof(buf_psw)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_id_setdate_psw(ddp, &buf_psw);
		break;

	case IOCTL_DPCI_ID_GETDATE:
		ret = dpci_id_getdate(ddp, &buf);
		if (ret < 0)
			break;
		ret = copy_to_user((void *)arg, &buf, sizeof(buf));
		break;

	case IOCTL_DPCI_ID_FWVERSION:
		{
			unsigned int version;

			ret = dpci_id_fwversion(ddp, &version);
			if (ret == 0)
			{
				ret = version & 0xff;
			}
			break;
		}

	case IOCTL_DPCI_ID_FWVERSIONFULL:
		{
			unsigned int version;

			ret = dpci_id_fwversion(ddp, &version);
			if (ret == 0)
			{
				ret = copy_to_user((void *)arg, &version, sizeof(version));
			}
		}
		break;

	case IOCTL_DPCI_ID_SETSLEEPTIME:
		ret = dpci_id_setsleeptime(ddp, arg);
		break;

	case IOCTL_DPCI_ID_INTRUSIONSTATUS:
		ret = dpci_id_intrusionstatus(ddp);
		break;

	case __IOCTL_DPCI_ID_WAITEVENT:
	case _IOCTL_DPCI_ID_WAITEVENT:
		idevent_tm.timeout = arg;
		ret = dpci_id_wait_event(ddp, &idevent_tm, 0);

		/*	 	
		 * This is only for backward compatibility for old 
		 * ID_WAITEVENTS. 
		 */
		if (ret > 0)
		{
			ddp->idevent_temp = idevent_tm.id_event;
			ddp->idevent_temp_flag = 1;
		}
		break;

	case IOCTL_DPCI_ID_WAITEVENT:
		if (copy_from_user(&idevent_tm, 
				(void *)arg, 
				sizeof(idevent_tm)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_id_wait_event(ddp, &idevent_tm, 1);
		if (ret > 0)
		{
			if (copy_to_user((void *)arg, 
				&idevent_tm, 
				sizeof(idevent_tm)))
			{
				ret = -EFAULT;
				break;
			}
		}
		break;

	case _IOCTL_DPCI_ID_WAITEVENT_PSW:
		if (copy_from_user(&idwait_psw_buf, 
				(void *)arg, 
				sizeof(idwait_psw_buf)))
		{
			ret = -EFAULT;
			break;
		}
		idevent_tm_psw.idevent_tm.timeout = idwait_psw_buf.timeout;
		strcpy(idevent_tm_psw.psw, idwait_psw_buf.psw);
		ret = dpci_id_wait_event_psw(ddp, &idevent_tm_psw, 0);
		/*	 	
		 * This is only for backward compatibility for old 
		 * ID_WAITEVENTS. 
		 */
		if (ret > 0)
		{
			ddp->idevent_temp = idevent_tm.id_event;
			ddp->idevent_temp_flag = 1;
		}
		break;

	case IOCTL_DPCI_ID_WAITEVENT_PSW:
		if (copy_from_user(&idevent_tm_psw, 
				(void *)arg,
				sizeof(idevent_tm_psw)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_id_wait_event_psw(ddp, &idevent_tm_psw, 1);
		if (ret > 0)
		{
			if(copy_to_user((void *)arg, 
				&idevent_tm_psw, 
				sizeof(idevent_tm_psw)))
			{
				ret = -EFAULT;
				break;
			}
		}
		break;

	case IOCTL_DPCI_ID_GET_SELECTED_EVENT:
		if (copy_from_user(&event_no, (void *)arg, sizeof(event_no)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_id_get_selected_event(ddp, event_no, &buf);
		if (ret != 0)
			goto done;
		ret = copy_to_user((void *)arg, &buf, sizeof(buf));
		break;

	default:
		ret = -EINVAL;
		break;

	case IOCTL_DPCI_ID_GETPROMBYTE:
		if (copy_from_user(&offs, (void *)arg, sizeof(offs)))
		{
			ret = -EFAULT;
			break;
		}
		ret = dpci_id_get_prom_byte(ddp, offs);
		break;

	case IOCTL_DPCI_ID_DEBUG_CMD:
		if (copy_from_user(&cmdbuf, (void *)arg, sizeof(cmdbuf)))
		{
			ret = -EFAULT;
			break;
		}
		if (cmdbuf.dic_n_tx_bytes < 1 || cmdbuf.dic_n_rx_bytes < 0)
		{
			ret = -EINVAL;
			break;
		}
		ret = ddp->idlp_command(ddp,
					cmdbuf.dic_tx_bytes[0],
					&cmdbuf.dic_tx_bytes[1],
					cmdbuf.dic_n_tx_bytes - 1,
					&cmdbuf.dic_rx_bytes[0],
					cmdbuf.dic_n_rx_bytes);
		if (ret == 0)
		{
			ret = copy_to_user((void *)arg, &cmdbuf, sizeof(cmdbuf));
		}
		break;
	}

done:
	PRINT_IOC("(file=0x%p, cmd=0x%x, arg=0x%lx) = %d\n",
		file, cmd, arg, ret);

	return ret;
}


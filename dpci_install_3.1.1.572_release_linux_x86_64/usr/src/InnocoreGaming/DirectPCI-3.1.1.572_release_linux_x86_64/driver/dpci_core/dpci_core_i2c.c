/******************************************************************************
 *
 * $Id: dpci_core_i2c.c 11905 2015-09-07 15:28:17Z aidan $
 *
 * Copyright 2003-2012 Advantech Co Limited.
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

#define MAX_TOTAL_CLOCKS        2000
#define MAX_INTERSEG_CLOCKS     25
#define MAX_RWBYTE_ATTEMPTS     3
#define INTER_ATTEMPT_DELAY_MS  5

static void i2c_bitbang_set_scl_sda(struct dpci_i2c_port *dipp,
					uint8_t set,
					uint8_t clr)
{
	uint8_t set2 = set, clr2 = clr;

	set2 = (clr & ((dipp->dip_sda_w | dipp->dip_scl_w) & (dipp->dip_sda_iw | dipp->dip_scl_iw))) | (set & ((dipp->dip_sda_w | dipp->dip_scl_w) & ~(dipp->dip_sda_iw | dipp->dip_scl_iw)));
	clr2 = (set & ((dipp->dip_sda_w | dipp->dip_scl_w) & (dipp->dip_sda_iw | dipp->dip_scl_iw))) | (clr & ((dipp->dip_sda_w | dipp->dip_scl_w) & ~(dipp->dip_sda_iw | dipp->dip_scl_iw)));
	dpci_reg_chg(dipp->dip_ddp, dipp->dip_reg, set2, clr2, 0);
}


static uint8_t i2c_bitbang_get_sda_scl(struct dpci_i2c_port *dipp)
{
	return dpci_io_readbyte(dipp->dip_ddp, dipp->dip_reg) ^
					(dipp->dip_sda_ir | dipp->dip_scl_ir);
}


/*******************************************************************************
 *
 * Function:    i2c_bitbang_execute()
 *
 * Parameters:  pContext - the context in which this DPC executes.
 *
 * Returns:     BOOLEAN - TRUE if DPC was successful, FALSE otherwise.
 *
 * Description: This is the bit-banging I2C state machine.
 *
 * Timing Diagram of a single byte transfer, with start and stop conditions
 *
 * BIT          8   7   6   5   4   2   1   0  ACK
 *        _     ___   ___               _              ___
 * SDA     |___|   |_|   |_____________| |____________|
 *        ___   _   _   _   _   _   _   _   _   _    _____
 * SCL       |_| |_| |_| |_| |_| |_| |_| |_| |_| |__|
 *        :: :: : : : :                         : ::: :  :
 * STATE  || || | | | |                         | ||| |  \
 *        || || | | | \                         | ||| \   COMPLETED
 *        || || | | \  DATA_CLKLO               | ||\  STOP_SDAHI
 *        || || | \  DATA_TX                    | |\ STOP_SCLHI
 *        || || \  DATA_CLKLO                   | \ STOP_SCLLO
 *        || |\  DATA_TX                        \  ACK_CLKLO
 *        || \ START_CLKWAIT                     ACK_RX
 *        |\  START_CLKLO
 *        \ START_SDALO
 *        |INACTIVE
 *
 * You should be familiar with the description of I2C transactions which is
 * given on the Wiki.  (Search for i2c and it should appear.)
 *
 * This is not by any means a simple state machine.  But it's not really such a
 * complicated one either.  It's just possible this could be reduced to in-line
 * code but I don't think it's worth the bother because of all the testing and
 * risk of error involved.  The code would not be any simpler or prettier.
 *
 * The state model execution operates in two stages, and goes round and around
 * continually until the COMPLETED state is achieved.
 *
 * 1. We check whether the state we're in is the correct one and decide whether
 *    or not to move to another state.   Moves to another state may be predica-
 *    ted on the condition of the bus lines or simple timing progression - or
 *    even both.  It's in this phase where we check data lines to read input
 *    data or acknowledgements and where we determine, ultimately, the final
 *    result of the whole i2c command.
 *
 * 2. For whatever is the new state decided by #1 above, we manipulate the bus
 *    signals or other timings so that the next check in #1 can proceed to
 *    determine the move to the following state.
 *
 * There is a delay between each state so that the typical execution profile
 * looks roughly like this.
 *
 * check-old move-new delay -> check-old move-new delay -> check-old ...
 *
 * If there is no change of state detected, then the actions for a new state
 * are not executed but the delay is still implemented before the next check.
 *
 * Currently the inter-stage delay is 5us which allows for a 200kHz clock.
 *
 * This means that with an AT24C256 slave, a 32-byte page read will take:
 *
 * START condition:	  4 cycles
 * Slave address:	 18 cycles (1 byte + ack)
 * Page address:	 36 cycles (2 bytes + 2 acks)
 * Page data:		576 cycles (32 bytes + 32 acks)
 * STOP condition:	  4 cycles
 * -------------------------------
 * Total time:          638 cycles @ 4us = 2552us.
 *
 ******************************************************************************/
void i2c_bitbang_execute(struct dpci_i2c_context *i2c_context)
{
	dpci_i2c_state_t new_state;
	unsigned char I2cReg;
	struct dpci_i2c_port *dipp = i2c_context->port;
#define	MAX_CLOCKS_ANY_STATE	50

	I2cReg = i2c_bitbang_get_sda_scl(dipp);
	PRINT_I2C("i2c_bitbang_execute: reg=%02x CLK=%d DATA=%d\n",
			I2cReg,
			!!(I2cReg & dipp->dip_scl_r),
			!!(I2cReg & dipp->dip_sda_r));
	new_state = i2c_context->i2c_state;

	/*
	 * Check to see if the current state can change to another.
	 */
	switch (i2c_context->i2c_state)
	{
	case I2C_INACTIVE:
		/*
		 * Write SDA and SCL high and then check that they are both
		 * high.  If so, move to the SDALO state.  Writing these high
		 * won't make any difference to anyone writing a 0 to the bus.
		 */
		//PRINT_I2C("chk I2C_INACTIVE\n");
		i2c_bitbang_set_scl_sda(dipp,
				(dipp->dip_sda_w | dipp->dip_scl_w),
				0);
		I2cReg = i2c_bitbang_get_sda_scl(dipp);
		if ((I2cReg & (dipp->dip_sda_r | dipp->dip_scl_r)) ==
					(dipp->dip_sda_r | dipp->dip_scl_r))
			new_state = I2C_START_SDALO;
		break;

	case I2C_START_SDALO:
		/*
		 * See if SDA really went low like we need.
		 * If so, then we can move on to SCL being moved lo.
		 */

		//PRINT_I2C("chk I2C_START_SDALO\n");
		if (!(I2cReg & dipp->dip_sda_r))
		{
			new_state = I2C_START_SCLLO;
		}
		break;

	case I2C_START_SCLLO:
		/*
		 * See if SCL really went low like we need.
		 */
		//PRINT_I2C("chk I2C_START_SCLLO\n");
		if (!(I2cReg & dipp->dip_scl_r))
		{
			new_state = I2C_START_CLKWAIT;
		}
		break;

	case I2C_START_CLKWAIT:
		/*
		 * There's nothing to check here - we go straight to the data
		 * transmit stage as the first thing the master does is
		 * transmit the slave addr.
		 */
		//PRINT_I2C("chk I2C_START_CLKWAIT\n");
		new_state = I2C_DATA_TX;
		i2c_context->current_byte_no = 0;
		i2c_context->current_bit_mask = 0x80;
		i2c_context->current_segment_length =
				i2c_context->i2c_cmdbuf_p->
				segment_lengths[i2c_context->current_segment];
		i2c_context->current_address = *i2c_context->current_byte_p;
		PRINT_I2C("I2C_ACK_CLKLO "
				"slave=%02x, next TX byte[%d]=%02x\n",
				i2c_context->current_address,
				i2c_context->current_byte_no,
			 *(i2c_context->current_byte_p));
		break;

	case I2C_DATA_TX:
		/*
		 * Check to see if we got an arbitration failure.  This happens
		 * if we try to send a one but a zero is on the bus.  If this
		 * is the case then we lose.  If we write a zero when someone
		 * else's writes a one then they lose - presuming they're
		 * bothering to check.  The loser must simply stop using the
		 * bus without any further actions, stop conditions etc.
		 */
		if (!(I2cReg & dipp->dip_sda_r) &&
			(*(i2c_context->current_byte_p) &
			i2c_context->current_bit_mask))
		{
			new_state = I2C_COMPLETED;
			i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_LOSTARB;
			break;
		}

		/*
		 * We always move into the CLKWAIT state where we wait to ensure
		 * the clock stays high for at least one clock cycle.
		 */
		//PRINT_I2C("chk I2C_DATA_TX\n");
		new_state = I2C_DATA_CLKLO;
		break;

	case I2C_DATA_RX:
		/*
		 * We always move into the CLKWAIT state where we wait to ensure
		 * the clock stays high for at least one clock cycle.
		 */
		PRINT_I2C("chk I2C_DATA_RX: bit-mask=%02x DATA=%d\n",
				i2c_context->current_bit_mask,
				(I2cReg & dipp->dip_sda_r) ? 1 : 0);
		if (I2cReg & dipp->dip_sda_r)
		{
		 *(i2c_context->current_byte_p) |= i2c_context->current_bit_mask;
		}
		new_state = I2C_DATA_CLKLO;
		break;

	case I2C_DATA_CLKWAIT:
		/*
		 * See if SCL really went high like we need.
		 * If so, then we can move on to SCL being moved low again.
		 */
		//PRINT_I2C("chk I2C_DATA_CLKWAIT\n");
		new_state = I2C_DATA_CLKLO;
		break;

	case I2C_DATA_CLKLO:
		/*
		 * We always go into one of four states now:
		 *
		 * I2C_DATA_TX:  on byte 0 of the segment,
		 *               on any byte of a Tx segment
		 * I2C_DATA_RX:  on the first or later byte of an Rx segment
		 * I2C_ACK_TX:   on the acknowledge bit of a Tx segment
		 * I2C_ACK_RX:   on the acknowledge bit of a Rx segment
		 */
		//PRINT_I2C("chk I2C_DATA_CLKLO\n");
		i2c_context->current_bit_mask >>= 1;
		if (!i2c_context->current_bit_mask)
		{
			if (!(i2c_context->current_address & 1) || !i2c_context->current_byte_no)
			{
				new_state = I2C_ACK_RX;
			}
			else
			{
				new_state = I2C_ACK_TX;
			}
		}
		else
		{
			if (!(i2c_context->current_address & 1) || !i2c_context->current_byte_no)
			{
				new_state = I2C_DATA_TX;
			}
			else
			{
				new_state = I2C_DATA_RX;
			}
		}
		break;

	case I2C_ACK_TX:
		/*
		 * Transmit the acknowledge bit.  Check whether the clock is
		 * actually high before commiting to changing state.
		 */
		PRINT_I2C("chk I2C_ACK_TX "
					"byte[%d]=%02x\n",
						i2c_context->current_byte_no,
					 *(i2c_context->current_byte_p));
		if (!(I2cReg & dipp->dip_scl_r))
		{
			break;
		}

		/*
		 * Check to see if we got an arbitration failure.  This happens
		 * if we try to send a one but a zero is on the bus.  If this
		 * is the case then we lose.  If we write a zero when someone
		 * else's writes a one then they lose - presuming they're
		 * bothering to check.  The loser must simply stop using the
		 * bus without any further actions, stop conditions etc.
		 */
		if ((I2cReg & dipp->dip_sda_r) &&
			(i2c_context->current_byte_no ==
			i2c_context->current_segment_length))
		{
			new_state = I2C_COMPLETED;
			i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_LOSTARB;
		}
		else
		{
			new_state = I2C_ACK_CLKLO;
		}
		break;

	case I2C_ACK_RX:
		/*
		 * Check the data line in - if it's high then that's a NAK.
		 * Set the i2c buffer result to the correct value.  If the NAK
		 * came when sending the address byte then the slave's not
		 * there; otherwise, the slave refused the data.
		 */
		if (I2cReg & dipp->dip_sda_r)
		{
			PRINT_I2C("chk I2C_ACK_RX: NO SLAVE ACK!\n");
			new_state = I2C_STOP_SCLLO;
			if (i2c_context->current_byte_no)
			{
				i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_TXNAK;
			}
			else
			{
				i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_NOSLAVE;
			}
		}
		else
		{
			PRINT_I2C("chk I2C_ACK_RX: SLAVE ACK!\n");
			new_state = I2C_ACK_CLKWAIT;
		}
		break;

	case I2C_ACK_CLKWAIT:
		//
		// See if SDA really went high like we need.
		// If so, then we can move on to SCL being moved low again.
		//
		//PRINT_I2C("chk I2C_ACK_CLKWAIT\n");
		if ((I2cReg & dipp->dip_scl_r) && i2c_context->clocks > 0)
		{
			new_state = I2C_ACK_CLKLO;
		}
		break;

	case I2C_ACK_CLKLO:
		/*
		 * We can go into one of three states now:
		 *
		 * I2C_DATA_TX:  on byte 0 of any segment,
		 *               on subsequent bytes of a Tx segment
		 * I2C_DATA_RX:  on the first or later byte of an Rx segment
		 * I2C_STOP_SCLLO: on the last low-clock time of the segment
		 */
		PRINT_I2C("chk I2C_ACK_CLKLO: shifted byte #%d of %d: %02x\n",
			i2c_context->current_byte_no + 1,
			i2c_context->current_segment_length,
		 *(i2c_context->current_byte_p));
		i2c_context->current_byte_p++;
		if (++(i2c_context->current_byte_no) >=
					i2c_context->current_segment_length)
		{
			new_state = I2C_STOP_SCLLO;
		}
		else
		{
			i2c_context->current_bit_mask = 0x80;
			if (i2c_context->current_address & 1)
			{
			 *(i2c_context->current_byte_p) = 0;
				new_state = I2C_DATA_RX;
			}
			else
			{
				new_state = I2C_DATA_TX;
				PRINT_I2C("I2C_ACK_CLKLO "
						"next TX byte[%d]=%02x\n",
						i2c_context->current_byte_no,
					 *(i2c_context->current_byte_p));
			}
		}
		break;

	case I2C_STOP_SCLLO:
		/*
		 * See if SDA really went low like we need.
		 * If so, then we can move on to SCL being moved high.
		 */
		//PRINT_I2C("chk I2C_STOP_SCLLO\n");
		if (!(I2cReg & dipp->dip_scl_r))
		{
			new_state = I2C_STOP_SCLHI;
		}
		break;

	case I2C_STOP_SCLHI:
		/*
		 * See if SCL really went high like we need.
		 * If so, then we can move on to SDA being moved high.
		 */
		//PRINT_I2C("chk I2C_STOP_SCLHI\n");
		if (I2cReg & dipp->dip_scl_r)
		{
			new_state = I2C_STOP_SDAHI;
		}
		break;

	case I2C_STOP_SDAHI:
		/*
		 * See if SDA really went high like we need.
		 * If so, then we can move on to either the completed or
		 * inactive states, depending upon whether there are further
		 * segments and whether the last segment was completed okay.
		 */
		//PRINT_I2C("chk I2C_STOP_SDAHI\n");
		if (I2cReg & dipp->dip_sda_r &&
			i2c_context->clocks > MAX_INTERSEG_CLOCKS)
		{
			/*
			 * If we got this far and someone changed the result
			 * setting from the default in-progress setting then it
			 * means they're signalling a failure.  Go immediately
			 * to the completed state.
			 */
			if (i2c_context->i2c_cmdbuf_p->result !=
						I2C_RESULT_INPROGRESS)
			{
				new_state = I2C_COMPLETED;
				break;
			}

			/*
			 * Move onto the next segment.  If there isn't one then
			 * we succeeded.
			 */
			i2c_context->current_segment++;
			if (i2c_context->i2c_cmdbuf_p->
				segment_lengths[i2c_context->current_segment])
			{
				new_state = I2C_INACTIVE;
				PRINT_I2C("chk I2C_STOP_SDAHI: new_segment #%d\n", i2c_context->current_segment);
			}
			else
			{
				/*
				 * Move to the new state COMPLETED.
				 */
				i2c_context->i2c_cmdbuf_p->result =
						I2C_RESULT_SUCCESS;
				new_state = I2C_COMPLETED;
				PRINT_I2C("chk I2C_STOP_SDAHI: completed\n");
			}
		}
		break;

	default:
		PRINT_I2C("default case: unknown state %d\n",
				i2c_context->i2c_state);
		i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_INTERNALERR;
		new_state = I2C_COMPLETED;
		break;
	}

	i2c_context->clocks++;
	if (i2c_context->clocks >= MAX_CLOCKS_ANY_STATE)
	{
		/*
		 * Look at what state we were in when we detected to many clocks
		 * and work out what result to send.
		 *
		 * Any state where we're waiting for the bus to go high but it
		 * stays low is a time-out situation which normally occurs when
		 * a slave gets stuck and doesn't release the clock line.
		 *
		 * Any state where we try to take the bus low but it stays high
		 * is a stuck bus - usually a slave or a short on the bus is
		 * taken the line high through a low impedance.
		 *
		 * Any other state indicates an internal error.
		 */
		PRINT_I2C("too many clocks (%d) in this state (%d).\n",
				i2c_context->clocks,
				i2c_context->i2c_state);
		switch (i2c_context->i2c_state)
		{
		case I2C_START_CLKWAIT:
		case I2C_DATA_CLKWAIT:
		case I2C_ACK_CLKWAIT:
			i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_TIMEDOUT;
			break;

		case I2C_INACTIVE:
			i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_BUSBUSY;
			break;

		case I2C_START_SDALO:
		case I2C_START_SCLLO:
		case I2C_DATA_CLKLO:
		case I2C_ACK_CLKLO:
			i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_STUCKBUS;
			break;

		default:
			i2c_context->i2c_cmdbuf_p->result = I2C_RESULT_INTERNALERR;
		}
		new_state = I2C_COMPLETED;
	}
	if (new_state == i2c_context->i2c_state)
	{
		udelay(i2c_context->usec_delay);
		return;
	}

	/*
	 * Reset the clock counter for the new state.
	 */
	i2c_context->clocks = 0;

	/*
	 * Now handle the new state we're in.
	 */
	switch (new_state)
	{
	case I2C_START_SDALO:
		/*
		 * Take SDA low
		 */
		PRINT_I2C("new I2C_START_SDALO\n");
		i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_sda_w);
		break;

	case I2C_START_SCLLO:
		/*
		 * Take SCL low
		 */
		PRINT_I2C("new I2C_START_SCLLO\n");
		i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_scl_w);
		break;

	case I2C_START_CLKWAIT:
		/*
		 * Do nothing - the clock line is already low.  Just wait.
		 */
		PRINT_I2C("new I2C_START_CLKLO\n");
		break;

	case I2C_DATA_TX:
		/*
		 * Set the SDA bit to the correct value for the current bit and
		 * set the SCL bit high.
		 */
		if (*(i2c_context->current_byte_p) & i2c_context->current_bit_mask)
		{
			i2c_bitbang_set_scl_sda(dipp, dipp->dip_sda_w, 0);
		}
		else
		{
			i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_sda_w);
		}
		i2c_bitbang_set_scl_sda(dipp, dipp->dip_scl_w, 0);
		PRINT_I2C("new I2C_DATA_TX: DATA[bitmask=0x%x]=%d\n",
				i2c_context->current_bit_mask,
				(I2cReg & dipp->dip_sda_r) ? 1 : 0);
		break;

	case I2C_DATA_RX:
		/*
		 * Do nothing but take clock high and data lines high.
		 * We wait until the exit state check to actually check
		 * the SDA line.
		 */
		PRINT_I2C("new I2C_DATA_RX\n");
		i2c_bitbang_set_scl_sda(dipp, dipp->dip_sda_w, 0);
		i2c_bitbang_set_scl_sda(dipp, dipp->dip_scl_w, 0);
		break;

	case I2C_DATA_CLKWAIT:
		/*
		 * Do nothing - the clock should be high but might not be if
		 * the slave needs more time.  The exit-state check wil decide
		 * wether we need to move on.
		 */
		PRINT_I2C("new I2C_DATA_CLKWAIT\n");
		break;

	case I2C_DATA_CLKLO:
		/*
		 * Take the clock bit low again.
		 */
		PRINT_I2C("new I2C_DATA_CLKLO\n");
		i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_scl_w);
		i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_sda_w);
		break;

	case I2C_ACK_TX:
		/*
		 * Transmit an acknowledgement bit (for byte reception).
		 * Set the SDA bit to the correct value.  This a 0 unless we're
		 * receiving the last byte in this segment.
		 */
		if (!(i2c_context->i2c_cmdbuf_p->options & I2C_OPTIONS_FORCEACK) &&
			(i2c_context->current_byte_no + 1 >=
					i2c_context->current_segment_length))
		{
			i2c_bitbang_set_scl_sda(dipp, dipp->dip_sda_w, 0);
		}
		else
		{
			i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_sda_w);
		}
		i2c_bitbang_set_scl_sda(dipp, dipp->dip_scl_w, 0);
		PRINT_I2C("new I2C_ACK_TX: ACK=%d\n",
				(I2cReg & dipp->dip_sda_w) ? 1 : 0);
		break;

	case I2C_ACK_RX:
		/*
		 * Do nothing but take clock and data lines high.
		 * We wait until the exit state check to actually check the
		 * SDA line.
		 */
		PRINT_I2C("new I2C_ACK_RX\n");
		i2c_bitbang_set_scl_sda(dipp, dipp->dip_scl_w | dipp->dip_sda_w, 0);
		break;

	case I2C_ACK_CLKWAIT:
		/*
		 * Do nothing - the clock should be high but might not be if
		 * the slave needs more time.  The exit-state check wil decide
		 * wether we need to move on.
		 */
		PRINT_I2C("new I2C_ACK_CLKWAIT\n");
		break;

	case I2C_ACK_CLKLO:
		/*
		 * Take the clock bit low again.
		 */
		PRINT_I2C("new I2C_ACK_CLKLO\n");
		i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_scl_w);
		i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_sda_w);
		break;

	case I2C_STOP_SCLLO:
		/*
		 * Take the clock and data bits low
		 */
		PRINT_I2C("new I2C_STOP_SCLLO\n");
		i2c_bitbang_set_scl_sda(dipp, 0, dipp->dip_scl_w | dipp->dip_sda_w);
		break;

	case I2C_STOP_SCLHI:
		/*
		 * Take the clock bit high
		 */
		PRINT_I2C("new I2C_STOP_SCLHI\n");
		i2c_bitbang_set_scl_sda(dipp, dipp->dip_scl_w, 0);
		break;

	case I2C_STOP_SDAHI:
		/*
		 * Take the data bit high
		 */
		PRINT_I2C("new I2C_STOP_SDAHI\n");
		i2c_bitbang_set_scl_sda(dipp, dipp->dip_sda_w, 0);
		break;

	case I2C_COMPLETED:
		PRINT_I2C("new I2C_COMPLETED: result=%d\n",
				i2c_context->i2c_cmdbuf_p->result);
		break;

	default:
		break;
	}

	if (new_state != I2C_COMPLETED)
	{
		udelay(i2c_context->usec_delay);
	}
	i2c_context->i2c_state = new_state;
	//PRINT_I2C("I2c-execute: exiting in state %d\n", new_state);
}


/*******************************************************************************
 *
 * Function:    dpci_i2c_fixbus()
 *
 * Parameters:  i2c_cmdbuf_p - the i2c buffer structure with command to execute.
 *
 * Returns:     int - -error number or 0.
 *
 * Description: This handles special i2c buffers with the UNLOCK option set,
 *		such packets being initiated to fix stuck buses.  (Busses get
 *		stuck for a wide variety of reasons, from invalid slave use to
 *		basic i2c protocol violations.)
 *
 ******************************************************************************/
static int dpci_i2c_fixbus(struct dpci_device *ddp, struct dpci_i2c_cmdbuf *i2c_cmdbuf_p)
{
	struct dpci_i2c_context i2c_context;
	char dummies[2];
	int attempts = MAX_RWBYTE_ATTEMPTS;

	/*
	 * Ensure the request bus actually exists.
	 */
	if (i2c_cmdbuf_p->bus < 0 ||
		i2c_cmdbuf_p->bus >= MAX_I2C_PORTS ||
		!ddp->i2c_ports[i2c_cmdbuf_p->bus].dip_reg)
	{
		i2c_cmdbuf_p->result = I2C_RESULT_NOBUS;
		return -ENODEV;
	}

	PRINT_I2C("Attempting to fix bus #%d\n", i2c_cmdbuf_p->bus);
	i2c_cmdbuf_p->options = 0; // I2C_OPTIONS_FORCEACK;
	i2c_cmdbuf_p->segment_lengths[0] = 2;
	i2c_cmdbuf_p->segment_lengths[1] = 0;
	i2c_cmdbuf_p->result = I2C_RESULT_INPROGRESS;
	i2c_context.usec_delay = 20;
	i2c_context.i2c_cmdbuf_p = i2c_cmdbuf_p;
	i2c_context.port = &ddp->i2c_ports[i2c_cmdbuf_p->bus];

	/*
	 * Claim the i2c bus for ourselves.
	 */
	if (down_interruptible(&i2c_context.port->dip_sem) != 0)
	{
		return -ERESTARTSYS;
	}

	while (attempts--)
	{
		PRINT_I2C("Attempt #%d\n", 3 - attempts);
		i2c_context.i2c_state = I2C_DATA_TX;
		i2c_context.clocks = 0;
		i2c_context.total_clocks = 0;
		i2c_context.current_segment = 0;
		i2c_context.current_address = 0;
		i2c_context.current_segment_length = 2;
		i2c_context.current_bit_mask = 0x80;
		i2c_context.current_byte_no = 1;
		i2c_context.current_byte_p = dummies;
		do
		{
			i2c_context.total_clocks++;
			if (i2c_context.total_clocks >= MAX_TOTAL_CLOCKS)
			{
				PRINT_I2C("transaction taking too long.\n");
				i2c_cmdbuf_p->result = I2C_RESULT_TIMEDOUT;
				i2c_context.i2c_state = I2C_COMPLETED;
			}
			i2c_bitbang_execute(&i2c_context);

		} while (i2c_context.i2c_state != I2C_COMPLETED);

		/*
		 * If the command failed and there's yet another attempt left
		 * at it then we wait for 50ms.
		 */
		if (i2c_cmdbuf_p->result == I2C_RESULT_SUCCESS)
		{
			break;
		}
		else if (attempts)
		{
			wait_event_interruptible_timeout(ddp->i2c_wqh, 0, INTER_ATTEMPT_DELAY_MS);
		}
	}

	/*
	 * Claim the i2c bus for ourselves.
	 */
	up(&i2c_context.port->dip_sem);
	return 0;
}


/*******************************************************************************
 *
 * Function:    i2c_bitbang_init()
 *
 * Parameters:  i2c_port_p - the port to initialise.
 *
 * Returns:     nothing
 *
 * Description: Initialises the port concerned.
 *
 ******************************************************************************/
void i2c_bitbang_init(struct dpci_i2c_port *i2c_port_p)
{
	i2c_bitbang_set_scl_sda(i2c_port_p,
			i2c_port_p->dip_sda_w | i2c_port_p->dip_scl_w,
			0);
}


/*******************************************************************************
 *
 * Function:    i2c_bitbang_command()
 *
 * Parameters:  i2c_cmdbuf_p - the i2c buffer structure with command to execute.
 *
 * Returns:     nothing
 *
 * Description: Sends a command over the selected i2c bus.  We simply prepare
 *              an i2c context and pass the hardwark off to the state engine.
 *              If the bus is invalid then we return -ENODEV.  However, any
 *              other failures at the I2c level are handled by setting the
 *              buffer's result member and returning 0.
 *
 ******************************************************************************/
int i2c_bitbang_command(struct dpci_device *ddp,
			struct dpci_i2c_cmdbuf *i2c_cmdbuf_p)
{
	struct dpci_i2c_context i2c_context;
	int attempts = 0;

	if (i2c_cmdbuf_p->options & I2C_OPTIONS_UNLOCK)
	{
		return dpci_i2c_fixbus(ddp, i2c_cmdbuf_p);
	}

	/*
	 * Work out the bus speed.
	 */
	i2c_context.i2c_cmdbuf_p = i2c_cmdbuf_p;
	i2c_context.port = &ddp->i2c_ports[i2c_cmdbuf_p->bus];
	if (i2c_cmdbuf_p->speed_hz)
		i2c_context.usec_delay = 500000 / i2c_cmdbuf_p->speed_hz;
	else
		i2c_context.usec_delay = DEFAULT_CLK_PERIOD_US;
	PRINT_I2C("clock speed is %dHz (user requested %dHz), delay: %dus\n",
			2000000 / i2c_context.usec_delay,
			i2c_cmdbuf_p->speed_hz,
			i2c_context.usec_delay);

	/*
	 * Now attempt the transaction requested.
	 */
	while (i2c_cmdbuf_p->attempts--)
	{
		attempts++;
		PRINT_I2C("Attempt #%d\n", attempts);
		i2c_context.i2c_state = I2C_INACTIVE;
		i2c_context.clocks = 0;
		i2c_context.total_clocks = 0;
		i2c_context.current_segment = 0;
		i2c_context.current_byte_no = 0;
		i2c_context.current_byte_p = &i2c_cmdbuf_p->buffer[0];
		i2c_cmdbuf_p->result = I2C_RESULT_INPROGRESS;

		/*
		 * Claim the i2c bus for ourselves.
		 */
		if (down_interruptible(&i2c_context.port->dip_sem) != 0)
		{
			return -ERESTARTSYS;
		}
		do
		{
			i2c_context.total_clocks++;
			if (i2c_context.total_clocks >= MAX_TOTAL_CLOCKS)
			{
				PRINT_I2C("transaction taking too long.\n");
				i2c_cmdbuf_p->result = I2C_RESULT_TIMEDOUT;
				i2c_context.i2c_state = I2C_COMPLETED;
			}
			i2c_bitbang_execute(&i2c_context);

		} while (i2c_context.i2c_state != I2C_COMPLETED);

		/*
		 * Claim the i2c bus for ourselves.
		 */
		up(&i2c_context.port->dip_sem);

		/*
		 * If the command failed and there's yet another attempt left
		 * at it then we wait for 50ms.
		 */
		if (i2c_cmdbuf_p->result == I2C_RESULT_SUCCESS)
		{
			break;
		}
		else if (i2c_cmdbuf_p->attempts > 0)
		{
			wait_event_interruptible_timeout(ddp->i2c_wqh, 0, INTER_ATTEMPT_DELAY_MS);
		}
	}
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_i2c_command()
 *
 * Parameters:  arg - pointer to user memory containing buffer
 *
 * Returns:     int - -error number or 0.
 *
 * Description: Sends a command over the selected i2c bus.  We copy in data
 *              from user space and hand it off to the i2c bus handler via the
 *              dip_i2c_command function.  We copy back the entire input buffer
 *              since we can't be bothered to work out which segments were read
 *		or write.
 *
 ******************************************************************************/
int dpci_i2c_command(struct dpci_device *ddp, struct dpci_i2c_cmdbuf *i2c_cmdbuf_p)
{
	/*
	 * Ensure the request bus actually exists.
	 */
	if (i2c_cmdbuf_p->bus < 0 ||
		i2c_cmdbuf_p->bus >= MAX_I2C_PORTS ||
		!ddp->i2c_ports[i2c_cmdbuf_p->bus].dip_reg)
	{
		i2c_cmdbuf_p->result = I2C_RESULT_NOBUS;
		return 0;
	}
	return ddp->i2c_ports[i2c_cmdbuf_p->bus].dip_command(ddp, i2c_cmdbuf_p);
}


/*******************************************************************************
 *
 * Function:    user_i2c_command()
 *
 * Parameters:  arg - pointer to user memory containing buffer
 *
 * Returns:     int - -error number or 0.
 *
 * Description: Sends a command over the selected i2c bus.  We copy in data
 *              from user space and hand it off to the i2c bus handler via the
 *              dpci_i2c_command function.  We copy back the entire input buffer
 *              since we can't be bothered to work out which segments were read
 *		or write.
 *
 ******************************************************************************/
int user_i2c_command(struct dpci_device *ddp, unsigned long arg)
{
	int segno, bytes;
	struct dpci_i2c_cmdbuf i2c_buf;
	struct dpci_i2c_cmdbuf *i2c_cmdbuf_p = NULL;
	int ret;

	/*
	 * Copy over the buffer without any trailing data.
	 */
	if (copy_from_user(&i2c_buf, (void *)arg, sizeof(i2c_buf)))
	{
		return -EFAULT;
	}

	/*
	 * Work out how much total data there is.
	 */
	for (bytes = 0, segno = 0; segno < MAX_I2C_COMMAND_SEGS; segno++)
	{
		PRINT_IOC("seg %d: %d bytes\n", segno, i2c_buf.segment_lengths[segno]);
		if (!i2c_buf.segment_lengths[segno])
			break;
		bytes += i2c_buf.segment_lengths[segno];
	}
	PRINT_IOC("loading %d extra bytes from user land.\n", bytes);

	/*
	 * Allocate memory - enough for the buffer plus the original bytes.
	 */
	i2c_cmdbuf_p = (struct dpci_i2c_cmdbuf *)
			kmalloc(sizeof(*i2c_cmdbuf_p) + bytes - 1, GFP_KERNEL);
	if (!i2c_cmdbuf_p)
	{
		return -ENOMEM;
	}

	/*
	 * Now copy the original buffer and copy in the extra data.
	 */
	if (copy_from_user(i2c_cmdbuf_p, (void *)arg, sizeof(i2c_buf) + bytes - 1))
	{
		ret = -EFAULT;
		goto done;
	}

	/*
	 * Print the buffer.  We use printk() directly here to avoid having our
	 * driver splattered all over the output.
	 */
#ifdef DEBUG
	if (debug & 1)
	{
		int byte, segcntr, segno;
		for (byte = 0, segcntr = 0, segno = 0; byte < bytes; byte++)
		{
			if (segcntr == 0)
				PRINT_INFO("segment %d in: ", segno);
			printk("%02x ", i2c_cmdbuf_p->buffer[byte]);
			if (++segcntr == i2c_cmdbuf_p->segment_lengths[segno])
			{
				printk("\n");
				segcntr = 0;
				segno++;
			}
		}
	}
#endif

	/*
	 * Execute the command
	 */
	ret = dpci_i2c_command(ddp, i2c_cmdbuf_p);
	if (ret < 0)
		goto done;

	/*
	 * Print the buffer again
	 */
#ifdef DEBUG
	if (debug & 1)
	{
		int byte, segcntr, segno;
		for (byte = 0, segcntr = 0, segno = 0; byte < bytes; byte++)
		{
			if (segcntr == 0)
				PRINT_INFO("segment %d out: ", segno);
			printk("%02x ", i2c_cmdbuf_p->buffer[byte]);
			if (++segcntr == i2c_cmdbuf_p->segment_lengths[segno])
			{
				printk("\n");
				segcntr = 0;
				segno++;
			}
		}
	}
#endif

	ret = copy_to_user((void *)arg,
				i2c_cmdbuf_p,
				sizeof(*i2c_cmdbuf_p) + bytes - 1);
done:
	if (i2c_cmdbuf_p)
		kfree(i2c_cmdbuf_p);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_i2c_setup()
 *
 * Parameters:  dev - the input device being setup.
 *
 * Returns:	nothing
 *
 * Description: Initialises resources for EEPROM/I2C access.
 *		We ensure the correct interrupts are enabled.
 *
 ******************************************************************************/
void dpci_i2c_setup(struct dpci_device *ddp)
{
		int i, j = 0;

	init_waitqueue_head(&ddp->i2c_wqh);

	for (i = 0;
		ddp->mainboard->db_i2c_ports[i].dip_reg && i < MAX_I2C_PORTS;
		i++)
	{
		ddp->i2c_ports[j] = ddp->mainboard->db_i2c_ports[i];
		ddp->i2c_ports[j].dip_ddp = ddp;
		init_MUTEX(&ddp->i2c_ports[j].dip_sem);
		if (!ddp->i2c_ports[j].dip_sda_r)
			ddp->i2c_ports[j].dip_sda_r = ddp->i2c_ports[j].dip_sda_w;
		if (!ddp->i2c_ports[j].dip_scl_r)
			ddp->i2c_ports[j].dip_scl_r = ddp->i2c_ports[j].dip_scl_w;
		ddp->i2c_ports[j].dip_init(&ddp->i2c_ports[j]);
		PRINT_DBG("I2C bus #%d \"%s\" "
			"at reg %02x, (sda=%02x scl=%02x, sda_r=%02x scl_r=%02x)\n",
			j,
			ddp->i2c_ports[j].dip_desc,
			ddp->i2c_ports[j].dip_reg,
			ddp->i2c_ports[j].dip_sda_w,
			ddp->i2c_ports[j].dip_scl_w,
			ddp->i2c_ports[j].dip_sda_r,
			ddp->i2c_ports[j].dip_scl_r);
		j++;
	}
	if (ddp->ioboard)
	{
		for (i = 0;
			ddp->ioboard->db_i2c_ports[i].dip_reg &&
				i < MAX_I2C_PORTS;
			i++)
		{
			ddp->i2c_ports[j] = ddp->ioboard->db_i2c_ports[i];
			ddp->i2c_ports[j].dip_ddp = ddp;
			init_MUTEX(&ddp->i2c_ports[j].dip_sem);
			if (!ddp->i2c_ports[j].dip_sda_r)
				ddp->i2c_ports[j].dip_sda_r = ddp->i2c_ports[j].dip_sda_w;
			if (!ddp->i2c_ports[j].dip_scl_r)
				ddp->i2c_ports[j].dip_scl_r = ddp->i2c_ports[j].dip_scl_w;
			ddp->i2c_ports[j].dip_init(&ddp->i2c_ports[j]);
			PRINT_DBG("I2C bus #%d \"%s\" "
				"at reg %02x, (sda_w=%02x scl_w=%02x) "
				"(sda_r=%02x scl_r=%02x)\n",
				j,
				ddp->i2c_ports[j].dip_desc,
				ddp->i2c_ports[j].dip_reg,
				ddp->i2c_ports[j].dip_sda_w,
				ddp->i2c_ports[j].dip_scl_w,
				ddp->i2c_ports[j].dip_sda_r,
				ddp->i2c_ports[j].dip_scl_r);
			j++;
		}
	}
}

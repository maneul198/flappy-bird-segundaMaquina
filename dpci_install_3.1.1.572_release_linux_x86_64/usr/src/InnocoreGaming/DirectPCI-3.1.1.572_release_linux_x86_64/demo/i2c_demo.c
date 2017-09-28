/******************************************************************************
 *
 * $Id: i2c_demo.c 12926 2016-12-05 16:12:51Z amar $
 *
 * Copyright 2003-2015 Advantech Co. Ltd.
 * All rights reserved.
 *
 * Description:
 * DirectPCI diagnostic demo.
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
 *****************************************************************************/

#ifdef __linux
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "dpci_core_api.h"
#include "dpci_core_hw.h"
#include "dpci_version.h"
#include "dpci_boards.h"

#define	DEFAULT_SLAVE		0xa0		/* eeprom */
#define	DEFAULT_BUS		0
#define	DEFAULT_ATTEMPTS	1
#define	DEFAULT_OPTIONS		0
#define	DATA_BUF_SIZE		128
#define DEFAULT_PAGE_SIZE	32
#define DEFAULT_ADDR_SIZE	16

static int current_slave = DEFAULT_SLAVE;
static int current_bus = DEFAULT_BUS;
static int current_speed = 0;
static int max_attempts = DEFAULT_ATTEMPTS;
static int current_options = DEFAULT_OPTIONS;
static int address_size = DEFAULT_ADDR_SIZE;
static int page_size = DEFAULT_PAGE_SIZE;
static char current_bus_name[256];
static int max_i2c_buses = 0;

/*
 * This is the structure we use for storing i2c commands and data.  The data
 * follows on directly from the i2c command buf so that the buffer member
 * 'buffer' can be used to access all data.  As such, we don't access the
 * 'extra_data' member directly.
 */
static struct i2c_buf {
	struct dpci_i2c_cmdbuf dpci_i2c_buf;
	unsigned char extra_data[DATA_BUF_SIZE];
} my_i2c_buf;

static struct dpci_i2c_rwbuf my_rwbuf;
static unsigned char rwbuf_data[DATA_BUF_SIZE];

static void menu(void)
{
	static const char *api_version_string = NULL;
	static const char *drv_version_string = NULL;
	static const char *board_name = NULL;

	if (!api_version_string)
	{
		api_version_string = dpci_api_version_string();
		drv_version_string = dpci_drv_version_string();
		board_name = dpci_board_get_host_board_name();
	}
	printf("\n-------------------------------------------------------------------------------\n");
	printf("DirectPCI I2c Demo, $Revision: 12926 $\n");
	printf("(C) 2006-2015 Advantech Co. Ltd.  All rights reserved.\n");
   if (api_version_string)
	   printf("DLL v%s  ", api_version_string);
   else
	   printf("DLL version unknown  ");
   if (drv_version_string)
	   printf("Driver v%s  \n", drv_version_string);
   else
	   printf("Driver version unknown.  \n");
   printf("-------------------------------------------------------------------------------\n");
	printf(" 1. Select default bus                   (currently %d - %s).\n",
		current_bus,
		current_bus_name);
	printf(" 2. Select default slave address         (currently %02x).\n", current_slave);
	printf(" 3. Select # attempts                    (currently %d).\n", max_attempts);
	printf(" 4. Select speed (Hz)                    (currently %d).\n", current_speed);
	printf(" 5. Select options                       (currently %d).\n", current_options);
	printf(" 6. Set device's memory page size        (currently %d bytes)\n", page_size);
	printf(" 7. Set device's memory address size     (currently %d bits)\n", address_size);
	printf(" 8. Read 1 byte from 8-bit device slave   9. Write 1 byte to 8-bit device slave\n");
	printf("10. Read from slave with mem address     11. Write to slave with mem address\n");
	printf("12. Read from slave (raw read)           13. Write to slave (raw data)\n");
	printf("14. Attempt to fix crashed bus           15. Scan bus\n");
	printf("Q. Quit (or use Ctrl+C)\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("%s: %d I2C buses.\n", board_name, max_i2c_buses);
	printf("-------------------------------------------------------------------------------\n");
	printf("\n");
}

int main(void)
{
	char line[256] = "";
	int cmd = 0;
	int val = 0;
	int addr;
	int ret;
	int count, i;
	char tmpname[256];
	unsigned char byte;
	int tempnum;
	int i2c_error;

	if( dpci_core_open() < 0)
	{
		fprintf(stderr,
			"Error: Ensure the DirectPCI core driver is correctly installed, enabled and that you have the required access privileges.\n");
		return 1;
	}

	max_i2c_buses = dpci_i2c_numbuses();
	dpci_i2c_bus_name(current_bus, current_bus_name, sizeof current_bus_name);
	menu();
	while (1)
	{
		printf("Enter option (0 for menu): ");
		cmd = 0;
		fgets(line,sizeof(line),stdin);
		if (strlen(line) == 1)
		{
			continue;
		}
		if (tolower(line[0]) == 'q')
		{
			break;
		}
		sscanf(line,"%d", &cmd);

		switch (cmd)
		{
		case 0:
			menu();
			break;

		case 1:
			/*
			 * Display names of all buses.
			 */
			printf("Valid I2C busses (%d):\n", max_i2c_buses);
			for (val = 0; val < max_i2c_buses; val++)
			{
				if (dpci_i2c_bus_name(val,
							tmpname,
							sizeof(tmpname)) == 0)
				{
					printf("%d: %s\n", val, tmpname);
				}
				else
					break;
			}

			/*
			 * Invite user to select a new bus number.
			 */
			printf("Enter bus number [currently %d - %s]: ",
				current_bus, current_bus_name);
			fgets(line,sizeof(line),stdin);
			line[strlen(line) - 1] = 0; // remove CR/LF

			/*
			 * If the first character is a digit, assume a number
			 * has been provided.
			 */
			if (isdigit(*line))
			{
				val = atoi(line);
				if (val < 0 || val >= max_i2c_buses)
				{
					printf("Value %d ignored - invalid.\n",
						val);
					break;
				}
			}
			else
			{
				/*
				 * Otherwise attempt to convert the name to a
				 * number.
				 */
				val = dpci_i2c_bus_number(line);
				if (val == -1)	
				{
					printf("Name %s ignored - invalid.\n",
						line);
					break;
				}
			}

			/*
			 * Record current bus number and name.
			 */
			current_bus = val;
			dpci_i2c_bus_name(val,
						current_bus_name,
						sizeof (current_bus_name));
			break;

		case 2:
			/*
			 * Select a new slave number.
			 */
			printf("Enter slave name, '?', or address in hex with R/W bit=0 "
				"[currently %02x]: ",
				current_slave);
			fgets(line,sizeof(line),stdin);
			if (strcmp(line, "?\n") == 0)
			{
				printf("\nDefined slave names:\n\n");
				printf("mbee     m/b EEPROM                   32kb, bus 0, addr A0, page size 32\n");
				printf("altmbee  alternative m/b EEPROM       32kb, bus 0, addr AE, page size 32\n");
				printf("ioee     80-0062 I/O Board EEPROM     256 bytes, bus 1, addr A0, page size 8\n");
				printf("cbee     Eval Connector board EEPROM  256 bytes, bus 0, addr A6, page size 8\n");
				printf("\n");
				break;
			}
			if (strcmp(line, "mbee\n") == 0)
			{
				current_bus = 0;
				current_slave = 0xa0;
				address_size = 16;
				page_size = 32;
				break;
			}
			if (strcmp(line, "altmbee\n") == 0)
			{
				current_bus = 0;
				current_slave = 0xae;
				address_size = 16;
				page_size = 32;
				break;
			}
			if (strcmp(line, "ioee\n") == 0)
			{
				current_bus = 0;
				current_slave = 0xa0;
				address_size = 8;
				page_size = 8;
				break;
			}
			if (strcmp(line, "cbee\n") == 0)
			{
				current_bus = 0;
				current_slave = 0xa6;
				address_size = 8;
				page_size = 8;
				printf("\n*** If selecting the connector board EEPROM, you must still match the address\n");
				printf("*** on the connector board's DIP switches with the address selected here.\n\n");
				printf("*** You can also still change the address manually here by typing it.\n\n");
				break;
			}
			val = strtol(line, (char **)0, 16);
			if (val == 0 || val & 1)
			{
				printf("Value %d ignored - invalid.\n", val);
				break;
			}
			current_slave = val;
			break;

		case 3:
			/*
			 * Select number of attempts for commands
			 */
			printf("Enter maximum number of attempts at command "
				"[currently %d]: ", max_attempts);
			fgets(line,sizeof(line),stdin);
			val = atoi(line);
			if (val <= 0)
			{
				printf("Value %d ignored - invalid.\n", val);
				break;
			}
			max_attempts = val;
			break;

		case 4:
			/*
			 * Select clock speed
			 */
			printf("Enter clock speed in Hertz (Hz) "
				"[currently %d]: ", current_speed);
			fgets(line,sizeof(line),stdin);
			val = atoi(line);
			if (val < 0)
			{
				printf("Value %d ignored - invalid.\n", val);
				break;
			}
			current_speed = val;
			break;

		case 5:
			/*
			 * Select new options word.
			 */
			printf("Enter options word in hex "
				"[currently %08x]: ", current_options);
			fgets(line,sizeof(line),stdin);
			current_options = strtol(line, (char **)0, 16);
			break;

		case 6:
			/*
			 * Select memory page size.
			 */
			printf("Enter device's memory page size (#byte) "
				"[currently %d] (8/11/16): ", page_size);
			fgets(line,sizeof(line),stdin);
			tempnum = strtol(line, (char **)0, 0);
			if (tempnum > 256)
			{
				printf("Invalid page size %d\n", tempnum);
				break;
			}
			page_size = tempnum;
			break;

		case 7:
			/*
			 * Select memory address size.
			 */
			printf("Enter memory address size (#bits) for protocol "
				"[currently %d] (8/11/16): ", address_size);
			fgets(line,sizeof(line),stdin);
			tempnum = strtol(line, (char **)0, 0);

			if ( !((tempnum == 16) || (tempnum == 11) || (tempnum == 8)) )
			{
				printf("Invalid address size %d\n", tempnum);
				break;
			}

			address_size = tempnum;
			break;

		case 8:
			/*
			 * Read 1 byte from a device slave with an 8-bit memory address
			 */
			printf("Enter address in slave space to read (hex): ");
			fgets(line,sizeof(line),stdin);
			addr = strtol(line, (char **)0, 16);
			if (addr < 0 || addr > 0xff)
			{
				printf("Address %x ignored - invalid.\n", addr);
				break;
			}

			/*
			 * Run the i2c command.  Print the error if it fails.
			 */
			ret = dpci_i2c_read8(current_bus, current_slave, addr, &byte);
			if (ret < 0)
			{
				perror("dpci_i2c_read8");
			}
			i2c_error = dpci_i2c_last_error();
			printf("I2C command status: %d - %s\n",
				i2c_error,
				dpci_i2c_error_string(i2c_error));
			if (i2c_error != I2C_RESULT_SUCCESS)
			{
				break;
			}

			/*
			 * Print the returned data.
			 */
			printf("Returned data: %02x\n", byte);
			break;

		case 10:
			/*
			 * Read byte(s) from slave with memory addr.
			 *
			 * The maximum we can read back (presuming the slave
			 * allows it) is DATA_BUF_SIZE - 4.  (3 = 2 bytes for
			 * slave addressing + 2 bytes for 2-byte address.)
			 */

			/*
			 * Gather useful data from the user.
			 */
			printf("Enter # bytes to read (max %d): ",
				DATA_BUF_SIZE - 4);
			fgets(line,sizeof(line),stdin);
			count = atoi(line);
			if (count < 1 || count > DATA_BUF_SIZE - 4)
			{
				printf("Count %d ignored - invalid.\n", count);
				break;
			}

			printf("Enter address in slave space to read (hex): ");
			fgets(line,sizeof(line),stdin);
			addr = strtol(line, (char **)0, 16);
			if (addr < 0 || addr > 0xffff)
			{
				printf("Address %x ignored - invalid.\n", addr);
				break;
			}

			/*
			 * Prepare the buffer with the required data.
			 */
			my_rwbuf.bus = current_bus;
			my_rwbuf.slave = current_slave;
			my_rwbuf.buf = rwbuf_data;
			my_rwbuf.offset = addr;
			my_rwbuf.pagesize = page_size;
			my_rwbuf.count = count;
			my_rwbuf.options = current_options;
			my_rwbuf.speed_hz = current_speed;
			my_rwbuf.attempts = max_attempts;
			if (address_size == 16)
				my_rwbuf.options |= I2C_OPTIONS_2BYTEADDR;
			else if (address_size == 11)
				my_rwbuf.options |= I2C_OPTIONS_11BITADDR;
			else if (address_size == 8)
				my_rwbuf.options |= I2C_OPTIONS_1BYTEADDR;

			/*
			 * Initialise data to known pattern.
			 */
			for (i = 0; i < count; i++)
			{
				my_i2c_buf.dpci_i2c_buf.buffer[4 + i] = 0xff - i;
			}

			/*
			 * Run the i2c command.  Print the error if it fails.
			 */
			ret = dpci_i2c_read(&my_rwbuf);
			if (ret < 0)
			{
				/*
				 * For some reason a system failure occurred
				 * and the i2c operation did not execute.
				 */
				perror("dpci_i2c_read");
			}
			printf("I2C read command status: %d - %s\n",
				my_rwbuf.result,
				dpci_i2c_error_string(my_rwbuf.result));
			if (my_rwbuf.result != I2C_RESULT_SUCCESS)
			{
				break;
			}

			/*
			 * Print the returned data.
			 */
			printf("Returned data: \n");
			for (i = 0; i < count; i++)
			{
				printf("%02x ",
				   rwbuf_data[i]);
			}
			printf("\n");
			break;

		case 12:
			/*
			 * Read byte(s) from current slave/bus with no address.
			 *
			 * The maximum we can read back (presuming the slave
			 * allows it) is DATA_BUF_SIZE - 1.  (1 = 1 bytes for
			 * slave address.)
			 */

			/*
			 * Gather useful data from the user.
			 */
			printf("Enter # bytes to read (max %d): ",
				DATA_BUF_SIZE - 1);
			fgets(line,sizeof(line),stdin);
			count = atoi(line);
			if (count < 1 || count > DATA_BUF_SIZE - 1)
			{
				printf("Count %d ignored - invalid.\n", count);
				break;
			}
			/*
			 * Prepare the buffer with the required data.
			 * The list of segment lengths must be zero-terminated.
			 */
			my_i2c_buf.dpci_i2c_buf.bus = current_bus;
			my_i2c_buf.dpci_i2c_buf.speed_hz = current_speed;
			my_i2c_buf.dpci_i2c_buf.options = current_options;
			my_i2c_buf.dpci_i2c_buf.attempts = max_attempts;
			my_i2c_buf.dpci_i2c_buf.segment_lengths[0] = 1 + count;
			my_i2c_buf.dpci_i2c_buf.segment_lengths[1] = 0;
			my_i2c_buf.dpci_i2c_buf.buffer[0] = current_slave | 1;

			/*
			 * Initialise data to known pattern.
			 */
			for (i = 0; i < count; i++)
			{
				my_i2c_buf.dpci_i2c_buf.buffer[1 + i] = 0xff - i;
			}

			/*
			 * Run the i2c command.  Print the error if it fails.
			 */
			ret = dpci_i2c_command(&my_i2c_buf.dpci_i2c_buf);
			if (ret < 0)
			{
				perror("dpci_i2c_command");
			}
			printf("I2C command status: %d - %s\n",
				my_i2c_buf.dpci_i2c_buf.result,
				dpci_i2c_error_string(my_i2c_buf.dpci_i2c_buf.result));
			if (my_i2c_buf.dpci_i2c_buf.result != I2C_RESULT_SUCCESS)
			{
				break;
			}

			/*
			 * Print the returned data.
			 */
			printf("Returned data: \n");
			for (i = 0; i < count; i++)
			{
				printf("%02x ",
				   my_i2c_buf.dpci_i2c_buf.buffer[1 + i]);
			}
			printf("\n");
			break;

		case 9:
			/*
			 * Write 1 byte to device slave with 1-byte addr.
			 */

			/*
			 * Gather useful data from the user.
			 */
			printf("Enter address in slave space to write (hex): ");
			fgets(line,sizeof(line),stdin);
			addr = strtol(line, (char **)0, 16);
			if (addr < 0 || addr > 0xff)
			{
				printf("Address %x ignored - invalid.\n", addr);
				break;
			}

			/*
			 * Collect user data.
			 */
			printf("Enter byte to write (hex): ");
			fgets(line,sizeof(line),stdin);
			byte = (unsigned char)strtol(line, (char **)0, 16);

			/*
			 * Run the i2c command.  Print the error if it fails.
			 */
			ret = dpci_i2c_write8(current_bus,
						current_slave,
						addr,
						byte);
			if (ret < 0)
			{
				perror("dpci_i2c_write8");
			}
			i2c_error = dpci_i2c_last_error();
			printf("I2C command status: %d - %s\n",
				i2c_error,
				dpci_i2c_error_string(i2c_error));
			break;

		case 11:
			/*
			 * Write byte(s) to current slave/bus with memory addr.
			 *
			 * The maximum we can read back (presuming the slave
			 * allows it) is DATA_BUF_SIZE - 3.  (3 = 1 bytes for
			 * slave address + 2 bytes for 2-byte address.)
			 */

			/*
			 * Gather useful data from the user.
			 */
			printf("Enter # bytes to write (max %d): ",
				DATA_BUF_SIZE - 4);
			fgets(line,sizeof(line),stdin);
			count = atoi(line);
			if (count < 0 || count > DATA_BUF_SIZE - 3)
			{
				printf("Count %d ignored - invalid.\n", count);
				break;
			}

			printf("Enter address in slave space to write (hex): ");
			fgets(line,sizeof(line),stdin);
			addr = strtol(line, (char **)0, 16);
			if (addr < 0 || addr > 0xffff)
			{
				printf("Address %x ignored - invalid.\n", addr);
				break;
			}

			/*
			 * Prepare the buffer with the required data.
			 */
			my_rwbuf.bus = current_bus;
			my_rwbuf.slave = current_slave;
			my_rwbuf.buf = rwbuf_data;
			my_rwbuf.offset = addr;
			my_rwbuf.pagesize = page_size;
			my_rwbuf.count = count;
			my_rwbuf.options = current_options;
			my_rwbuf.speed_hz = current_speed;
			my_rwbuf.attempts = max_attempts;
			if (address_size == 16)
				my_rwbuf.options |= I2C_OPTIONS_2BYTEADDR;
			else if (address_size == 11)
				my_rwbuf.options |= I2C_OPTIONS_11BITADDR;
			else if (address_size == 8)
				my_rwbuf.options |= I2C_OPTIONS_1BYTEADDR;

			/*
			 * Collect user data.
			 */
			for (i = 0; i < count; i++)
			{
				printf("Enter byte #%d to write (hex): ", i);
				fgets(line,sizeof(line),stdin);
				byte = (unsigned char)strtol(line, (char **)0, 16);
				rwbuf_data[i] = byte;
			}

			/*
			 * Run the i2c command.  Print the error if it fails.
			 */
			ret = dpci_i2c_write(&my_rwbuf);
			if (ret < 0)
			{
				/*
				 * For some reason a system failure occurred
				 * and the i2c operation did not execute.
				 */
				perror("dpci_i2c_write");
			}
			printf("I2C write command status: %d - %s\n",
				my_rwbuf.result,
				dpci_i2c_error_string(my_rwbuf.result));
			break;

		case 13:
			/*
			 * Write raw byte(s) to current slave/bus with no addr.
			 *
			 * The maximum we can read back (presuming the slave
			 * allows it) is DATA_BUF_SIZE - 1.  (1 = 1 bytes for
			 * slave address.)
			 */

			/*
			 * Gather useful data from the user.
			 */
			printf("Enter # bytes to write (max %d): ",
				DATA_BUF_SIZE - 1);
			fgets(line,sizeof(line),stdin);
			count = atoi(line);
			if (count < 0 || count > DATA_BUF_SIZE - 1)
			{
				printf("Count %d ignored - invalid.\n", count);
				break;
			}

			/*
			 * Prepare the buffer with the required data.
			 * The list of segment lengths must be zero-terminated.
			 */
			my_i2c_buf.dpci_i2c_buf.bus = current_bus;
			my_i2c_buf.dpci_i2c_buf.options = current_options;
			my_i2c_buf.dpci_i2c_buf.speed_hz = current_speed;
			my_i2c_buf.dpci_i2c_buf.attempts = max_attempts;
			my_i2c_buf.dpci_i2c_buf.segment_lengths[0] = 1 + count;
			my_i2c_buf.dpci_i2c_buf.segment_lengths[1] = 0;
			my_i2c_buf.dpci_i2c_buf.buffer[0] = current_slave;

			/*
			 * Collect user data.
			 */
			for (i = 0; i < count; i++)
			{
				printf("Enter byte #%d to write (hex): ", i);
				fgets(line,sizeof(line),stdin);
				byte = (unsigned char)strtol(line, (char **)0, 16);
				my_i2c_buf.dpci_i2c_buf.buffer[1 + i] = byte;
			}

			/*
			 * Run the i2c command.  Print the error if it fails.
			 */
			ret = dpci_i2c_command(&my_i2c_buf.dpci_i2c_buf);
			if (ret < 0)
			{
				perror("dpci_i2c_command");
			}
			printf("I2C command status: %d - %s\n",
				my_i2c_buf.dpci_i2c_buf.result,
				dpci_i2c_error_string(my_i2c_buf.dpci_i2c_buf.result));
			break;

		case 14: 
			my_i2c_buf.dpci_i2c_buf.options = I2C_OPTIONS_UNLOCK;
			ret = dpci_i2c_command(&my_i2c_buf.dpci_i2c_buf);
			if (ret < 0)
			{
				perror("dpci_i2c_command");
			}
			printf("I2C command status: %d - %s\nslave ",
				my_i2c_buf.dpci_i2c_buf.result,
				dpci_i2c_error_string(my_i2c_buf.dpci_i2c_buf.result));
			break;

		case 15:
			{
				static int start = 0, end = 0;
				int slave, count = 0;

				printf("Enter start slave address in hex with R/W bit=0 [default %02x]: ", start);
				fgets(line,sizeof(line),stdin);
				val = strtol(line, (char **)0, 16);
				if (val & 1)
				{
					printf("Value %d ignored - invalid.\n", val);
					break;
				}
				if (val > 0)
					start = val;
				printf("Enter end slave address in hex with R/W bit=0 [default %02x]: ", end);
				fgets(line,sizeof(line),stdin);
				val = strtol(line, (char **)0, 16);
				if (val & 1)
				{
					printf("Value %d ignored - invalid.\n", val);
					break;
				}
				if (val > 0)
					end = val;
				if (start == 0)
					start = 0x0;
				if (end == 0)
					end = 0xfe;
				/*
				 * Prepare the buffer with the required data.
				 * The list of segment lengths must be zero-terminated.
				 */
				my_i2c_buf.dpci_i2c_buf.bus = current_bus;
				my_i2c_buf.dpci_i2c_buf.options = current_options;
				my_i2c_buf.dpci_i2c_buf.speed_hz = current_speed;
				my_i2c_buf.dpci_i2c_buf.attempts = max_attempts;
				my_i2c_buf.dpci_i2c_buf.segment_lengths[0] = 1;
				my_i2c_buf.dpci_i2c_buf.segment_lengths[1] = 0;

				for (slave = start; slave < end; slave += 2, count++)
				{
					if ((count & 7) == 0)
						printf("\n%02x: ", slave);
					if (slave == 0)
					{
						printf("   ");
						continue;
					}
					my_i2c_buf.dpci_i2c_buf.buffer[0] = slave;
					
					/*
					 * Run the i2c command.  Print the error if it fails.
					 */
					ret = dpci_i2c_command(&my_i2c_buf.dpci_i2c_buf);
					if (ret < 0)
					{
						printf("!!");
					}
					else if (my_i2c_buf.dpci_i2c_buf.result == I2C_RESULT_SUCCESS)
					{
						printf("%02x ", slave);
					}
					else
					{
						printf("-- ");
					}
				}
				printf("\n");
			}
			break;

		default:
			fprintf(stderr, "Not implemented yet.\n");
			break;
		} 
	}
	return 0;
}

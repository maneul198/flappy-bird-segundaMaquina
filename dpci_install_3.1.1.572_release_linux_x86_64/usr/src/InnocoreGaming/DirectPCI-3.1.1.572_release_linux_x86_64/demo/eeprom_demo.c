/******************************************************************************
 *
 * $Id: eeprom_demo.c 12370 2016-03-09 12:06:12Z james $
 *
 * Copyright 2005-2013 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * DirectPCI EEPROM demo.
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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>

#include "dpci_core_api.h"
#include "dpci_core_hw.h"
#include "dpci_version.h"
#include "dpci_boards.h"

#define EEPROM_MENU			0
#define EEPROM_READ_BYTE		1
#define EEPROM_WRITE_BYTE		2
#define EEPROM_WRITE_STRING	3
#define EEPROM_DUMP			4
#define EEPROM_CLEAR			5
#define EEPROM_SIZE			6


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
	printf("DirectPCI EEPROM Demo v" DPCI_VERSION ", $Revision: 12370 $\n");
	printf("(C) 2005-2015 Advantech Co Ltd. All Rights Reserved.\n");
   if (api_version_string)
	   printf("DLL v%s  ", api_version_string);
   else
	   printf("DLL version unknown  ");
   if (drv_version_string)
	   printf("Driver v%s  \n", drv_version_string);
   else
	   printf("Driver version unknown.  \n");
	printf("-------------------------------------------------------------------------------\n");
	printf(" 1. Read byte from EEPROM        2. Write byte to EEPROM\n");
	printf(" 3. Write string to EEPROM       4. Dump EEPROM contents\n");
	printf(" 5. Clear EEPROM                 6. Show size of EEPROM\n");
	printf(" Q. Quit (or use Ctrl+C)\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("%s: %d bytes of EEPROM space.\n", board_name, dpci_e2_size());
	printf("-------------------------------------------------------------------------------\n");
	printf("\n");
}

int main(void)
{
	char line[256];
	int cmd = 0;
	int val = 0;
	unsigned char val8;
	int offset = 0;

	if( dpci_core_open() < 0)
	{
		fprintf(stderr,
			"Error: Ensure the DirectPCI core driver is correctly installed, enabled and that you have the required access privileges.\n");
		return 1;
	}

	menu();
	while (1)
	{
		printf("Enter option (0 for menu): ");
		cmd = 0;
		fgets(line,sizeof(line),stdin);
		if (tolower(line[0]) == 'q')
		{
			break;
		}
		sscanf(line,"%d", &cmd);

		switch(cmd) {
		case EEPROM_MENU:
			menu();
			break;

		case EEPROM_READ_BYTE:
			printf("Enter offset to read from (Hex):");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%x", &offset);
			if (dpci_e2_read8(offset, &val8) == -1)
			{
				fprintf(stderr,
						"Can't read byte: sys error=%s, i2c error=%s\n",
						dpci_last_os_error_string(),
						dpci_i2c_error_string(dpci_i2c_last_error()));
			}
			else
			{
				printf("Read 0x%02x from offset 0x%02x\n", val8, offset);
			}
			break;

		case EEPROM_WRITE_BYTE:
			printf("Enter offset to write to (Hex):");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%x", &offset);
			printf("Enter value to write to address (Hex):");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%x", &val);
			if (dpci_e2_write8(offset, (unsigned char)val) == -1)
			{
				fprintf(stderr,
						"Can't write byte: sys error=%s, i2c error=%s\n",
						dpci_last_os_error_string(),
						dpci_i2c_error_string(dpci_i2c_last_error()));
			}
			else
			{
				printf("Written 0x%02x to address 0x%02x\n", val, offset);
			}
			break;

		case EEPROM_CLEAR:
			{
				unsigned char *zeromem;
				int size = 0;
				int blocksize = size;
				int offset;
				int ret;
				time_t start,end;

				printf("Enter size of EEPROM to clear (Hex, default is eeprom size):");
				fgets(line, sizeof(line), stdin);
				sscanf(line, "%x", &size);
				if (size == 0)
				{
					size = dpci_e2_size();
				}

				if (!size)
				{
					fprintf(stderr, "Cannot get size of EEPROM\n");
					break;
				}

				printf("Enter size of blocks to clear (Hex, default is eeprom size):");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%x", &blocksize);
				if (blocksize == 0)
				{
					blocksize = size;
				}

				/*
				 * Allocate and clear some memory for copying to eeprom.
				 */
				zeromem = (unsigned char *)malloc(blocksize);
				if (!zeromem)
				{
					fprintf(stderr,
							"Couldn't allocate %d bytes.\n",
							size);
					break;
				}
				memset(zeromem, 0, blocksize);

				/*
				 * Clear the eeprom in blocks of the requested size.
				 */
				printf("Clearing EEPROM: %d bytes in blocks of %d bytes.",
						size,
						blocksize);
				fflush(stdout);
				time(&start);
				for (offset = 0; offset < size; offset += blocksize)
				{
					int chunk;

					chunk = blocksize;
					if (offset + chunk > size)
						chunk = size - offset;
					ret = dpci_e2_write(offset, zeromem, chunk);
					if (ret == -1)
					{
						fprintf(stderr,
								"Can't write %d bytes to offset 0x%x: "
								"sys error=%s, i2c error=%s\n",
								blocksize,
								offset,
								dpci_last_os_error_string(),
								dpci_i2c_error_string(dpci_i2c_last_error()));
					}
					else if (ret != chunk)
					{
						fprintf(stderr,
								"\nError writing block to EEPROM - only %d bytes written\n",
								ret);
					}
					else
					{
						printf(".");
						fflush(stdout);
					}
				}
				time(&end);
				printf("done.\n");
				printf("%d bytes in %d seconds - %d bytes/second.\n",
						size,
						(int)(end - start),
						(int)(size / (end - start)));
				free(zeromem);
			}
			break;

		case EEPROM_DUMP:
			{
#define BYTES_PER_LINE	16
				unsigned char *data;
				int bytes = 0;
				int start_offset;
				int got;

				printf("Enter address to dump from (in hex): ");
				fgets(line, sizeof(line),stdin);
				sscanf(line,"%x", &offset);
				printf("Enter # bytes to dump (in hex): ");
				fgets(line, sizeof(line),stdin);
				sscanf(line,"%x", &bytes);

				if (bytes <= 0)
				{
					break;
				}

				/*
				 * Allocate memory for the whole amount of data to be dumped.
				 */
				data = (unsigned char *) malloc(bytes);
				if (!data)
				{
					perror("Can't allocate buffer memory");
					break;
				}

				/*
				 * Now read the data.
				 */
				got = dpci_e2_read(offset, data, bytes);
				if (got == -1)
				{
					fprintf(stderr,
							"Error reading block from EEPROM at offset 0x%x: sys error=%s, i2c error=%s\n",
							offset,
							dpci_last_os_error_string(),
							dpci_i2c_error_string(dpci_i2c_last_error()));
					break;
				}
				if (got == 0)
				{
					printf("No bytes\n");
					break;
				}

				/*
				 * Start display the data.
				 */
				start_offset = offset;
				offset = 0;
				do {
					int i;
					int chunk;

					chunk = bytes;
					if (chunk > got)
					{
						chunk = got;
					}
					if (chunk > BYTES_PER_LINE)
					{
						chunk = BYTES_PER_LINE;
					}
					printf("%06x: ", start_offset + offset);
					for (i = 0; i < BYTES_PER_LINE; i++)
					{
						if (i < got)
							printf("%02x ", data[offset + i]);
						else
							printf("   ");
					}
					printf("  ");
					for (i = 0; i < BYTES_PER_LINE; i++)
					{
						if (i < got)
							printf("%c",
								isprint(data[offset + i]) ? data[offset + i] : '.');
					}
					printf("\n");
					offset += chunk;
					bytes -= chunk;
					got -= chunk;
				} while (got);
				free(data);
			}
			break;

		case EEPROM_WRITE_STRING:
			{
				int ret;
				int size;

				printf("Enter address to write string to (in hex): ");
				fgets(line, sizeof(line),stdin);
				sscanf(line,"%x", &offset);
				printf("Enter text string: ");
				fgets(line, sizeof(line),stdin);
				size = strlen(line);
				line[strlen(line) - 1] = 0;
				if (size == 1)
				{
					break;
				}
				ret = dpci_e2_write(offset, (unsigned char *)line, size);
				if (ret == -1)
				{
					fprintf(stderr,
							"\nError writing string to EEPROM: %s\n",
							dpci_i2c_error_string(dpci_i2c_last_error()));
				}
				else if (ret < size)
				{
					fprintf(stderr,
							"\nError writing string to EEPROM - only %d bytes written\n",
							ret);
				}
			}
			break;		

		case EEPROM_SIZE:
			printf("EEPROM size is %d bytes.\n", dpci_e2_size());
			break;
		} 
	}
	return 0;
}

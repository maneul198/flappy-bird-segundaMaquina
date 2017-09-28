/******************************************************************************
 *
 * $Id: rom_demo.c 9995 2013-06-25 09:07:27Z richard $
 *
 * Copyright 2003-2013 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * DirectPCI ROM slot demo.
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

#ifdef WIN32
#include <windows.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#include "dpci_rom_api.h"

#define ROM_MENU			0
#define	ROM_SIZE			1
#define ROM_READ_BYTE		2
#define ROM_READ_WORD		3
#define ROM_READ_DWORD		4
#define ROM_DUMP			5
#define ROM_DIRECT_DUMP		6

/*******************************************************************************
 *
 * Function:    menu()
 *
 * Parameters:  none.
 *
 * Returns:     nothing.
 *
 * Description: prints a menu of options.
 *
 ******************************************************************************/
static void menu(void)
{
	printf("\n--------------------------------------------------------------------------------\n");
	printf("DirectPCI ROM API Demo, $Revision: 9995 $\n");
	printf("(C) 2004-2013 Advantech Co Ltd.  All rights reserved\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("1.  Get total size of ROM slot      2.  Read byte from rom\n");
        printf("3.  Read word from rom              4.  Read dword from rom\n");
	printf("5.  Dump ROM contents               6.  Dump ROM contents (mapped)\n");
	printf("Q.  Quit.\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("\n");
}


/*******************************************************************************
 *
 * Function:    main
 *
 * Parameters:  none.
 *
 * Returns:     nothing.
 *
 * Description: Function for handling user selection of tests.
 *
 *              No command line parameters are parsed so presently no automated
 *              testing is possible.
 *
 ******************************************************************************/
int main(void)
{
	unsigned int addr = 0;
	char line[256];
	int cmd = 0;
	int value = 0;
	unsigned int val32;
	unsigned short val16;
	unsigned char val8;

	menu();
	while (1)
	{
		printf("Please enter an option (0 for menu): ");
		cmd = 0;
		if(fgets(line,sizeof(line),stdin) == NULL)
		{
			break;
		}
		if (tolower(line[0]) == 'q')
		{
			break;
		}
		sscanf(line,"%d", &cmd);

		switch(cmd) {
		case ROM_MENU:
			menu();
			break;

		case ROM_SIZE:
			value = dpci_rom_size();
			if(value == -1)
			{
				printf("Could not obtain the ROM size"
					": %s.\n", dpci_last_os_error_string());
				break;
			}
			printf("The size of ROM slot is %u.\n",value);
			break;

		case ROM_READ_BYTE:
			printf("Enter address to read from (in hex): ");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%x", &addr);
			if (dpci_rom_read8(addr, &val8) == -1)
			{
				perror("Can't read byte");
			}
		    else
			{
				printf("Read 0x%02x from offset 0x%08x\n", val8, addr);
			}
			break;    

		case ROM_READ_WORD:
			printf("Enter address to read from (in hex): ");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%x", &addr);
			if (dpci_rom_read16(addr, &val16) == -1)
			{
				perror("Can't read byte");
			}
			else
			{
				printf("Read 0x%04x from offset 0x%08x\n", val16, addr);
	        }
			break;    

		case ROM_READ_DWORD:
			printf("Enter address to read from (in hex): ");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%x", &addr);
	        if (dpci_rom_read32(addr, &val32) == -1)
			{
				perror("Can't read byte");
			}
		    else
			{
				printf("Read 0x%08x from offset 0x%08x\n", val32, addr);
	        }
			break;    

		case ROM_DUMP:
			{
#define BYTES_PER_LINE	16
				unsigned char rom[16];
				int bytes = 0;
				int got;

				printf("Enter address to dump from (in hex): ");
				fgets(line, sizeof(line),stdin);
				sscanf(line,"%x", &addr);
				printf("Enter # bytes to dump (in hex): ");
				fgets(line, sizeof(line),stdin);
				sscanf(line,"%x", &bytes);

				if (bytes <= 0)
				{
					break;
				}
				do {
					int i;
					int chunk;

					chunk = bytes;
					if (chunk > BYTES_PER_LINE)
						chunk = BYTES_PER_LINE;
					got = dpci_rom_read(addr, rom, chunk);
					if (got == -1)
					{
						fprintf(stderr,
								"Error reading block from ROM at address 0x%x: %s\n",
								addr, dpci_last_os_error_string());
						break;
					}
					if (got == 0)
					{
						break;
					}
					printf("%06x: ", addr);
					for (i = 0; i < BYTES_PER_LINE; i++)
					{
						if (i >= got)
							printf("   ");
						else
							printf("%02x ", rom[i]);
					}
					printf("  ");
					for (i = 0; i < got; i++)
					{
						printf("%c", isprint(rom[i]) ? rom[i] : '.');
					}
					printf("\n");
					addr += BYTES_PER_LINE;
					bytes -= BYTES_PER_LINE;
				} while (bytes > 0 && got == BYTES_PER_LINE);
			}
			break;

		case ROM_DIRECT_DUMP:
			{
				unsigned char *cp;
				int bytes = 0;
				unsigned int rom_size;

				/*
				 * Map ROM and access directly. This
				 * function should behave and perform
				 * in an identical manner to the
				 * ROM_DUMP function.
				 */
				printf("Enter start address (in hex): ");
				fflush(stdout);
				if(fgets(line, sizeof(line),stdin) == NULL)
				{
					break;
				}
				sscanf(line,"%x", &addr);
				printf("Enter # bytes to dump (in hex): ");
				if(fgets(line, sizeof(line),stdin) == NULL)
				{
					break;
				}
				sscanf(line,"%x", &bytes);

				rom_size = dpci_rom_size();


				/*
				 * Check bounds
				 */
				if(addr >= rom_size)
				{
					break;
				}

				if(addr + bytes >= rom_size)
				{
					bytes = rom_size - addr;
				}

				cp = dpci_rom_map();

				if(cp != (unsigned char*)NULL)
				{
					unsigned char *buf = malloc(bytes);

					if(buf != (unsigned char*)NULL)
					{
						cp += addr;
						memcpy(buf, cp, bytes);
						cp = buf;

						do {
							int i;

							printf("%06x: ", addr);
							for (i = 0; i < BYTES_PER_LINE; i++)
							{
								if (i >= bytes)
									printf("   ");
								else
									printf("%02x ", *(cp + i));
							}
							printf("  ");
							for (i = 0; i < BYTES_PER_LINE; i++)
							{
								unsigned char ch;

								if(i >= bytes)
								{
									break;
								}
								ch = *(cp + i);
								putchar(isprint(ch) ? ch : '.');
							}
							putchar('\n');
							cp += BYTES_PER_LINE;
							bytes -= BYTES_PER_LINE;
							addr += BYTES_PER_LINE;
						} while (bytes > 0);
					}
					dpci_rom_unmap();
				}
			}
		}
	}
	return 0;
}

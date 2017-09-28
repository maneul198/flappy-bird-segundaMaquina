/******************************************************************************
 *
 * $Id: idprom_demo.c 12370 2016-03-09 12:06:12Z james $
 *
 * Copyright 2013-2015 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * DirectPCI IDPROM demo.
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

#include <dpci_core_api.h>
#include "dpci_core_hw.h"
#include "dpci_version.h"
#include "dpci_boards.h"

enum
{
	IDPROM_MENU,
	IDPROM_READ,
	IDPROM_WRITE_STRING,
	IDPROM_SHOW_ID,
	IDPROM_PAGE_STATUS,
	IDPROM_LOCK_PAGE,
	IDPROM_PROT_STATUS,
	IDPROM_SET_PROTECT,
	IDPROM_READ_MFID,
	IDPROM_WRITE_MFID,
	IDPROM_READ_BYTE
};

const char *const lock_info[] =
{
	"Writable",
	"EEPROM mode",
	"Read-only"
};

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
	printf("DirectPCI IDPROM API Demo v" DPCI_VERSION ", $Revision$\n");
	printf("(C) 2013-2015 Advantech Co Ltd. All Rights Reserved.\n");
   if (api_version_string)
	   printf("DLL v%s  ", api_version_string);
   else
	   printf("DLL version unknown  ");
   if (drv_version_string)
	   printf("Driver v%s  \n", drv_version_string);
   else
	   printf("Driver version unknown.  \n");
	printf("-------------------------------------------------------------------------------\n");
	printf(" 1. Read bytes from IDPROM       2. Write string to EEPROM\n");
	printf(" 3. Show IDPROM ID               4. Show memory page status\n");
	printf(" 5. Lock memory page             6. Show copy protection status\n");
        printf(" 7. Set copy-protect             8. Read manufacturer ID\n");
	printf(" 9. Set manufacturer ID         10. Read single byte from IDPROM\n");
	printf(" Q. Quit (or use Ctrl+C)\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("\n");
}

static int check_conf()
{
	char buffer[80];
	printf("*** This function cannot be undone! ***\n"
			"Please type \"YES\" to confirm: \n");

	if(fgets(buffer, sizeof buffer, stdin) == NULL)
	{
		return 0;
	}
	buffer[strlen(buffer) - 1] = '\0';
	if(!strcmp(buffer, "YES"))
	{
		return 1;
	}
	fprintf(stderr, "Function cancelled\n");
	return 0;
}

int main(void)
{
	char line[256];
	int cmd = 0;
	int rc;

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

		/*
		 * On EOF, exit
		 */
		if(fgets(line, sizeof(line), stdin) == NULL)
		{
			putchar('\n');
			break;
		}

		if (tolower(line[0]) == 'q')
		{
			break;
		}

		/*
		 * Failure here will trigger display of menu
		 */
		sscanf(line,"%d", &cmd);

		switch(cmd) {
			case IDPROM_MENU:
				menu();
				break;

			case IDPROM_READ:
				{
					unsigned int offset;
					size_t length;
					unsigned char data[128];
					unsigned int pos = 0;

					printf("Please enter address from which to read (hex):" );
					fflush(stdout);

					if(fgets(line, sizeof(line), stdin) == NULL)
					{
						break;
					}

					rc = sscanf(line, "%x", &offset);
					if(rc != 1)
					{
						break;
					}

					printf("Number of bytes to be read (hex):" );
					fflush(stdout);

					if(fgets(line, sizeof(line), stdin) == NULL)
					{
						break;
					}

					rc = sscanf(line, "%x", &length);
					if(rc != 1)
					{
						break;
					}

					rc = dpci_idprom_read(offset, data, length);

					if(rc < 0)
					{
						fprintf(stderr, "IDPROM read failed: %s\n", strerror(errno));
						break;
					}

					printf("IDPROM read %d bytes\n", rc);

					while(rc)
					{
						unsigned scan = 16;
						if(scan > rc)
						{
							scan = rc;
						}
						printf("%.4x: ", offset);
						for(length = 0; length < scan; length++)
						{
							printf("%.2x ", data[pos + length]);
						}

						for(length = 0; length <= (16 - scan) * 3; length++)
						{
							putchar(' ');
						}

						for(length = 0; length < scan; length++)
						{
							char ch = data[pos + length];
							putchar(isprint(ch) ? ch : '.');
						}
						putchar('\n');
						offset += scan;
						pos += scan;
						rc -= scan;
					}
				}

				break;

			case IDPROM_READ_BYTE:
				{
					unsigned int offset;
					unsigned char ch;
					
					printf("Please enter address from which to read (hex):" );
					fflush(stdout);

					if(fgets(line, sizeof(line), stdin) == NULL)
					{
						break;
					}

					rc = sscanf(line, "%x", &offset);
					if(rc != 1)
					{
						break;
					}

					rc = dpci_idprom_read8(offset, &ch);

					if(rc)
					{
						perror("dpci_idprom_read8");
						break;
					}
					printf("Value at byte %.4x is %.2x\n", offset, ch);
				}
				break;

			case IDPROM_WRITE_STRING:
				{
					unsigned int offset;

					printf("Please enter address at which to write (hex):" );
					fflush(stdout);

					if(fgets(line, sizeof(line), stdin) == NULL)
					{
						break;
					}

					rc = sscanf(line, "%x", &offset);
					if(rc != 1)
					{
						break;
					}


					printf("Please enter string:");
					if(fgets(line, sizeof line, stdin) == NULL)
					{
						break;
					}

					rc = dpci_idprom_write(offset, (unsigned char*)line, strlen(line));
					if(rc < 0)
					{
						fprintf(stderr, "Failed to write to IDPROM\n");
					}
				}
				break;

			case IDPROM_SHOW_ID:
				{
					unsigned char idpromid[8];

					if (dpci_idprom_readid(idpromid) == -1)
					{
						perror("Reading IDprom ID");
						break;
					}
					printf("ID PROM ID: %02x%02x%02x%02x%02x%02x%02x%02x\n",
							idpromid[0],
							idpromid[1],
							idpromid[2],
							idpromid[3],
							idpromid[4],
							idpromid[5],
							idpromid[6],
							idpromid[7]);
				}
				break;
			case IDPROM_PAGE_STATUS:
				{
					int i;
					int lock_value;

					for(i=0; i<4; i++)
					{
						lock_value = dpci_idprom_page_status(i);
						if(lock_value == -1)
						{
							continue;
						}

						switch(lock_value)
						{
							case LOCK_NONE:
								break;
							case LOCK_EEPROM:
								lock_value = 1;
								break;
							case LOCK_WRITEPROTECT:
								lock_value = 2;
								break;
							default:
								fprintf(stderr, "Fatal error\n");
						}

						printf("Page %d: %s\n", i, lock_info[lock_value]);
					}
					putchar('\n');
				}
				break;
			case IDPROM_LOCK_PAGE:
				{
					unsigned int page;
					int mode;

					printf("Please select page id [0-3]:");
					fflush(stdout);

					if(fgets(line, sizeof(line), stdin) == NULL)
					{
						break;
					}

					rc = sscanf(line, "%u", &page);

					if(rc != 1 || page >= 4)
					{
						break;
					}

					printf("Please select \n\t0) Read-only\n\t1) EPROM emulation mode\n");
					fflush(stdout);

					if(fgets(line, sizeof(line), stdin) == NULL)
					{
						break;
					}

					rc = sscanf(line, "%u", &mode);

					if(rc != 1 || mode < 0 || mode >= 2)
					{
						break;
					}

					mode = mode ? LOCK_WRITEPROTECT : LOCK_EEPROM;

					if(check_conf())
					{
						printf("Set page lock status\n");
						rc = dpci_idprom_lock_page(page, mode);
						if(rc == -1)
						{
							perror("dpci_idprom_lock_page");
						}
					}
				}
				break;
			case IDPROM_PROT_STATUS:
				{
					rc = dpci_idprom_get_copyprotect();

					if(rc == -1)
					{
						perror("dpci_idprom_get_copyprotect");
						continue;
					}

					printf("Copy protection is %s\n", rc ?  "on" : "off");
				}
				break;
			case IDPROM_SET_PROTECT:
				{
					if(check_conf())
					{
						rc = dpci_idprom_set_copyprotect();
						rc = 0;
						if(rc == -1)
						{
							perror("dpci_idprom_set_copyprotect");
						}
						else
						{
							printf("Copy protection is set\n");
						}
					}
				}
				break;
			case IDPROM_READ_MFID:
				{
					int mfid;
					mfid = dpci_idprom_read_mfid();
					if(mfid == -1)
					{
						perror("dpci_idprom_read_mfid");
						break;
					}
					printf("Manufacturer ID: 0x%.4x\n", mfid);
				}
				break;
			case IDPROM_WRITE_MFID:
				{
					int mfid;

					printf("Please enter MFID/User byte (hex): ");
					fflush(stdout);

					if(fgets(line, sizeof(line), stdin) == NULL)
					{
						break;
					}

					rc = sscanf(line, "%x", &mfid);
					if(rc != 1)
					{
						break;
					}

					mfid = dpci_idprom_write_mfid(mfid);
					if(mfid == -1)
					{
						perror("dpci_idprom_write_mfid");
						break;
					}
				}
				break;

		} 
	}
	return 0;
}

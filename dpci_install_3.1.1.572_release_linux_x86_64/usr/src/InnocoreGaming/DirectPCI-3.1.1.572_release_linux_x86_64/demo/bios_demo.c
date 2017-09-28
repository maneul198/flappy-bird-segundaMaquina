/******************************************************************************
 *
 * $Id: bios_demo.c 12286 2016-01-13 09:37:16Z amar $
 *
 * Copyright 2011-2015 Advantech Co. Ltd.
 * All rights reserved.
 *
 * Description:
 * DPCI demo program to read system bios memory
 *
 *****************************************************************************/

#ifdef __linux
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <dpci_version.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#include "dpci_core_api.h"
#include "dpci_core_hw.h"
#include "dpci_version.h"
#include "dpci_boards.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

#define DEFAULT_FILE_NAME	"dpx_bios_dump.bin"
#define BIOS_MENU			0
#define BIOS_DUMP			1
#define BIOS_SIZE			2

/*******************************************************************************
*
* Function:    menu()
*
* Parameters:  none
*
* Returns:     nothing
*
* Description: prints out a menu of how to use the bios demo
*
******************************************************************************/
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
	printf("DirectPCI BIOS Demo v" DPCI_VERSION ", $Revision: 12286 $\n");
	printf("(C) 2010-2016 Advantech Co Ltd.  All rights reserved.\n");
	if (api_version_string)
		printf("DLL v%s  ", api_version_string);
	else
		printf("DLL version unknown  ");
	if (drv_version_string)
		printf("Driver v%s  \n", drv_version_string);
	else
		printf("Driver version unknown.  \n");
	printf("-------------------------------------------------------------------------------\n");
	printf("1. Dump BIOS to \"file_name\" or " DEFAULT_FILE_NAME "\n");
	printf("2. Get size\n");
	printf("Q. Quit (or use Ctrl+C)\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("%s.\n", board_name);
	printf("-------------------------------------------------------------------------------\n");
	printf("\n");
}

/*******************************************************************************
 *
 * Function:    main()
 *
 * Parameters:  None
 *
 * Returns:     int.  0 for success, non-0 for failures.
 *
 * Description: Main body of utility. Checks invocation to see if it matches
 *              one of the supported patterns. 
 *
 ******************************************************************************/
int main()
{
	char bin_filename[1024];
	unsigned long bios_size;
	int cmd = 0;
	char line[256] = "";
	char file_name[256] =  "";

	if (dpci_core_open() < 0)
	{
		fprintf(stderr,
			"Error: Ensure the DirectPCI core driver is correctly installed, enabled and that you have the required access privileges.\n");
		return 1;
	}

	if (dpci_bios_size(&bios_size) != 0)
	{
		fprintf(stderr,
			"ERROR: Failed to get bios size.: %s\n",
			dpci_last_os_error_string());
		exit(1);
	}

	menu();

	while (1)
	{
		printf("Enter option (0 for menu): ");
		cmd = 0;
		fgets(line, sizeof(line), stdin);
		if (tolower(line[0]) == 'q')
		{
			break;
		}
		sscanf(line, "%d", &cmd);

		switch (cmd) {
		case BIOS_MENU:
			menu();
			break;
		case BIOS_DUMP:
		{
			static unsigned char *bios_mem_dump = NULL;
			long ret = 0;
			FILE *fp = NULL;

			printf("Enter the File Name: ");
			fgets(file_name, sizeof(file_name), stdin);
			file_name[strlen(file_name) - 1] = 0; // remove CR/LF

			if (strlen(file_name) == 0) {
				strcpy(bin_filename, DEFAULT_FILE_NAME);
			}
			else
			{
				strcpy(bin_filename, file_name);
			}

			bios_mem_dump = (unsigned char*)malloc(bios_size);
			memset(bios_mem_dump, 0, bios_size);

			fp = fopen(bin_filename, "wb");
			if (!fp)
			{
				fprintf(stderr,
					"ERROR: Failed to create %s: %s\n",
					bin_filename,
					dpci_last_os_error_string());
				break;
			}

			printf("Dumping %ld bytes to %s\n",
				bios_size,
				bin_filename);
			ret = dpci_bios_dump(bios_mem_dump, &bios_size);
			if (ret == -1)
			{
				fprintf(stderr,
					"ERROR: Reading BIOS memory: %s\n",
					dpci_last_os_error_string());
				goto done;
			}
			else if (ret == 0)
			{
				fprintf(stderr,
					"ERROR: Buffer too small. Required buffer length = 0x%lx\n",
					bios_size);
				goto done;
			}

			fwrite(bios_mem_dump, bios_size, 1, fp);
		done:
			free(bios_mem_dump);
			fclose(fp);
		}
		break;
		case BIOS_SIZE:
			printf("%ld bytes\n", bios_size);
			break;
		default:
			printf("Not Implemented\n");
		}
	}

	return 0;
}

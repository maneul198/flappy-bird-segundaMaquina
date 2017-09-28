/******************************************************************************
 *
 * $Id: batt_demo.c 12370 2016-03-09 12:06:12Z james $
 *
 * Copyright 2005-2015 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * DirectPCI Battery demo.
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dpci_core_api.h"
#include "dpci_core_hw.h"
#include "dpci_version.h"
#include "dpci_boards.h"

#define BAT_MENU			0
#define BAT_COUNT			1
#define BAT_NAME			2
#define BAT_NUMBER			3
#define BAT_SHOW_ALL		4
#define BAT_LEVEL			5
#define BAT_STATUS			6
#define BAT_SET_CHECKPERIOD	7
#define BAT_SET_ERRORLEVEL	8
#define BAT_GET_ERRORLEVEL	9

static int bat_count = 0;

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
	printf("DirectPCI Battery Demo v" DPCI_VERSION ", $Revision: 12370 $\n");
	printf("(C) 2005-2015 Advantech Co Ltd. All Rights Reserved.\n");
   if (api_version_string)
	   printf("DLL v%s  ", api_version_string);
   else
	   printf("DLL version unknown  ");
   if (drv_version_string)
	   printf("Driver v%s  \n", drv_version_string);
   else
	   printf("Driver version unknown.  \n");
	printf("--------------------------------------------------------------------------------\n");
	printf(" 1. Get Number of Batteries		2. Get Battery Name\n");
	printf(" 3. Get Battery Number			4. Show All Batteries\n");
	printf(" 5. Get Battery Level			6. Get Battery Status\n");
	printf(" 7. Set Battery Check Period\n");
	printf(" 8. Set Battery Error Level		9. Get Battery Error Level\n");
	printf(" Q. Quit (or use Ctrl+C)\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("%s: %d batteries present.\n", board_name, bat_count);
	printf("-------------------------------------------------------------------------------\n");
	printf("\n");
}

int main(void)
{
	char line[256];
	char bat_name[256];
	int bat_no;
	int bat_level;
	int bat_status;
	int ret;
	int cmd = 0;
		
	if( dpci_core_open() < 0)
	{
		fprintf(stderr,
			"Error: Ensure the DirectPCI core driver is correctly installed, enabled and that you have the required access privileges.\n");
		return 1;
	}
	bat_count = dpci_bat_num_batteries();
	if (bat_count < 0)
	{
		fprintf(stderr,
			"Unable to get count of batteries: %s\n",
			dpci_last_os_error_string());
		return 1;
	}
	if (bat_count == 0)
	{
		fprintf(stderr,
			"WARNING: No Batteries available on the board\n");
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
		case BAT_MENU:
			menu();
			break;

		case BAT_COUNT:
			bat_count = dpci_bat_num_batteries();
			if (bat_count < 0)
			{
				fprintf(stderr,
		       			"Couldn't obtain number of batteries: %s\n",
					dpci_last_os_error_string());
				break;
			}
			printf("Number of batteries on the board = %d\n",
				bat_count);
			break;

		case BAT_NAME:
			printf("Enter the Battery Number: ");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d", &bat_no);

			ret = dpci_bat_name(bat_no,bat_name,sizeof(bat_name));
			if(ret < 0)
			{
				fprintf(stderr,
					"Couldn't obtain battery name for battery %d: %s\n",
					bat_no,
					dpci_last_os_error_string());
				break;
			}
			printf("Name of battery number %d is %s\n",
				bat_no,
				bat_name);
			break;

		case BAT_NUMBER:
			printf("Enter the Battery Name: ");
			fgets(bat_name,sizeof(bat_name),stdin);
			bat_name[strlen(bat_name) - 1] = 0; // remove CR/LF
			bat_no = dpci_bat_number(bat_name);
			if(bat_no < 0)
			{
				fprintf(stderr,
					"Couldn't obtain battery number for battery %s\n",
					bat_name);
				break;
			}
			printf("Battery number of Battery %s is %d\n",
				bat_name,bat_no);
			break;

		case BAT_SHOW_ALL:
			bat_count = dpci_bat_num_batteries();
			if(bat_count < 0)
			{
				fprintf(stderr,
				       "Couldn't obtain number of batteries: %s\n",
					dpci_last_os_error_string());
				break;
			}

			for(bat_no = 0; bat_no < bat_count; bat_no++)
			{
				ret = dpci_bat_name(bat_no,bat_name,sizeof(bat_name));
				if(ret < 0)
				{
					fprintf(stderr,
						"Couldn't obtain battery name for battery %d: %s",
						bat_no,
						dpci_last_os_error_string());
					continue;
				}
				bat_level = dpci_bat_get_level(bat_no);
				if(bat_level < 0)
				{
					fprintf(stderr,
						"Couldn't obtain voltage level of battery %d: %s",
						bat_no,
						dpci_last_os_error_string());
					continue;
				}
				bat_status = dpci_bat_get_status_mask();
				if(bat_status < 0)
				{
					fprintf(stderr,
						"Couldn't obtain battery status for battery %d: %s\n",
						bat_no,
						dpci_last_os_error_string());
						break;
				}
				printf("%s\t%s\t%4.3fV\n",
						bat_name,
						((bat_status & (1 << bat_no)) ? "OK" : "FAULT"),
						((float)bat_level) / 1000);
			}
			break;

		case BAT_LEVEL:
			printf("Enter the Battery Number: ");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d", &bat_no);

			bat_level = dpci_bat_get_level(bat_no);
			if (bat_level < 0)
			{
				fprintf(stderr,
					"Couldn't get voltate of battery %d: error %s\n",
					bat_no,
					dpci_last_os_error_string());
				break;
			}
			printf("Voltage Level for Battery %d = %4.3fV\n",
				bat_no,
				((float)bat_level) / 1000);
			break;

		case BAT_STATUS:
			{
				int bat_no;

				printf("Enter the Battery Number: ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%d", &bat_no);

				bat_status = dpci_bat_get_status(bat_no);
				if (bat_status < 0)
				{
					fprintf(stderr,
						"Couldn't get battery status: error %s\n",
						dpci_last_os_error_string());
					break;
				}
				bat_status = bat_status & (1 << bat_no);
				if(bat_status == 0)
					printf("Battery %d is FAULTY\n", bat_no);
				else
					printf("Battery %d is OK\n", bat_no);
			}

			break;
		case BAT_SET_CHECKPERIOD:
			{
				int code;
				int ret;
				char bat_code;

				printf("Enter the Check Period Code:\n"
						"[Choices are: \n n - none\n "
						"m - every minute\n h - every hour\n "
						"d - every day\n o - every month]: ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%c", &bat_code);

				switch (tolower(bat_code))
				{
				case 'n':
					code = BAT_CHECKPERIOD_NONE;
					break;
				case 'm':
					code = BAT_CHECKPERIOD_1MIN;
					break;
				case 'h':
					code = BAT_CHECKPERIOD_1HOUR;
					break;
				case 'd':
					code = BAT_CHECKPERIOD_1DAY;
					break;
				case 'o':
					code = BAT_CHECKPERIOD_1MONTH;
					break;
				default:
					fprintf(stderr,
						"Period %c not supported.\n",
						bat_code);
					fprintf(stderr,
						"Valid Choices are: n - none, "
						"m - every minute, h - every hour, "
						"d - every day, o - every month\n");
					continue;
				}

				ret = dpci_bat_set_check_period(code);
				if(ret < 0)
					fprintf(stderr,
						"Couldn't set check period: %s\n",
						dpci_last_os_error_string());
				else
					printf("Battery Check Period set to %c\n", bat_code);
			}
			break;
		case BAT_SET_ERRORLEVEL:
			{
				int err_level;
				int bat_no;
				int ret;

				printf("Enter Battery Number: ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%d", &bat_no);

				printf("Enter Error Level Voltage (mV): ");
				fgets(line,sizeof(line),stdin);
				sscanf(line,"%d", &err_level);

				ret = dpci_bat_set_error_level(bat_no,err_level);
				if (ret < 0)
					fprintf(stderr,
						"Couldn't set Error Level For Battery %d: %s\n",
						bat_no,
						dpci_last_os_error_string());
				else
					printf("Error Level for Battery %d is set to %dmV\n",
							bat_no, err_level);

			}
			break;
		case BAT_GET_ERRORLEVEL:
			{
			int bat_no;
			int bat_errorlevel;

			printf("Enter the Battery Number: ");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d", &bat_no);

			bat_errorlevel = dpci_bat_get_errorlevel(bat_no);
			if(bat_errorlevel < 0)
			{
				fprintf(stderr,
					"Couldn't obtain error voltage level of battery %d: %s\n",
					bat_no,
					dpci_last_os_error_string());
				break;
			}
			printf("Error Voltage Level for Battery %d = %4.3fV\n",
				bat_no, ((float)bat_errorlevel) / 1000);
			}
			break;
		default:
			printf("Not Implemented");
		}
	}
	return 0;
}

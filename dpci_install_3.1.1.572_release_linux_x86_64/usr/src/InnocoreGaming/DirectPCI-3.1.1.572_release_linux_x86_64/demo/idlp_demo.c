/******************************************************************************
 *
 * $Id: idlp_demo.c 12370 2016-03-09 12:06:12Z james $
 *
 * Copyright 2003-2015 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * Intrusion-detection diagnostic demo.
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
#include <conio.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "dpci_core_api.h"
#include "dpci_core_hw.h"
#include "dpci_version.h"
#include "dpci_boards.h"

unsigned int fw_version = 0;

static void display_fw_version(unsigned int version)
{
	printf("IDLP f/w version: ");
	if (fw_version == (unsigned int)-1)
	{
		printf("Couldn't be determined.\n");
	}
	else if (fw_version > 256)
	{
		printf("%02d.%d (%d.%d=",
			IDLP_VERSION_MAJOR(fw_version),
			IDLP_VERSION_BUILD(fw_version),
			IDLP_VERSION_CONF1(fw_version),
			IDLP_VERSION_CONF2(fw_version));
			printf("Build flags: ");
		if (IDLP_VERSION_CONF2(fw_version) & IDLP_VERSION_CONF2_ISDEBUG)
			printf("DEBUG ");
		else
			printf("RELEASE ");
		if (IDLP_VERSION_CONF2(fw_version) & IDLP_VERSION_CONF2_NOTAUTOBUILD)
			printf("DEVELOPER-BUILD ");
		else
			printf("AUTOBUILD ");
		if (IDLP_VERSION_CONF2(fw_version) & IDLP_VERSION_CONF2_INTR67)
			printf("INTRUS67 ");
		printf(")\n");
	}
	else
	{
		printf("%d\n", fw_version);
	}
}


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
	printf("\n----------------------------------------"
		  "---------------------------------------\n");
	printf("DirectPCI IDLP Demo v" DPCI_VERSION ", $Revision: 12370 $\n");
	printf("(c) 2003-2015 Advantech Co Ltd. All Rights Reserved.\n");
   if (api_version_string)
	   printf("DLL v%s  ", api_version_string);
   else
	   printf("DLL version unknown  ");
   if (drv_version_string)
	   printf("Driver v%s  \n", drv_version_string);
   else
	   printf("Driver version unknown.  \n");
	printf("----------------------------------------"
		  "---------------------------------------\n");
	printf(" 1. Read logging processor firmware version\n");
	printf(" 2. Read number of events  3. Read last event\n");
	printf(" 4. Read unread events     5. Read old events\n"); 
	printf(" 6. Get time and date      7. Set time and date\n");
	printf(" 8. Enable watch-dog       9. Reset watch-dog\n");
	printf("10. Get intrusion status  11. Set intrusion sleep time\n");
	printf("12. Listen for logging processor interrupts.\n");
	printf("13. Show date/time of last event occurrence.\n");
	printf(" 0. Menu                   Q. Quit\n");
	printf("----------------------------------------"

		  "---------------------------------------\n");
	printf("%s: ", board_name);
	display_fw_version(fw_version);
	printf("----------------------------------------"
		  "---------------------------------------\n");
	printf("\n");
}

int main(void)
{
	char line[256] = "";
	int cmd = 0;
	struct idevent myres;
	int count;
	int sleepcode;
	int year, month, day, hour, min, sec;
	struct idevent inevent;
	char idlp_psw[256] = "";
	int ret;

	if( dpci_core_open() < 0)
	{
		fprintf(stderr,
			"Error: Ensure the DirectPCI core driver is correctly installed, enabled and that you have the required access privileges.\n");
		return 1;
	}

	fw_version = dpci_id_fwversion_full();

	menu();

	while (tolower(line[0]) != 'q') 
	{
		printf("Enter option: ");
		cmd = -1;
		fgets(line,sizeof(line),stdin);
		sscanf(line,"%d", &cmd);

		switch(cmd) 
		{
		case 0:
			menu();
			break;

		case 1:
			fw_version = dpci_id_fwversion_full();
			if (fw_version == -1)
			{
				printf("Couldn't get firmware version: %s\n",
					dpci_last_os_error_string());
			}
			display_fw_version(fw_version);
			break;

		case 2:
			 if ((count = dpci_id_numevents()) == -1)
			 {
				printf("Couldn't get event count from PIC: %s\n",
					   dpci_last_os_error_string());
				break;
			 }
			 printf("There are %d events waiting in the FIFO\n", count);
			 break;

		case 3:
			ret = dpci_id_readevent(&myres);
			if (ret == -1)
			{
				printf("Couldn't get event data from PIC: %s\n",
					   dpci_last_os_error_string());
				break;
			}
#ifdef __linux
			else if (ret == ERROR_PSW_REQ)
			{
				printf("Enter Password:");
				fgets(idlp_psw, sizeof(idlp_psw), stdin);
				idlp_psw[strlen(idlp_psw) - 1] = 0;
				ret = dpci_id_readevent_psw(&myres, idlp_psw);
				if (ret == ERROR_PSW_INVALID)
				{
					printf("Incorrect Password!\n");
					break;
				}
				else if (ret == -1)
				{
					printf("Couldn't get event data from PIC: %s\n",
					   dpci_last_os_error_string());
					break;
				}			
			}
#endif			
			printf("\nEvent Read\n==================\n");
			printf("YYYY/MM/DD HH:MM:SS  Code  Type\n");
			printf("%04d/%02d/%02d %02d:%02d:%02d  %4d  %s\n",
				2000 + myres.year,
				myres.month,
				myres.day,
				myres.hour,
				myres.min,
				myres.sec,
				myres.eventcode,
			dpci_id_event_name(myres.eventcode));
			break;

		case 4:
			printf("\nEvent Read\n==================\n");
			printf("YYYY/MM/DD HH:MM:SS  Code  Type\n");
			while (1)
			{
				ret = dpci_id_readevent(&myres);
				if (ret == -1)
				{
				   printf("Couldn't get event data from PIC: %s\n",
						  dpci_last_os_error_string());
				   break;
				}	
#ifdef __linux
				else if (ret == ERROR_PSW_REQ)
				{
					printf("Enter Password:");
					fgets(idlp_psw, sizeof(idlp_psw), stdin);
					idlp_psw[strlen(idlp_psw) - 1] = 0;
					ret = dpci_id_readevent_psw(&myres, idlp_psw);
					if (ret == ERROR_PSW_INVALID)
					{
						printf("Incorrect Password!\n");
						break;
					}
					else if (ret == -1)
					{
						printf("Couldn't get event data from PIC: %s\n",
						   dpci_last_os_error_string());
						break;
					}				
				}
#endif				
				if (myres.eventcode == ID_EVENT_NONE)
				{
				   break;
				}
				printf("%04d/%02d/%02d %02d:%02d:%02d  %4d  %s\n",
					   2000 + myres.year,
					   myres.month,
					   myres.day,
					   myres.hour,
					   myres.min,
					   myres.sec,
					   myres.eventcode,
				dpci_id_event_name(myres.eventcode));
			} /*end while (1)*/
			break;

		case 5:
			printf("\nEvent Read\n==================\n");
			printf("YYYY/MM/DD HH:MM:SS  Code  Type\n");
			for (count = 0; count < 255; count++)
			{
				ret = dpci_id_select_event(count, &myres);
				if (ret == -1)
				{
					/*
					 * If we get EINVAL (linux) or ERROR_INVALID_PARAMETER it means
					 * this is a newer driver and firmware v60 or newer and that the
					 * event log has events that have not been read yet.
					 *
					 * If we get EIO (linux) or ERROR_IO_DEVICE then this is either
					 * an older driver or f/w < v60.30 and we have unread events.
					 */
#ifdef linux
					if (errno == EINVAL || errno == EIO)
#else
					if (GetLastError() == ERROR_INVALID_PARAMETER ||
						GetLastError() == ERROR_IO_DEVICE)
#endif
					{
						ret = dpci_id_numevents();
						if (ret + count >= 255)
						{
							printf("Cannot read the whole old event log: at least %d event(s) still unread.\n", ret);
							break;
						}
					}
				   printf("Couldn't get old event #%d from PIC: %s\n",
						  count,
						  dpci_last_os_error_string());
				   break;
				}	
				if (myres.eventcode == ID_EVENT_NONE)
				{
				   break;
				}
				printf("%3d: %04d/%02d/%02d %02d:%02d:%02d  %4d  %s\n",
					   count,
					   2000 + myres.year,
					   myres.month,
					   myres.day,
					   myres.hour,
					   myres.min,
					   myres.sec,
					   myres.eventcode,
				dpci_id_event_name(myres.eventcode));
			} /*end while (1)*/
			break;

		case 6:
			if (dpci_id_getdate(&myres) == -1)
			{
				printf("Couldn't get time and date from PIC: %s\n",
					   dpci_last_os_error_string());
				break;
			}	
			printf("\nDate Read\n==================\n");
			printf("YYYY/MM/DD HH:MM:SS\n");
			printf("%04d/%02d/%02d %02d:%02d:%02d\n",
					2000 + myres.year,
					myres.month,
					myres.day,
					myres.hour,
					myres.min,
					myres.sec);
			break;

		case 7:
			printf("Enter Year(YY):");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d",&year);
			printf("Enter Month:");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d",&month);
			printf("Enter Day:");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d",&day);
			printf("Enter Hour:");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d",&hour);
			printf("Enter Min:");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d",&min);
			printf("Enter Sec:");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d",&sec);

			myres.sec = sec;
			myres.min = min;
			myres.hour = hour;
			myres.day = day;
			myres.month = month;
			myres.year = year;
			ret = dpci_id_setdate(&myres);
			if (ret == -1)
			{
				printf("Couldn't set time and date in PIC: %s\n",
					   dpci_last_os_error_string());
				break;
			}
#ifdef __linux
			else if (ret == ERROR_PSW_REQ)
			{
				printf("Enter Password:");
				fgets(idlp_psw, sizeof(idlp_psw), stdin);
				idlp_psw[strlen(idlp_psw) - 1] = 0;
				ret = dpci_id_setdate_psw(&myres, idlp_psw);
				if (ret == ERROR_PSW_INVALID)
				{
					printf("Incorrect Password!\n");
					break;
				}
				else if (ret == -1)
				{
					printf("Couldn't set time and date in PIC: %s\n",
					   dpci_last_os_error_string());
					break;
				}			
			}	
#endif
			break;

		case 8:
			printf("Enter time in seconds (0=disable): ");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d", &cmd);
			if (dpci_wd_enable(cmd) == -1)
			{
				printf("Couldn't set watchdog time-out: %s\n", dpci_last_os_error_string());
				break;
			}	
			printf("WatchDog enabled triggers every %d seconds\n\n",cmd);
			break;

		case 9:
			if (dpci_wd_reset() == -1)
			{
				printf("Couldn't reset watchdog: %s\n", dpci_last_os_error_string());
				break;
			}	
			printf("Reset Direct PCI Watchdog\n");
			break;

		case 10:
		{
			int status;

			status = dpci_id_intrusionstatus();
			if (status == -1)
			{
			   fprintf(stderr,
					   "Couldn't get intrusion status: %s\n",
					   dpci_last_os_error_string());
			   break;
			}
			printf("Intrusion detection circuit#0 is %s\n",
				   (status & 1) ? "closed" : "open");
			printf("Intrusion detection circuit#1 is %s\n",
				   (status & 2) ? "closed" : "open");
			printf("Intrusion detection circuit#2 is %s\n",
				   (status & 4) ? "closed" : "open");
			printf("Intrusion detection circuit#3 is %s\n",
				   (status & 8) ? "closed" : "open");
			printf("Intrusion detection circuit#4 is %s\n",
				   (status & 16) ? "closed" : "open");
			printf("Intrusion detection circuit#5 is %s\n",
				   (status & 32) ? "closed" : "open");
			if ((fw_version > 256) &&
				(IDLP_VERSION_CONF2(fw_version) &
					IDLP_VERSION_CONF2_INTR67))
			{
				printf("Intrusion detection circuit#6 is %s\n",
					(status & 64) ? "closed" : "open");
				printf("Intrusion detection circuit#7 is %s\n",
					(status & 128) ? "closed" : "open");
			}
		}
		break;

		case 11:
			printf("Enter sleep time in milliseconds (1000, 500, 250 or 125): ");
			fgets(line,sizeof(line),stdin);
			sscanf(line,"%d", &cmd);
			sleepcode = 0xff;
			switch (cmd)
			{
			case 1000:
				sleepcode = INTRUS_SLEEP_1SEC;
				break;
			case 500:
				sleepcode = INTRUS_SLEEP_500MSEC;
				break;
			case 250:
				sleepcode = INTRUS_SLEEP_250MSEC;
				break;
			case 125:
				sleepcode = INTRUS_SLEEP_125MSEC;
				break;
			default:
				printf("Sleep time of %dms is not supported.\n", cmd);
				break;
			}
			if (dpci_id_setsleeptime(sleepcode) == -1)
			{
				printf("Couldn't set sleep period to %dms: %s\n",
					   cmd,
					   dpci_last_os_error_string());
				break;
			}
			printf("Intrusion detection sleep period set to %dms.\n", cmd);
			break;

		case 12:
		{
			int iResult;
			int timeout_ms;

			printf("Enter time-out in seconds (default 10000): ");
			fgets(line,sizeof(line),stdin);
			if (sscanf(line,"%d", &timeout_ms) <= 0)
			{
				timeout_ms = 10000;
			}

			printf("\nListening for interrupts from the logging processor. (Ctrl+C to stop)\n");
			while (1)
			{
				/*
				 * Wait for the event.
				 */
				iResult = dpci_id_wait_event(&inevent, timeout_ms);
				if (iResult < 0)
				{
					perror("dpci_id_wait_event()");
					break;
				}
#ifdef __linux
				else if (iResult == ERROR_PSW_REQ)
				{
					printf("Enter Password:");
					fgets(idlp_psw, sizeof(idlp_psw), stdin);
					idlp_psw[strlen(idlp_psw) - 1] = 0;
					iResult = dpci_id_wait_event_psw(&inevent, 
						timeout_ms, idlp_psw);
					if (iResult == ERROR_PSW_INVALID)
					{
						printf("Incorrect Password!\n");
						break;
					}
					else if (iResult < 0)
					{
						perror("dpci_id_wait_event()");
						break;
					}			
				}	
#endif
				/*
				 * A return code of zero means no event 
				 * occurred during the specified period.
				 */
				if (iResult == 0)
				{
					printf("dpci_id_wait_event() timed-out.\n");
					continue;
				}

				/* Print the event */
				printf("Event code: %d - %s\n",
						inevent.eventcode,
						dpci_id_event_name(inevent.eventcode));
				printf("Event time: %04d/%02d/%02d %02d:%02d:%02d\n",
					inevent.year + 2000,
					inevent.month,
					inevent.day,
					inevent.hour,
					inevent.min,
					inevent.sec);
			}
			break;

		}/*end case 11:*/

		case 13:
			printf("Code  Date & time          Type\n");
			printf("---------------------------------------------------\n");
			for (count = 1; count <= MAX_ID_EVENT; count++)
			{
				if (dpci_id_readevent_instance(count, &myres) == -1)
				{
					/*
					 * If the IDLP does not retain this
					 * event then it returns "invalid
					 * parameter" error, whereupon we move
					 * on to the next type.
					 */
#ifdef linux
					if (dpci_last_os_error_code() == EINVAL)
#else
					if (dpci_last_os_error_code() == ERROR_INVALID_PARAMETER)
#endif
					{
						continue;
					}
					printf("Couldn't get event #%d data: %s\n",
						count,
						dpci_last_os_error_string());
					break;
				}
				/*
				 * If we get an event back but the eventcode is
				 * different from what we expect then it means
				 * that IDLP can store this event but the space
				 * for it is shared with another event code.
				 */
				if (myres.eventcode != count)
				{
					printf("%4d  <not recorded>       %s\n",
						count,
						dpci_id_event_name(count));
				}
				else
				{
					printf("%4d  %04d/%02d/%02d %02d:%02d:%02d  %s\n",
						myres.eventcode,
						2000 + myres.year,
						myres.month,
						myres.day,
						myres.hour,
						myres.min,
						myres.sec,
						dpci_id_event_name(myres.eventcode));
				}
			}
			break;
	  }

	}/*end while (tolower(line[0]) != 'q') */

	return 0;
}

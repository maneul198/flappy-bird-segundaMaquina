/******************************************************************************
 *
 * $Id: callback_demo.c 12370 2016-03-09 12:06:12Z james $
 *
 * Copyright 2010-2013 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * DirectPCI core callback demo.
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
 *		support@advantech-innocore.com
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
#include <stdarg.h>
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

#ifdef __linux
#include <linux/input.h>
#define	INPUT_INTERRUPT_DEVICE	"/dev/input/dpci"
#endif

#ifdef WIN32
# define	usleep(us)	Sleep((us) / 1000)
#endif

unsigned int debounce_mask;
unsigned int debounce_timeout_ms;

typedef struct _callback_data callback_data;
struct _callback_data
{
	unsigned int callback_number;
	unsigned int callback_type;
	unsigned int change_mask;
	unsigned int dio_value;
	unsigned int eventcode;
};

typedef struct _callback_data_list callback_data_list;
struct _callback_data_list
{
	callback_data *cbd;
	callback_data_list *next;
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
	printf("DirectPCI Callback Demo v" DPCI_VERSION ", $Revision: 12370 $\n");
	printf("(C) 2010-2015 Advantech Co Ltd.  All rights reserved.\n");
	if (api_version_string)
		printf("DLL v%s  ", api_version_string);
	else
		printf("DLL version unknown  ");
	if (drv_version_string)
		printf("Driver v%s  \n", drv_version_string);
	else
		printf("Driver version unknown.  \n");
	printf("-------------------------------------------------------------------------------\n");
	printf("1. Register callback\n");
	printf("2. Unregister callback\n");
	printf("3. Set debounce parameters\n");
	printf("4. Send IO pulse\n");
	printf("5. Multiple Callbacks\n");
	printf("6. Print Callbacks\n");
	printf("Q. Quit (or use Ctrl+C)\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("%s: Debounce mask = 0x%08x, timeout = %d ms\n",
		board_name,
		debounce_mask,
		debounce_timeout_ms);
	printf("-------------------------------------------------------------------------------\n");
	printf("\n");
}

static void std_callback(struct dpci_event *evp, void *arg)
{
#ifdef __linux	
	printf("Type=%d ts=%llu/%llu arg=%u: ",
#else
	printf("Type=%d ts=%I64d/%I64d arg=%u: ",
#endif
		evp->de_type,
		evp->de_ts,
		evp->de_ts_delta,
		((callback_data*)arg)->callback_number);
	switch (evp->de_type)
	{
	case EVENT_IDLP:
		printf("%04d/%02d/%02d %02d:%02d:%02d %2d %s",
			2000 + idlp_event(evp).year,
			idlp_event(evp).month,
			idlp_event(evp).day,
			idlp_event(evp).hour,
			idlp_event(evp).min,
			idlp_event(evp).sec,
			idlp_event(evp).eventcode,
			dpci_id_event_name(idlp_event(evp).eventcode));
		break;

	case EVENT_DIG_IP:
		printf("changed=%08lx state=%08lx",
			dio_event(evp).change_mask,
			dio_event(evp).dio_value);
		break;
	}
	printf("\n");
}

static int getval(unsigned int *p, const char *fmt, ...)
{
	va_list ap;
	char line[256] = "";

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf(": ");
	fflush(stdout);
	if (!fgets(line, sizeof(line), stdin))
		return 0;
	*p = strtoul(line, NULL, 0);
	return 1;
}

static int get_event_details(struct dpci_event *evp)
{
	memset(evp, 0, sizeof(*evp));
	if (!getval((unsigned int *)&evp->de_type,
		"Enter type of event (0 for all)"))
		return 0;
	switch (evp->de_type)
	{
	case 0:
		break;
	case EVENT_DIG_IP:
		if (!getval((unsigned int *)&dio_event(evp).change_mask, "Change mask"))
			break;
		if (!getval((unsigned int *)&dio_event(evp).dio_value, "Value"))
			break;
		break;
	case EVENT_IDLP:
		if (!getval((unsigned int *)&idlp_event(evp).eventcode, "Event code"))
			break;
		break;
	case EVENT_PFD:
		break;
	case EVENT_TS:
		break;
	default:
		printf("Event type %d is not allowed.\n", evp->de_type);
		return 0;
	}
	return 1;
}

static void free_list(callback_data_list* cbdl)
{
	callback_data_list* tmp;

	while (cbdl != NULL)
	{
		tmp = cbdl;
		cbdl = cbdl->next;
		free(tmp->cbd);
		free(tmp);
	}
}

static void print_list(callback_data_list* cbdl)
{
	while (cbdl->next != NULL)
	{
		printf("Callback Data: %u\n", cbdl->cbd->callback_number);
		printf("Callback Type: %u\n", cbdl->cbd->callback_type);
		printf("Callback Change Mask: 0x%08x\n", cbdl->cbd->change_mask);
		printf("Callback Value: 0x%08x\n", cbdl->cbd->dio_value);
		printf("Callback Event Code: %u\n\n", cbdl->cbd->eventcode);

		cbdl = cbdl->next;
	}
}

static callback_data_list* search_list(callback_data_list* cbdl, unsigned int value)
{
	while (cbdl != NULL)
	{
		if (cbdl->cbd->callback_number == value)
		{
			return cbdl;
		}
		cbdl = cbdl->next;
	}

	return NULL;
}

static callback_data_list* add_node(callback_data *cbd, callback_data_list *head, callback_data_list *tail)
{
	tail->cbd = cbd;
	tail->next = head;
	return tail;
}

static void delete_node(callback_data_list* cbdl)
{
	callback_data_list *tmp = cbdl->next;

	if (tmp != NULL)
	{
		cbdl->cbd = tmp->cbd;
		cbdl->next = tmp->next;
		free(tmp);
	}
}

static void set_node_values(callback_data* cbd, struct dpci_event event)
{
	cbd->callback_type = event.de_type;
	cbd->change_mask = dio_event(&event).change_mask;
	cbd->dio_value = dio_event(&event).dio_value;
	cbd->eventcode = 0;
	if (event.de_type == EVENT_IDLP)
	{
		cbd->eventcode = idlp_event(&event).eventcode;
	}
}

int main(void)
{
	char line[256] = "";
	int cmd = 0;
	struct dpci_event event;

	unsigned int port = 0;
	unsigned int value = 0;
	unsigned int change_mask = 0;
	unsigned int count = 0;
	unsigned int delay = 0;
	unsigned int i = 0;
	unsigned int arg = 0;

	callback_data *cbd = NULL;
	callback_data_list *head = NULL, *tail = NULL, *remove = NULL;

	if( dpci_core_open() < 0)
	{
		fprintf(stderr,
			"Error: Ensure the DirectPCI core driver is correctly installed, enabled and that you have the required access privileges.\n");
		return 1;
	}

	cbd = (callback_data*)malloc(sizeof(callback_data));
	tail = (callback_data_list*)malloc(sizeof(callback_data_list));
	head = add_node(cbd, head, tail);

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
		if (sscanf(line, "%d", &cmd) < 1)
			continue;

		switch (cmd)
		{
		case 0:
			menu();
			break;
		case 1:
			cbd = (callback_data*)malloc(sizeof(callback_data));

			if (!get_event_details(&event))
				break;

			if (!getval(&cbd->callback_number, "Callback argument"))
				break;

			if (dpci_ev_register_callback(std_callback, (void *)cbd, &event) != 0)
			{
				fprintf(stderr, "failed to register callback\n");
				break;
			}

			set_node_values(cbd, event);

			tail = (callback_data_list*)malloc(sizeof(callback_data_list));
			head = add_node(cbd, head, tail);

			break;
		case 2:
			if (!get_event_details(&event))
				break;
			if (!getval(&arg, "Callback argument"))
				break;

			remove = search_list(tail, arg);

			if (remove != NULL)
			{
				if (dpci_ev_unregister_callback(std_callback, (void *)remove->cbd, &event) != 0)
				{
					fprintf(stderr, "failed to unregister callback\n");
					break;
				}
				delete_node(remove);
			}
			else
			{
				fprintf(stderr, "failed to find argument in list\n");
			}

			break;
		case 3:
			if (!getval(&debounce_mask, "Debounce mask"))
				break;
			if (!getval(&debounce_timeout_ms, "Debounce timeout (ms)"))
				break;
			if (dpci_ev_set_debounce(debounce_mask, debounce_timeout_ms) == -1)
			{
				fprintf(stderr, "failed to set debounce parameters\n");
				break;
			}
			break;
		case 4:
			if (!getval(&port, "Enter port to write to (Hex)"))
				break;

			if (!getval(&change_mask, "Enter change mask"))
				break;

			if (!getval(&value, "Enter value"))
				break;

			if (!getval(&delay, "Enter delay in ms"))
				break;

			if (!getval(&count, "Enter edge count"))
				break;

			value &= change_mask;

			if (dpci_io_change_port(port, (unsigned char)value, (unsigned char)~value & change_mask, (unsigned char)change_mask) == -1)
			{
				perror("Can't write byte to port");
				break;
			}

			for (i = 0; i < count; i++)
			{

				if (dpci_io_change_port(port, 0, 0, (unsigned char)change_mask) == -1)
				{
					perror("Can't write byte to port");
					break;
				}

				usleep(delay * 1000);
			}
			break;
		case 5:
			count = 0;

			if (!getval(&count, "Enter number of callbacks"))
				break;

			if (!get_event_details(&event))
				break;

			for (i = 0; i < count; i++)
			{
				cbd = (callback_data*)malloc(sizeof(callback_data));

				if (!getval(&cbd->callback_number, "Callback argument for callback: %u", i))
					break;

				if (dpci_ev_register_callback(std_callback, (void *)cbd, &event) != 0)
				{
					fprintf(stderr, "failed to register callback\n");
					break;
				}

				set_node_values(cbd, event);
				tail = (callback_data_list*)malloc(sizeof(callback_data_list));
				head = add_node(cbd, head, tail);
			}

			break;
		case 6:
			print_list(tail);
			break;
		default:
			fprintf(stderr, "Not implemented.\n");
		}
	}

	free_list(tail);

	return 0;
}

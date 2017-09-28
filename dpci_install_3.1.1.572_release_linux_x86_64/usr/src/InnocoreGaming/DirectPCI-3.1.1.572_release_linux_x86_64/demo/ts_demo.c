/******************************************************************************
 *
 * $Id: ts_demo.c 12370 2016-03-09 12:06:12Z james $
 *
 * Copyright 2006-2013 Advantech Co Limited.
 * All rights reserved.
 *
 * Description:
 * DirectPCI demo.
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
#include <signal.h>
#else
#include <windows.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>

#include <dpci_core_api.h>
#include <dpci_core_hw.h>

static struct lm89_reg
{
	int r_offs;
	int w_offs;
	char *name;
	char *desc;
} lm89_regs[] =
{
	{LM89_REG_LT, -1, "LT", "Local temperature"},
	{LM89_REG_RTHB, -1, "RTHB", "Remote temperature high byte"},
	{LM89_REG_RTLB, -1, "RTLB", "Remote temperature low byte"},
	{LM89_REG_SR, -1, "SR", "Status register"},
	{LM89_REG_C_R, LM89_REG_C_W, "C", "Configuration register"},
	{LM89_REG_CR_R, LM89_REG_CR_W, "CR", "Conversion Rate"},
	{LM89_REG_LLS_R, LM89_REG_LLS_W, "LLS", "Local low-alert point"},
	{LM89_REG_LHS_R, LM89_REG_LHS_W, "LHS", "Local high-alert point"},
	{LM89_REG_LCS, LM89_REG_LCS, "LCS", "Local critical set-point"},
	{LM89_REG_RLSHB_R, LM89_REG_RLSHB_W, "RLSHB", "Remote low-alert point hi-byte"},
	{LM89_REG_RLSLB, LM89_REG_RLSLB, "RLSLB", "Remote low-alert point low-byte"},
	{LM89_REG_RHSHB_R, LM89_REG_RHSHB_W, "RHSHB", "Remote high-alert point hi-byte"},
	{LM89_REG_RHSLB, LM89_REG_RHSLB, "RHSLB", "Remote high-alert point low-byte"},
	{LM89_REG_RCS, LM89_REG_RCS, "RCS", "Remote critical set-point"},
	{LM89_REG_RTOHB, LM89_REG_RTOHB, "RTOHB", "Remote offset hi-byte"},
	{LM89_REG_RTOLB, LM89_REG_RTOLB, "RTOLB", "Remote offset low-byte"},
	{LM89_REG_RDTF, LM89_REG_RDTF, "RDTF", "Remote diode temp. filter"},
	{LM89_REG_TH, LM89_REG_TH, "TH", "Critical alert hysteresis"},
	{-1, LM89_REG_ONESHOT, "ONESHOT", "One-shot conversion"},
	{LM89_REG_RMID, -1, "RMID", "Manufacturer's ID"},
	{LM89_REG_RDR, -1, "RDR", "Die/Steppting revision code"},
	{0, 0, NULL, NULL}
};

static const char *alarm_names[] =
{
	"none",
	"critical",
	"high/low",
	"<bogus>"
};

static void menu(void)
{
	printf("\n--------------------------------------------------------------------------------\n");
	printf("DirectPCI Temperature Sensor Demo, $Revision: 12370 $\n");
	printf("(C) 2007-2013 Advantech Co Ltd.  All rights reserved.\n");
	printf("--------------------------------------------------------------------------------\n");
	printf(" 1. Initialise LM89\n");
	printf(" 2. Read temperature sensors\n");
	printf(" 3. Configure a temperature sensor.\n");
	printf(" 4. Get temperature sensor configuration.\n");
	printf(" 5. Check alarm status.\n");
	printf(" 6. Read LM89 register.\n");
	printf(" 7. Write LM89 register.\n");
	printf(" 8. Wait for a sensor alarm interrupt\n");
#ifdef __linux
	printf(" 9. Wait for a sensor alarm signal.\n");
#endif
	printf("10. Dump registers.\n");
	printf(" Q. Quit (or use Ctrl+C)\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("\n");
}

char *get_string(char *fmt, ...)
{
	va_list ap;
	static char buf[256];
	char *ret;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	ret = fgets(buf,sizeof(buf),stdin);
	if (ret)
	{
		ret[strlen(ret) - 1] = 0;
	}
	return ret;
}

int get_int(int base, char *fmt, ...)
{
	va_list ap;
	static char buf[256];

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	fgets(buf,sizeof(buf),stdin);
	return strtol(buf, NULL, base);
}

static int find_reg(char *text, int write)
{
	int i;

	if (text[strlen(text) - 1] == '\n')
		text[strlen(text) - 1] = 0;
	for (i = 0; lm89_regs[i].name; i++)
	{
		if (strcmp(text, lm89_regs[i].name) == 0)
		{
			if (write)
				return lm89_regs[i].w_offs;
			return lm89_regs[i].r_offs;
		}
	}
	return strtol(text, NULL, 16);
}

int main(void)
{
	char *line = NULL;
	int cmd = 0;
	int val = 0;
	int addr, ret;
	int i;
	unsigned char byte;
	int high, low, crit, alarm, offs;
	int local_state, remote_state;
	int sensor, max_sensors;

	if( dpci_core_open() < 0)
	{
		fprintf(stderr,
			"Error: Ensure the DirectPCI core driver is correctly installed, enabled and that you have the required access privileges.\n");
		return 1;
	}

	/*
	 * Check it's an 80-0062 I/O Board.
	 */
	if (dpci_io_getboard_id() != DPCI_IOBOARD_ID_80_0062A)
	{
		fprintf(stderr, "Temperature Sensor feature is available only on IO card 80-0062 on a ConnectBus-II Board.\n");
		exit(1);
	}

	max_sensors = dpci_ts_numsensors();
	menu();
	while (1)
	{
		line = get_string("Enter option (0 for menu): ");
		cmd = 0;
		if (tolower(line[0]) == 'q')
		{
			break;
		}
		cmd = atoi(line);

		switch (cmd)
		{
		case 0:
			menu();
			break;

		case 1:
			if (dpci_ts_init() != 0)
			{
				fprintf(stderr,
					"Couldn't initialise the LM89: "
					"system error %d (%s), "
					"i2c error %d (%s)\n",
					errno,
					dpci_last_os_error_string(),
					dpci_i2c_last_error(),
					dpci_i2c_error_string(dpci_i2c_last_error()));
				break;
			}
			printf("Initialised the temperature sensors.\n");
			break;

		case 2:
			for (sensor = 0; sensor < max_sensors; sensor++)
			{
				int temp, connected;
				char sensor_name[64];

				if (dpci_ts_sensor_name(sensor, sensor_name, sizeof(sensor_name)) == -1)
				{
					fprintf(stderr,
							"Couldn't get sensor name or sensor #%d: %d/%s\n",
							sensor,
							errno,
							dpci_last_os_error_string());
					break;
				}
				if (dpci_ts_getsensor(sensor, &temp, &connected) != 0)
				{
					fprintf(stderr,
							"Couldn't read temperature %d (%s): %d/%s\n",
							sensor,
							sensor_name,
							errno,
							dpci_last_os_error_string());
					break;
				}
				printf("Sensor %d \"%-8s\": ", sensor, sensor_name);
				if (connected)
				{
					printf(" %3d", temp);
				}
				else
				{
					printf(" N/C");
				}
				printf("\n");
			}
			break;

		case 3:
			sensor = get_int(0, "Enter sensor number: ");
			if (sensor < 0 || sensor >= max_sensors)
			{
				fprintf(stderr,
					"Invalid sensor number %d\n", 
					sensor);
				break;
			}
			low = get_int(0, "Enter low temperature: ");
			high = get_int(0, "Enter high temperature: ");
			crit = get_int(0, "Enter critical temperature: ");
			alarm = get_int(0, "Enter alarm configuration (0 = none, 1=critical level, 2=alarm level): ");
			if (sensor == IOBD2_SENSOR_REMOTE)
				offs = get_int(0, "Enter temperature offset: ");
			if (alarm < 0 || alarm > 2)
			{
				fprintf(stderr,
					"Invalid alarm type %d\n", 
					sensor);
				break;
			}
			if (dpci_ts_configure(sensor, high, low, crit, offs, alarm) == -1) 
			{
				fprintf(stderr,
					"Couldn't read register %d (%s): "
					"system error %d (%s), "
					"i2c error %d (%s)\n",
					lm89_regs[i].r_offs,
					lm89_regs[i].desc,
					errno,
					dpci_last_os_error_string(),
					dpci_i2c_last_error(),
					dpci_i2c_error_string(dpci_i2c_last_error()));
			}
			break;

		case 4:
			sensor = get_int(0, "Enter sensor number: ");
			if (sensor < 0 || sensor >= max_sensors)
			{
				fprintf(stderr,
					"Invalid sensor number %d\n", 
					sensor);
				break;
			}
			if (dpci_ts_getconfig(sensor, &high, &low, &crit, &offs, &alarm) == -1) 
			{
				fprintf(stderr,
					"Couldn't read register %d (%s): "
					"system error %d (%s), "
					"i2c error %d (%s)\n",
					lm89_regs[i].r_offs,
					lm89_regs[i].desc,
					errno,
					dpci_last_os_error_string(),
					dpci_i2c_last_error(),
					dpci_i2c_error_string(dpci_i2c_last_error()));
			}
			printf("Low temperature:      %d.\n", low);
			printf("High temperature:     %d.\n", high);
			printf("Critical temperature: %d.\n", crit);
			if (sensor == IOBD2_SENSOR_REMOTE)
				printf("Offset:               %d.\n", offs);
			printf("Alarm type:           %s.\n",
					alarm_names[alarm]);
			break;

		case 5:
			local_state = 4;	// bogus initialisation value.
			remote_state = 4;	// bogus initialisation value.
			if (dpci_ts_check_state(IOBD2_SENSOR_MASK_ALL, &local_state, &remote_state) == -1)
			{
				fprintf(stderr,
					"Couldn't check sensor alarm status: "
					"system error %d (%s), "
					"i2c error %d (%s)\n",
					errno,
					dpci_last_os_error_string(),
					dpci_i2c_last_error(),
					dpci_i2c_error_string(dpci_i2c_last_error()));
				break;
			}
			printf("Local  sensor state: %d - %s\n", local_state, dpci_ts_state_name(local_state));
			printf("Remote sensor state: %d - %s\n", remote_state, dpci_ts_state_name(remote_state));
			break;

		case 6:
			line = get_string("Enter register to read (Hex): ");
			addr = find_reg(line, 0);
			if (dpci_i2c_read8(LM89_BUS_ADDR, LM89_SLAVE_ADDR, addr, &byte) == -1)
			{
				fprintf(stderr,
					"Couldn't read register %d: "
					"system error %d (%s), "
					"i2c error %d (%s)\n",
					addr,
					errno,
					dpci_last_os_error_string(),
					dpci_i2c_last_error(),
					dpci_i2c_error_string(dpci_i2c_last_error()));
			}
			else
			{
				printf("Read 0x%02x from reg %d\n", byte, addr);
			}
			break;

		case 7:
			line = get_string("Enter register to write (Hex): ");
			addr = find_reg(line, 1);
			val = get_int(16, "Enter value to write (Hex): ");
			if (dpci_i2c_write8(LM89_BUS_ADDR, LM89_SLAVE_ADDR, addr, (unsigned char)val) == -1)
			{
				fprintf(stderr,
					"Couldn't write register %d: "
					"system error %d (%s), "
					"i2c error %d (%s)\n",
					addr,
					errno,
					dpci_last_os_error_string(),
					dpci_i2c_last_error(),
					dpci_i2c_error_string(dpci_i2c_last_error()));
			}
			else
			{
				printf("Written 0x%02x to reg %d\n", val, addr);
			}
			break;

		case 8:
			val = get_int(10, "Timeout in milliseconds: ");
			if (val < 0)
			{
				fprintf(stderr, "Invalid timout value.\n");
				break;
			}
			if ((ret = dpci_ts_wait_alarm(val)) == -1)
			{
				fprintf(stderr,
						"Couldn't wait for alarm: %s\n",
						dpci_last_os_error_string());
						break;
			}
			if (ret)
			{
				printf("Event!\n");
			}
			else
			{
				printf("Timeout!\n");
			}
			break;

#ifdef linux
		case 9:
			val = get_int(10, "Enter signal number to send: ");
			if (!byte || byte > NSIG)
			{
				fprintf(stderr, "Invalid signal number.\n");
				break;
			}
			if (dpci_ts_signal_alarm(val) == -1)
			{
				fprintf(stderr,
					"Couldn't set up signal: %s\n",
					dpci_last_os_error_string());
				break;
			}
			break;
#endif
		case 10:
			/*
			 * Show all readable registers.
			 */
			printf("\nRegister dump\n");
			printf("------------------------------------------\n");
			printf("Wreg  Rreg  Name  Data  Description\n");
			printf("------------------------------------------\n");
			for (i = 0; lm89_regs[i].name; i++)
			{
				/*
				 * Don't show write-only registers.
				 */
				if (lm89_regs[i].r_offs == -1)
					continue;
				if (dpci_i2c_read8(LM89_BUS_ADDR,
							LM89_SLAVE_ADDR,
							lm89_regs[i].r_offs,
							&byte) == -1)
				{
					fprintf(stderr,
						"Couldn't read register %d (%s): "
						"system error %d (%s), "
						"i2c error %d (%s)\n",
						(unsigned char)lm89_regs[i].r_offs,
						lm89_regs[i].desc,
						errno,
						dpci_last_os_error_string(),
						dpci_i2c_last_error(),
						dpci_i2c_error_string(dpci_i2c_last_error()));
					continue;
				}
				if (lm89_regs[i].w_offs > -1)
					printf("%4x  ", lm89_regs[i].w_offs);
				else
					printf("      ");
				printf("%4x  %-5s %4x  %s\n",
					lm89_regs[i].r_offs,
					lm89_regs[i].name,
					(int)byte,
					lm89_regs[i].desc);
					
			}
			printf("------------------------------------------\n");
			break;

		default:
			fprintf(stderr, "Not implemented yet.\n");
			break;
		} 
	}
	return 0;
}

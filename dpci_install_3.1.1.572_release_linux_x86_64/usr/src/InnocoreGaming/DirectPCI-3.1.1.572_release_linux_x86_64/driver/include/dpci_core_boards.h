/******************************************************************************
 *
 * $Id: dpci_core_boards.h 12197 2015-12-04 09:50:47Z aidan $
 *
 * Copyright 2003-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * Driver board descriptions
 *
 *****************************************************************************/

#ifndef _DPCI_CORE_BOARDS_H_
#define _DPCI_CORE_BOARDS_H_

#ifdef __linux
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/cdev.h>
#include <linux/dpci_core_ioctl.h>
#else
#include <stddcls.h>
#include <dpci_core.h>
#include <dpci_core_defines.h>
#include <dpci_core_priv.h>
#include <dpci_core_hw.h>
#endif
#include "dpci_core_hw.h"
#include "dpci_boards.h"
/****************************************************************************/
/*
 * DPX- and I/O-board descriptions. - Organized by CPLD version
 */
static const struct dpci_board boards[] = {
	{"DPX-116 DirectPCI I/O",
		DPX_116,
		2,
		BIOS_SIZE_512KB,

		0x34,
		HAVE_DENSITRON_IOEXP | HAVE_CLRWDT,
		DPCI_STDONBOARD_START,
		DPCI_STDONBOARD_SIZE,
		0,	/* Spare area on I/O board */
		0,	/* Spare area on I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL, dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL, dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1}},
		{{"BAT1", DPCI_REG0, DPCI_REG0_BATOK1},
		 {"BAT2", DPCI_REG0, DPCI_REG0_BATOK2}},
		multiboard_irq_handler,
	},
	{"DPX-116U DirectPCI I/O",
		DPX_116U,
		2,
		BIOS_SIZE_512KB,

		0x45,
		HAVE_NODIPSW | HAVE_CUSTOMER_IOEXP | HAVE_NODIPSW | HAVE_CLRWDT,
		DPX116U_ONBOARD_START,	/* Spare area on I/O board */
		DPX116U_ONBOARD_SIZE,	/* Spare area on I/O board */
		DPX116U_CUSTOMER_START,	/* Spare area on I/O board */
		DPX116U_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{DPX116U_IP0,
			DPX116U_IP1,
			DPX116U_IP2,
			DPX116U_IP3},
		{DPX116U_IP0INTSTAT},
		{DPX116U_IP0INTPOL},
		{DPX116U_IP0INTEN},
		{DPX116U_OP0,
			DPX116U_OP1,
			DPX116U_OP2,
			DPX116U_OP3,
			DPX116U_OP4},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},		
		{{0}},					/* No OW Ports */
		{{"BAT1", DPCI_REG0, DPCI_REG0_BATOK1},
		 {"BAT2", DPCI_REG0, DPCI_REG0_BATOK2}},
		oneboard_irq_handler,
	},
	{"DPX-112 DirectPCI I/O",
		DPX_112,
		0,
		BIOS_SIZE_512KB,

		0x64,
		HAVE_NODIPSW |
			HAVE_CUSTOMER_IOEXP | HAVE_CLRWDT |
			HAVE_EXINTEN | HAVE_IOINTONEXINT,
		DPX112_ONBOARD_START,	/* Spare area on I/O board */
		DPX112_ONBOARD_SIZE,	/* Spare area on I/O board */
		DPX112_CUSTOMER_START,	/* Spare area on I/O board */
		DPX112_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{DPX112_IP0,
			DPX112_IP1,
			DPX112_IP2,
			DPX112_IP3},
		{DPX112_IP0INTSTAT,
			DPX112_IP1INTSTAT,
			DPX112_IP2INTSTAT,
			DPX112_IP3INTSTAT},
		{DPX112_IP0INTPOL,
			DPX112_IP1INTPOL,
			DPX112_IP2INTPOL,
			DPX112_IP3INTPOL},
		{DPX112_IP0INTEN,
			DPX112_IP1INTEN,
			DPX112_IP2INTEN,
			DPX112_IP3INTEN},
		{DPX112_OP0,
			DPX112_OP1,
			DPX112_OP2,
			DPX112_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL, dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL, dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1}},
		{{0}},					/* no batteries */
		oneboard_irq_handler,
	},
	{"DPX-117 DirectPCI I/O",
		DPX_117,
		0,
		BIOS_SIZE_1MB,

		0x54,
		HAVE_DENSITRON_IOEXP | HAVE_CLRWDT,
		DPCI_STDONBOARD_START,	/* Spare area on I/O board */
		DPCI_STDONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA2, DPCI_REG1_I2CCLK2}},
		{{"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL, dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL, dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1}},
		{{"BAT1", DPCI_REG0, DPCI_REG0_BATOK1},
		 {"BAT2", DPCI_REG0, DPCI_REG0_BATOK2}},
		multiboard_irq_handler,
	},
	{"DPX-S410/S415/S420/S425/S430/S305 DirectPCI I/O",
		DPX_S410,						/* S410 board */
		1,								/* Number of batteries */
		BIOS_SIZE_1MB,					/* BIOS size */

		0x71,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_QMODE_PASSTHRU,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-S410/S415/S420/S425/S430/S305 DirectPCI I/O",
		DPX_S305,
		1,
		BIOS_SIZE_1MB,

		0x71,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_QMODE_PASSTHRU,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-S410/S415/S420/S425/S430/S305 DirectPCI I/O",
		DPX_E115,
		1,
		BIOS_SIZE_1MB,

		0x71,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-S410/S415/S420/S425/S430/S305 DirectPCI I/O",
		DPX_S415,
		2,
		BIOS_SIZE_1MB,

		0x71,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_1ROMSOCKET,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-S410/S415/S420/S425/S430/S305 DirectPCI I/O",
		DPX_S425,
		2,
		BIOS_SIZE_2MB,

		0x71,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_1ROMSOCKET,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-S410/S415/S420/S425/S430/S305 DirectPCI I/O",
		DPX_S430,
		2,
		BIOS_SIZE_4MB,

		0x71,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_1ROMSOCKET,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-S435 DirectPCI I/O", // and DAC-BJ08
		DPX_S435,
		2,
		BIOS_SIZE_4MB,

		0x4,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_MBDISINREG |
			HAVE_IDLP_QUICK |
			HAVE_1ROMSOCKET,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-E135 DirectPCI I/O",
		DPX_E135,
		3,
		BIOS_SIZE_8MB,

		0x4,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_MBDISINREG |
			HAVE_IDLP_QUICK |
			HAVE_1ROMSOCKET,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-S440 DirectPCI I/O",
		DPX_S440,
		3,
		BIOS_SIZE_8MB,

		0x4,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_MBDISINREG |
			HAVE_IDLP_QUICK |
			HAVE_1ROMSOCKET,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DirectPCIei development I/O",
		DPX_PCIE_DEVEL,
		2,
		BIOS_SIZE_512KB,

		0,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-C710 DirectPCI I/O",
		DPX_C710,
		2,
		BIOS_SIZE_4MB,

		0x71,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_NEWDIGIOREGS |
			HAVE_1ROMSOCKET,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN,
			GENIO_NEW_INTCFG2EVEN,
			GENIO_NEW_INTCFG3EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD,
			GENIO_NEW_INTCFG2ODD,
			GENIO_NEW_INTCFG3ODD},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3},
#ifdef __linux
		{0},	/* UARTs via MOSTEK PCI devices */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM/EXP", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK},
		},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184}},
		oneboard_irq_handler,		
	},
	{"DPX-E105/DPX-E120/DPX-C705/DPX-C605 DirectPCI I/O",
		DPX_C705C605,
		1,
		BIOS_SIZE_1MB,

		0x80,
		HAVE_DENSITRON_IOEXP | 
		HAVE_IDLPPLUS | 
		HAVE_QUIETMODE |
		HAVE_POWERFAILDETECT |
		HAVE_QMODE_PASSTHRU,
		DPCI_STDONBOARD_START,	/* Spare area on I/O board */
		DPCI_STDONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
#ifdef __linux
		{0},	/* UARTs via I/O board */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"SYSID",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			NULL,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER,
			DPCI_REG2, DPCI_REG2_SYSOW, DPCI_REG2_SYSOW},
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		{"MB1",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO1, DPCI_REG2_GPQ1, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		multiboard_irq_handler,
	},
	{"DPX-E105/DPX-E120/DPX-C705/DPX-C605 DirectPCI I/O",
		DPX_E105S,
		1,
		BIOS_SIZE_1MB,

		0x81,
		HAVE_DENSITRON_IOEXP | 
		HAVE_IDLPPLUS | 
		HAVE_QUIETMODE |
		HAVE_POWERFAILDETECT,
		DPCI_STDONBOARD_START,	/* Spare area on I/O board */
		DPCI_STDONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
#ifdef __linux
		{0},	/* UARTs via I/O board */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		multiboard_irq_handler,
	},
	{"DPX-E105/DPX-E120/DPX-C705/DPX-C605 DirectPCI I/O",
		DPX_E120S,
		1,
		BIOS_SIZE_2MB,

		0x81,
		HAVE_DENSITRON_IOEXP | 
		HAVE_IDLPPLUS | 
		HAVE_QUIETMODE |
		HAVE_POWERFAILDETECT,
		DPCI_STDONBOARD_START,	/* Spare area on I/O board */
		DPCI_STDONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
		{0},	/* Digital I/O via I/O board */
#ifdef __linux
		{0},	/* UARTs via I/O board */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{
		 {"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1},
		},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184},
		 {"BAT3", DPCI_BAT_REG_IDLP, 3, 184}},
		multiboard_irq_handler,
	},
	{"DPX-S4xx/S305/E130 DirectPCI I/O",
		DPX_E130,
		2,
		BIOS_SIZE_2MB,

		0x71,
		HAVE_CUSTOMER_IOEXP |
			HAVE_EXINTEN | HAVE_EXINT0 |
			HAVE_IDLPPLUS |
			HAVE_POWERFAILDETECT |
			HAVE_QUIETMODE |
			HAVE_NEWDIGIOREGS |
			HAVE_QMODE_PASSTHRU |
			HAVE_1ROMSOCKET,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT},
		{GENIO_NEW_INTCFG0EVEN,
			GENIO_NEW_INTCFG1EVEN},
		{GENIO_NEW_INTCFG0ODD,
			GENIO_NEW_INTCFG1ODD},
		{GENIO_OP0,
			GENIO_OP1},
#ifdef __linux
		{0},	/* UARTs via I/O board */
#else
		0,	/* no serial ports available */
#endif
		{{"MB/EEPROM", i2c_bitbang_init, i2c_bitbang_command,
			DPCI_REG1, DPCI_REG1_I2CDATA, DPCI_REG1_I2CCLK}},
		{{"MB0",
			dpci_ow_init_port,
			dpci_ow_touch_bit,
			dpci_ow_reset_pulse,
			dpci_ow_program_pulse,
			dpci_ow_set_port_level, dpci_ow_set_port_speed,
			DOWP_FLAGS_HAVE_ODRIVE | DOWP_FLAGS_HAVE_POWER | DOWP_FLAGS_HAVE_PROGRAM,
			DPCI_REG2, DPCI_REG2_GPIO0, DPCI_REG2_GPQ0, DPCI_REG2_PGM1}},
		{{"BAT1", DPCI_BAT_REG_IDLP, 1, 184},
		 {"BAT2", DPCI_BAT_REG_IDLP, 2, 184}},
		oneboard_irq_handler,
	},
	{"80-1003 I/O Board",
		DPCI_IOBOARD_ID_80_1003,
		0,
		0,

		0x03,
		HAVE_IOSHAREWDT|HAVE_DENSITRON_IOEXP,
		DPCI_IOBOARD03_ONBOARD_START,	/* Spare area on I/O board */
		DPCI_IOBOARD03_ONBOARD_SIZE,	/* Spare area on I/O board */
		DPCI_IOBOARD03_CUSTOMER_START,	/* Spare area on I/O board */
		DPCI_IOBOARD03_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{DPCI_IOBOARD03_IP0,
			DPCI_IOBOARD03_IP1},
		{DPCI_IOBOARD03_IP0INTSTAT},
		{DPCI_IOBOARD03_IP0INTPOL},
		{DPCI_IOBOARD03_IP0INTEN},
		{DPCI_IOBOARD03_OP0,
			DPCI_IOBOARD03_OP1,
			DPCI_IOBOARD03_OP2,
			DPCI_IOBOARD03_OP3,
			DPCI_IOBOARD03_OP4},
#ifdef __linux
		{DPCI_IOBOARD03_COM3,
			DPCI_IOBOARD03_COM4,
			DPCI_IOBOARD03_COM5,
			DPCI_IOBOARD03_COM6},
#else
		1,	/* serial ports available */
#endif
		{{0}},	/* No I2C Ports */
		{{0}},	/* No OW Ports */
		{{0}},	/* No Batteries */
		ioboard_80_1003_irq_handler,
	},
	{"80-0062/A I/O Board II",
		DPCI_IOBOARD_ID_80_0062A,
		0,
		0,

		0x02,
		0,
		GENIO_ONBOARD_START,	/* Spare area on I/O board */
		GENIO_ONBOARD_SIZE,	/* Spare area on I/O board */
		GENIO_CUSTOMER_START,	/* Spare area on I/O board */
		GENIO_CUSTOMER_SIZE,	/* Spare area on I/O board */
		{GENIO_IP0,
			GENIO_IP1,
			GENIO_IP2,
			GENIO_IP3},
		{GENIO_IP0INTSTAT,
			GENIO_IP1INTSTAT,
			GENIO_IP2INTSTAT,
			GENIO_IP3INTSTAT},
		{GENIO_IP0INTPOL,
			GENIO_IP1INTPOL,
			GENIO_IP2INTPOL,
			GENIO_IP3INTPOL},
		{GENIO_IP0INTEN,
			GENIO_IP1INTEN,
			GENIO_IP2INTEN,
			GENIO_IP3INTEN},
		{GENIO_OP0,
			GENIO_OP1,
			GENIO_OP2,
			GENIO_OP3,
			GENIO_OP4},
#ifdef __linux
		{GENIO_COM3,
			GENIO_COM4,
			GENIO_COM5,
			GENIO_COM6,
			GENIO_COM7,
			GENIO_COM8,
			GENIO_COM9,
			GENIO_COM10},
#else
		1,		/* serial ports available */
#endif
		{{"I/O EEPROM",
			i2c_bitbang_init, i2c_bitbang_command,
			DPCI_IOBOARD62_I2CEEPROM,
			DPCI_IOBOARD62_I2CDATA, DPCI_IOBOARD62_I2CCLK},
		{"I/O Thermal",
			i2c_bitbang_init, i2c_bitbang_command,
			DPCI_IOBOARD62_I2CTHERMAL,
			DPCI_IOBOARD62_I2CDATA, DPCI_IOBOARD62_I2CCLK},
		{"Backplane",
			i2c_bitbang_init, i2c_bitbang_command,
			DPCI_IOBOARD62_I2CBP,
			DPCI_IOBOARD62_I2CDATA, DPCI_IOBOARD62_I2CCLK}},
		{{0}},	/* No OW Ports */
		{{0}},	/* No Batteries */
		ioboard_80_0062_irq_handler,
	},
	{0},
};

/*******************************************************************************
 *
 * Function:	dpci_find_board()
 *
 * Parameters:	tag - Board tag value to look for.
 *
 * Returns:		On success the function returns a pointer to a board 
 *				description structure from the specified pBoardList static
 *				array. If it fails, it returns NULL.
 *
 * Description:	The function searches the specified array of board description
 *				tables to find a match for the specified iTag.
 *
 ******************************************************************************/
const struct dpci_board* dpci_find_board(int board_id)
{
	const struct dpci_board *dbp;

	for (dbp = boards; dbp->db_name; dbp++)
	{
		if (dbp->db_board_id == board_id)
		{
			return (dbp);
		}
	}

	return NULL;
}

#endif /*_DPCI_CORE_BOARDS_H_*/

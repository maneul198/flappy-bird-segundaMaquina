/******************************************************************************
 *
 * $Id: dpci_core_hw.h 11962 2015-09-24 14:23:39Z aidan $
 *
 * Copyright 2003-2014 Advantech Co Ltd.
 * All rights reserved.
 *
 * Description:
 * The header file which contains common macros and structures that are used.
 * both in windows and linux dpci drivers.
 *
 *****************************************************************************/
#ifndef DPCI_CORE_HW_H
#define DPCI_CORE_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#define DENSITRON_VENDOR_ID				0x16cd
#define DPCI_DEVICE_CORE114_ID			0x0105
#define DPCI_DEVICE_CORE116_ID			0x0106
#define DPCI_DEVICE_CORE116U_ID			0x0107
#define DPCI_DEVICE_CORE117_ID			0x0108
#define DPCI_DEVICE_CORE112_ID			0x0109
#define DPCI_DEVICE_CORE_C705605_ID		0x010a
#define DPCI_DEVICE_CORE_S410305_ID		0x010b
#define DPCI_DEVICE_CORE_PCIE_ID		0x0180
#define DPCI_DEVICE_CORE_INVALID_ID		0x0
#define DENSITRON_CORE_REVISION_116		0x30
#define DENSITRON_CORE_REVISION_116U	0x40
#define DPCI_S425_HB_VENID		0x8086	 /* S425 Hostbridge */
#define DPCI_S425_HB_DEVID1		0x006a
#define DPCI_S425_HB_DEVID2		0x0044
#define DPCI_E120_HB_VENID		0x1022	 /* E120 Hostbridge */
#define DPCI_E120_HB_DEVID		0x1510


/******************************************************************************/
/*
 * These macros define the standard registers in the DPCI chip's I/O space.
 */
#define DPCI_STDONBOARD_START		0x00
#define DPCI_STDONBOARD_SIZE		0x10

/*
 * These macros define the standard registers in the DPCI chip's I/O space.
 */
#define DPCI_REG0			0x00
#define		DPCI_REG0_SPISDI		0x01 /* read only */
#define		DPCI_REG0_SPISS			0x01 /* write only */
#define		DPCI_REG0_EXINT0		0x01 /* read only - DPX-S*,C* */
#define		DPCI_REG0_LOGINT		0x02 /* read only */
#define		DPCI_REG0_SPISCK		0x02 /* write only */
#define		DPCI_REG0_IOINT			0x04 /* read only */
#define		DPCI_REG0_SPISDO		0x04 /* write only */
#define		DPCI_REG0_IOINTEN		0x08 /* read write */
#define		DPCI_REG0_BATOK1		0x10 /* read only 114/115/116/117 */
#define		DPCI_REG0_EXIOAD		0x10 /* write only */
#define		DPCI_REG0_BATOK2		0x20 /* read only 114/115/116/117 */
#define		DPCI_REG0_PICACK		0x10 /* read-only, C705/C605/S410/S305 */
#define		DPCI_REG0_ACKIEN		0x10 /* write-only, C705/C605/S410/S305 */
#define		DPCI_REG0_PFD			0x20 /* read-only, C705/C605/S410/S305 */
#define		DPCI_REG0_PFDIEN		0x20 /* write-only, C705/C605/S410/S305 */
#define		DPCI_REG0_EXINT			0x20 /* read only */
#define		DPCI_REG0_ROMEXT		0x40 /* read only */
#define		DPCI_REG0_PICINTEN		0x40
#define		DPCI_REG0_EXINTEN		0x80
#define DPCI_REG1			0x01
#define		DPCI_REG1_DSW1			0x01 /* read only */
#define		DPCI_REG1_DSW2			0x02 /* read only */
#define		DPCI_REG1_DSW3			0x04 /* read only */
#define		DPCI_REG1_DSW4			0x08 /* read only */
#define		DPCI_REG1_HSB1			0x01 /* write only - DPX116*/
#define		DPCI_REG1_HSB2			0x02 /* write only - DPX116*/
#define		DPCI_REG1_CLRWDT		0x08 /* write only */
#define		DPCI_REG1_QMPT			0x10 /* read/write - C705/C605/S410/S305 */
#define		DPCI_REG1_QMLV			0x20 /* read/write - C705/C605/S410/S305 */
#define		DPCI_REG1_I2CCLK2		0x10 /* read/write - only DPX117 */
#define		DPCI_REG1_I2CDATA2		0x20 /* read/write - only DPX117 */
#define		DPCI_REG1_I2CCLK		0x40 /* read/write - DPX116U/116r1.1/112 */
#define		DPCI_REG1_I2CDATA		0x80 /* read/write - DPX116U/116r1.1/112 */
#define	DPCI_REG2			0x02
#define		DPCI_REG2_GPQ0			0x01 /* read/write, since DPX116r1.1 */
#define		DPCI_REG2_GPQ1			0x02 /* read/write, since DPX116r1.1 */
#define		DPCI_REG2_PGM0			0x04 /* write-only, C705/C605/S410/S305 */
#define		DPCI_REG2_PGM1			0x08 /* write-only, C705/C605/S410/S305 */
#define		DPCI_REG2_SRAM0			0x04 /* read-only, PCIe f/w */
#define		DPCI_REG2_SRAM1			0x08 /* read-only, PCIe f/w */
#define		DPCI_REG2_GPIO0			0x10 /* read-only, 112,116r1.1,117 */ /* read/write, 112,116r1.1,117 */
#define		DPCI_REG2_GPIO1			0x20 /* read-only, since DPX116r1.1 */
#define		DPCI_REG2_PICE0			0x10 /* write-only, since DPX116r1.1 */ /* read/write, 112,116r1.1,117 */
#define		DPCI_REG2_PICE1			0x20 /* write-only, since DPX116r1.1 */
#define		DPCI_REG2_PCIE_ROM		0x40 /* read-only, PCIe f/w, 0 if JP10 is jumpered to enable ROM */
#define		DPCI_REG2_PICEALL		(DPCI_REG2_PICE0 | DPCI_REG2_PICE1)
#define		DPCI_REG2_SYSOW			0x80 /* read/write, C705/C605/S410/S305 */
#define	DPCI_REGIDLPDATA		0x03 /* DPX-S410/S305/C705/C605 */


/*
 * DPX 116 rev.1.0 I2C registers. Firmware 0x34 and earlier.  Doesn't work very
 * well so please ask us to have your firmware upgraded.
 */
#define DPCI_REGI2CDATA			0x02 /* read/write - only DPX116 */
#define DPCI_REGI2CCTL			0x03 /* only DPX116 */
#define 	DPCI_REGI2CCTL_STATUS_MASK	0x03 /* read only */
#define 	DPCI_REGI2CCTL_STATUS_INACT	0x00
#define 	DPCI_REGI2CCTL_STATUS_ABORT	0x01
#define 	DPCI_REGI2CCTL_STATUS_ACTIVE	0x02
#define 	DPCI_REGI2CCTL_STATUS_DREADY	0x03
#define 	DPCI_REGI2CCTL_RW		0x01 /* write only */

/*
 * Registers for all I/O boards.
 *
 * 80-1003:   I/O Board: 40 output lines, 16 inputs (8 interrupts), 16C554
 * 80-0062/A: I/O Board: 40 output lines, 32 inputs (32 intr), 2x 16C554
 */
#define DPCI_IOBOARD_ID				0x10
#define 	DPCI_IOBOARD_ID_80_1003			0x03
#define 	DPCI_IOBOARD_ID_80_0062A		0x15
#define 	DPCI_IOBOARD_ID_NONE			0xff
#define DPCI_IOBOARD_PRESENT(id)	\
			(((id) != DPCI_IOBOARD_ID_NONE) && \
			((id) != 0x10) && \
			((id) != 0x00))
#define DPCI_IOBOARD_REV			0x11
#define DPCI_IOBOARD_INTCFG			0x1f
#define DPCI_IOBOARD_INTSTAT			DPCI_IOBOARD_INTCFG 
#define DPCI_IOBOARD_INTCFG_OE			0x80
#define DPCI_IOBOARD_INTCFG_WDRC		0x40
#define DPCI_IOBOARD_INTCFG_IO			0x20

/*
 * Registers for the 80-1003 I/O board.
 */
#define DPCI_IOBOARD03_IP0			0x16
#define DPCI_IOBOARD03_IP1			0x17
#define DPCI_IOBOARD03_OP0			0x18
#define DPCI_IOBOARD03_OP1			0x19
#define DPCI_IOBOARD03_OP2			0x1a
#define DPCI_IOBOARD03_OP3			0x1b
#define DPCI_IOBOARD03_OP4			0x1c
#define 	DPCI_IOBOARD03_IP0INTPOL	0x1d
#define 	DPCI_IOBOARD03_IP0INTSTAT	DPCI_IOBOARD03_IP0INTPOL
#define 	DPCI_IOBOARD03_IP0INTEN		0x1e
#define DPCI_IOBOARD03_COM3			0x20
#define DPCI_IOBOARD03_COM4			0x28
#define DPCI_IOBOARD03_COM5			0x30
#define DPCI_IOBOARD03_COM6			0x38
#define	DPCI_IOBOARD03_ONBOARD_START		0x10
#define	DPCI_IOBOARD03_ONBOARD_SIZE		0x10 	/* 16 bytes space */
#define	DPCI_IOBOARD03_CUSTOMER_START		0x40
#define	DPCI_IOBOARD03_CUSTOMER_SIZE		0xc0 	/* 192 bytes space */
#define DPCI_IOBOARD03_INTCFG_COMWD		0x10

/*
 * Registers for the 80-0062 I/O board.
 */
#define DPCI_IOBOARD62_I2CBP		0x1C
#define DPCI_IOBOARD62_I2CTHERMAL	0x1D
#define DPCI_IOBOARD62_I2CEEPROM	0x1E
#define     DPCI_IOBOARD62_I2CCLK	0x40 /* read/write - DPX112/DPX116U */
#define     DPCI_IOBOARD62_I2CDATA	0x80 /* read/write - DPX112/DPX116U */
#define DPCI_IOBOARD62_INTCFG_COM	0x10
#define DPCI_IOBOARD62_INTCFG_WD	0x08
#define DPCI_IOBOARD62_INTCFG_TS	0x04

/*
 * Registers for the DPX116U board with on-board I/O.
 */
#define	DPX116U_ONBOARD_START			0x00
#define	DPX116U_ONBOARD_SIZE			0x10
#define	DPX116U_CUSTOMER_START			0x10
#define	DPX116U_CUSTOMER_SIZE			0xf0 /* 240 bytes space */
#define DPX116U_IP0INTPOL   0x02
#define DPX116U_IP0INTSTAT  DPX116U_IP0INTPOL
#define DPX116U_IP0INTEN    0x03
#define DPX116U_IP0     0x04
#define DPX116U_IP1     0x05
#define DPX116U_IP2     0x06
#define DPX116U_IP3     0x07
#define DPX116U_OP0     0x08
#define DPX116U_OP1     0x09
#define DPX116U_OP2     0x0a
#define DPX116U_OP3     0x0b
#define DPX116U_OP4     0x0c

/*
 * Registers for the DPX112 board with on-board I/O.
 */
#define DPX112_IP0INTPOL    0x04
#define DPX112_IP1INTPOL    0x05
#define DPX112_IP2INTPOL    0x06
#define DPX112_IP3INTPOL    0x07
#define DPX112_IP0INTSTAT   0x0c
#define DPX112_IP1INTSTAT   0x0d
#define DPX112_IP2INTSTAT   0x0e
#define DPX112_IP3INTSTAT   0x0f
#define DPX112_IP0INTEN     0x0c
#define DPX112_IP1INTEN     0x0d
#define DPX112_IP2INTEN     0x0e
#define DPX112_IP3INTEN     0x0f
#define DPX112_IP0      0x04
#define DPX112_IP1      0x05
#define DPX112_IP2      0x06
#define DPX112_IP3      0x07
#define DPX112_OP0      0x08
#define DPX112_OP1      0x09
#define DPX112_OP2      0x0a
#define DPX112_OP3      0x0b

#define	DPX112_ONBOARD_START			0x00
#define	DPX112_ONBOARD_SIZE				0x10 /* 16 bytes space */
#define	DPX112_CUSTOMER_START			0x10
#define	DPX112_CUSTOMER_SIZE			0xf0 /* 240 bytes space */

/*
 * Generic registers for new I/O facilities - not all of these are available on
 * all boards.
 */
#define	GENIO_ONBOARD_START				0x10
#define	GENIO_ONBOARD_SIZE				0x30 /* 64 bytes space */
#define	GENIO_CUSTOMER_START			0x40
#define	GENIO_CUSTOMER_SIZE				0x40 /* 64 bytes space */
#define	GENIO_OP0						0x20
#define	GENIO_OP1						0x21
#define	GENIO_OP2						0x22
#define	GENIO_OP3						0x23
#define	GENIO_OP4						0x24
#define	GENIO_OP5						0x25
#define	GENIO_OP6						0x26
#define	GENIO_OP7						0x27
#define	GENIO_OP8						0x28
#define	GENIO_OP9						0x29
#define	GENIO_OP10						0x2a
#define	GENIO_OP11						0x2b
#define	GENIO_OP12						0x2c
#define	GENIO_OP13						0x2d
#define	GENIO_OP14						0x2e
#define	GENIO_OP15						0x2f
#define	GENIO_IP0						0x30
#define	GENIO_IP0INTPOL					0x30
#define	GENIO_IP0INTEN					0x38
#define	GENIO_IP0INTSTAT				0x38
#define	GENIO_IP1						0x31
#define	GENIO_IP1INTPOL					0x31
#define	GENIO_IP1INTEN					0x39
#define	GENIO_IP1INTSTAT				0x39
#define	GENIO_IP2						0x32
#define	GENIO_IP2INTPOL					0x32
#define	GENIO_IP2INTEN					0x3a
#define	GENIO_IP2INTSTAT				0x3a
#define	GENIO_IP3						0x33
#define	GENIO_IP3INTPOL					0x33
#define	GENIO_IP3INTEN					0x3b
#define	GENIO_IP3INTSTAT				0x3b
#define	GENIO_IP4						0x34
#define	GENIO_IP4INTPOL					0x34
#define	GENIO_IP4INTEN					0x3c
#define	GENIO_IP4INTSTAT				0x3c
#define	GENIO_IP5						0x35
#define	GENIO_IP5INTPOL					0x35
#define	GENIO_IP5INTEN					0x3d
#define	GENIO_IP5INTSTAT				0x3d
#define	GENIO_IP6						0x36
#define	GENIO_IP6INTPOL					0x36
#define	GENIO_IP6INTEN					0x3e
#define	GENIO_IP6INTSTAT				0x3e
#define	GENIO_IP7						0x37
#define	GENIO_IP7INTPOL					0x37
#define	GENIO_IP7INTEN					0x3f
#define	GENIO_IP7INTSTAT				0x3f
#define	GENIO_NEW_INTCFG0EVEN				0x30
#define	GENIO_NEW_INTCFG1EVEN				0x31
#define	GENIO_NEW_INTCFG2EVEN				0x32
#define	GENIO_NEW_INTCFG3EVEN				0x33
#define	GENIO_NEW_INTCFG4EVEN				0x34
#define	GENIO_NEW_INTCFG5EVEN				0x35
#define	GENIO_NEW_INTCFG6EVEN				0x36
#define	GENIO_NEW_INTCFG7EVEN				0x37
#define	GENIO_NEW_INTCFG0ODD				0x38
#define	GENIO_NEW_INTCFG1ODD				0x39
#define	GENIO_NEW_INTCFG2ODD				0x3a
#define	GENIO_NEW_INTCFG3ODD				0x3b
#define	GENIO_NEW_INTCFG4ODD				0x3c
#define	GENIO_NEW_INTCFG5ODD				0x3d
#define	GENIO_NEW_INTCFG6ODD				0x3e
#define	GENIO_NEW_INTCFG7ODD				0x3f
#define	GENIO_COM3						0x80
#define	GENIO_COM4						0x90
#define	GENIO_COM5						0xa0
#define	GENIO_COM6						0xb0
#define	GENIO_COM7						0xc0
#define	GENIO_COM8						0xd0
#define	GENIO_COM9						0xe0
#define	GENIO_COM10						0xf0

#define	IS_SYS_REG(reg) (((reg) < 4) || ((reg) == 0x1f))


/*
 * I2C bus identifiers for I/O BD II hardware.
 *
 * You need to use dpci_i2c_getbusnumber() to obtain the bus number.  While
 * the result is generally fixed, moving between mainboards where the number of
 * on-board I2C busses is different will cause the I/O board I2c bus numbers to
 * change too.
 */
#define	IOBD_II_EEPROM_BUS_NAME		"IO EEPROM"
#define	IOBD_II_THERMAL_BUS_NAME	"IO Thermal"
#define	IOBD_II_BP_BUS_NAME		"Backplane"

/*
 * I2C peripheral address definitions
 *
 */
#define DPCI_I2C_ADDR_C710_EEPROM	0xae
#define DPCI_I2C_ADDR_C710_IDENT	0xa0
#define DPCI_I2C_ADDR_EEPROM_DEFAULT	0xa0
#define DPCI_I2C_ADDR_EEPROM_NEW	0xae	/* C710/E135/S440 and newer */

#define DPCI_I2C_DEVICE_FAMILY_DS28CM00	0x70

/*
 * These LM89 register definitions are quoted here for your convenience.
 * Registers can be read using dpci_i2c_read8() and dpci_i2c_write8().  You
 * should avoid using dpci_i2c_read() and dpci_i2c_write() unless you use a
 * page-size of 1; larger page-size settings may cause the LM89 to lock-up.
 *
 * For further information consult the LM89 datasheet (lm89cim.pdf) available
 * on the Internet from many stockists.
 */
#define LM89_BUS_ADDR		2
#define LM89_SLAVE_ADDR		0x98
#define LM89_REG_LT		0x00
#define LM89_REG_RTHB		0x01
#define LM89_REG_SR		0x02
#define 	LM89_REG_SR_LCRIT	0x01
#define 	LM89_REG_SR_RCRIT	0x02
#define 	LM89_REG_SR_OPENCCT	0x04
#define 	LM89_REG_SR_RLOW	0x08
#define 	LM89_REG_SR_RHIGH	0x10
#define 	LM89_REG_SR_LLOW	0x20
#define 	LM89_REG_SR_LHIGH	0x40
#define 	LM89_REG_SR_BUSY	0x80
#define LM89_REG_C_R		0x03
#define		LM89_REG_C_FAULTQ	0x01
#define 	LM89_REG_C_LCLCRIT	0x04
#define 	LM89_REG_C_REMCRIT	0x10
#define 	LM89_REG_C_STOP		0x40
#define 	LM89_REG_C_ALERTN	0x80
#define LM89_REG_CR_R		0x04
#define LM89_REG_LHS_R		0x05
#define LM89_REG_LLS_R		0x06
#define LM89_REG_RHSHB_R	0x07
#define LM89_REG_RLSHB_R	0x08
#define LM89_REG_C_W		0x09
#define LM89_REG_CR_W		0x0a
#define LM89_REG_LHS_W		0x0b
#define LM89_REG_LLS_W		0x0c
#define LM89_REG_RHSHB_W	0x0d
#define LM89_REG_RLSHB_W	0x0e
#define LM89_REG_ONESHOT	0x0f
#define	LM89_REG_RTLB		0x10
#define	LM89_REG_RTOHB		0x11
#define	LM89_REG_RTOLB		0x12
#define	LM89_REG_RHSLB		0x13
#define	LM89_REG_RLSLB		0x14
#define	LM89_REG_RCS		0x19
#define	LM89_REG_LCS		0x20
#define	LM89_REG_TH		0x21
#define	LM89_REG_RDTF		0xBF
#define	LM89_REG_RMID		0xFE
#define	LM89_REG_RDR		0xFF

/*
 * IDLP and IDLP+ commands.
 */
#define DPCI_IDLP_CMD_GET_NUM_EVENTS		0x00
#define DPCI_IDLP_CMD_GET_EVENT			0x01
#define DPCI_IDLP_CMD_SET_DATE_TIME		0x02
#define DPCI_IDLP_CMD_GET_DATE_TIME		0x03
#define DPCI_IDLP_CMD_INT_ENABLE		0x04
#define DPCI_IDLP_CMD_INT_DISABLE		0x05
#define DPCI_IDLP_CMD_SET_WD_TIMEOUT		0x06
#define DPCI_IDLP_CMD_CLEAR_WD			0x07
#define DPCI_IDLP_CMD_GET_FW_VERSION		0x08
#define DPCI_IDLP_CMD_SET_SLEEP_TIME		0x09
#define DPCI_IDLP_CMD_GET_INTRUSION_STATUS	0x0a
#define DPCI_IDLP_CMD_GET_NUM_EVENTS_3B		0x0b	// f/w versions 25-32
#define DPCI_IDLP_CMD_SELECT_EVENT		0x0c
#define DPCI_IDLP_CMD_GET_SELECTED_EVENT	0x0d
#define DPCI_IDLP_CMD_GET_FW_BUILD_NUM		0x0e	// f/w version < 40
#define DPCI_IDLP_CMD_SELECT_PROM_BYTE		0x0e
#define DPCI_IDLP_CMD_GET_PROM_BYTE		0x0f
#define DPCI_IDLP_CMD_GET_BATT_STATUS		0x10
#define DPCI_IDLP_CMD_GET_BATT1_VOLTAGE		0x11
#define DPCI_IDLP_CMD_GET_BATT2_VOLTAGE		0x12
#define DPCI_IDLP_CMD_GET_BATT3_VOLTAGE		0x13
#define DPCI_IDLP_CMD_SET_BATT_CHECK_PERIOD	0x14
#define DPCI_IDLP_CMD_SET_BATT1_ERROR_LEVEL	0x16
#define DPCI_IDLP_CMD_SET_BATT2_ERROR_LEVEL	0x17
#define DPCI_IDLP_CMD_SET_BATT3_ERROR_LEVEL	0x18
#define DPCI_IDLP_CMD_GET_FW_VERSION_FULL	0x19	// f/w version > 52
#define DPCI_IDLP_CMD_CLEAR_CASEOPEN_LED_FLAG	0x1a  //only for DPX- E130
#define DPCI_IDLP_CMD_GET_LAST_EVENT		0x1b	// f/w version >= 59.13
#define DPCI_IDLP_CMD_GET_CHECKSUM		0x1c	// f/w version >= 59.13

#define	DPCI_IDLP_ACKNOWLEDGE			0xaa
#define	DPCI_IDLP_ERR_DATA_REQUIRED		0xbe
#define	DPCI_IDLP_ERR_INVAL_CMD			0xce
#define	DPCI_IDLP_ERR_INVAL_DATA		0xde
#define	DPCI_IDLP_ERR_EXECUTION			0xee
#define	DPCI_IDLP_ERR_EXCESS_DATA		0xfe

#define IDLP_VERSION_MAJOR(verfull)	((verfull) & 0xff)
#define IDLP_VERSION_BUILD(verfull)	(((verfull) >> 8) & 0xff)
#define IDLP_VERSION_CONF1(verfull)	(((verfull) >> 16) & 0xff)
#define IDLP_VERSION_CONF2(verfull)	(((verfull) >> 24) & 0xff)

#define IDLP_VERSION_CONF2_ISDEBUG	0x01
#define IDLP_VERSION_CONF2_NOTAUTOBUILD	0x02
#define IDLP_VERSION_CONF2_DIAGMODE	0x04
#define IDLP_VERSION_CONF2_1LIBATT	0x08
#define IDLP_VERSION_CONF2_8MHZ		0x10
#define IDLP_VERSION_CONF2_INSTRUMENTED	0x20
#define IDLP_VERSION_CONF2_INTR67	0x80

/*
 * Constants relating to various IDLP/IDLP+ commands.
 */
#define	DPCI_IDLP_MAX_EVENTS			(255)

#define INTRUSION0_MASK 0x01
#define INTRUSION1_MASK 0x02
#define INTRUSION2_MASK 0x04
#define INTRUSION3_MASK 0x08
#define INTRUSION4_MASK 0x10
#define INTRUSION5_MASK 0x20
#define INTRUSION6_MASK 0x40
#define INTRUSION7_MASK 0x80

#define BAT_CHECKPERIOD_NONE	0
#define BAT_CHECKPERIOD_1MIN	1
#define BAT_CHECKPERIOD_1HOUR	2
#define BAT_CHECKPERIOD_1DAY	3
#define BAT_CHECKPERIOD_1MONTH	4

/*
 * Identifiers for IDLP event codes
 */
#define	ID_EVENT_NONE		0
#define	ID_EVENT_SYSRESET_10	5	/* system reset went low. */
#define	ID_EVENT_SYSRESET_01	6	/* system reset went high. */
#define	ID_EVENT_WDRESET		7	/* watchdog processor time-out */
#define	ID_EVENT_INTERNALRESET_POWERUP	8	/* IDLP internal reset: power up */
#define	ID_EVENT_PROCRESET		8	/* intrusion detector processor reset */
#define	ID_EVENT_OLDTIME		17	/* old time and date */
#define	ID_EVENT_NEWTIME		18	/* new time and date */
#define	ID_EVENT_INTERNALRESET_WDT	19	/* IDLP internal reset: WDT */
#define	ID_EVENT_INTERNALRESET		19	/* old name for compatibility */
#define	ID_EVENT_INTERNALRESET_MCLR	20	/* IDLP internal reset: MCLR */
#define	ID_EVENT_DIAGENABLED	21	/* Diagnostic port enabled */
#define	ID_EVENT_BATT1TOOLOW	22	/* battery 1 too low */
#define	ID_EVENT_BATT1CORRECT	23	/* battery 1 ok now */
#define	ID_EVENT_BATT2TOOLOW	24	/* battery 2 too low */
#define	ID_EVENT_BATT2CORRECT	25	/* battery 2 ok now */
#define	ID_EVENT_BATT3TOOLOW	26	/* battery 3 too low */
#define	ID_EVENT_BATT3CORRECT	27	/* battery 3 ok now */
#define	ID_EVENT_IDLPRESETNOEE	28	/* reset but no event eeprom */
#define	ID_EVENT_INTERNALRESET_BOR	33	/* IDLP internal reset: BOR */
#define	ID_EVENT_FIRMWARE_CHECKSUM_ERROR	34	/* Firmware Checksum Error */
#define	MAX_ID_EVENT		34	/* Highest event number */

/*
 * Pre-v3.0.0 event codes for Intrusion events 
 */
#define	ID_EVENT_INTRUS0_OPEN	1	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS1_OPEN	2	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS2_OPEN	3	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS3_OPEN	4	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS0_CLOSE	9	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS1_CLOSE	10	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS2_CLOSE	11	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS3_CLOSE	12	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS4_CLOSE	13	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS5_CLOSE	14	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS4_OPEN	15	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS5_OPEN	16	/* closed circuit (c/c) i.e. 1->0 */

/*
 * New event codes (from v3.0.0)
 */
#define	ID_EVENT_INTRUS0_CLOSED_CIRCUIT	1	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS1_CLOSED_CIRCUIT	2	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS2_CLOSED_CIRCUIT	3	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS3_CLOSED_CIRCUIT	4	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS0_OPEN_CIRCUIT	9	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS1_OPEN_CIRCUIT	10	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS2_OPEN_CIRCUIT	11	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS3_OPEN_CIRCUIT	12	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS4_OPEN_CIRCUIT	13	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS5_OPEN_CIRCUIT	14	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS4_CLOSED_CIRCUIT	15	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS5_CLOSED_CIRCUIT	16	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS6_OPEN_CIRCUIT	29	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS7_OPEN_CIRCUIT	30	/* open circuit (o/c) i.e. 0->1 */
#define	ID_EVENT_INTRUS6_CLOSED_CIRCUIT	31	/* closed circuit (c/c) i.e. 1->0 */
#define	ID_EVENT_INTRUS7_CLOSED_CIRCUIT	32	/* closed circuit (c/c) i.e. 1->0 */

/*
 * IDLP Event names
 */
#define DECLARE_EVENT_NAME_ARRAY(array) \
    const char *array[] =  {"no event ",	/* 00 */ \
	"intrusion line#0 c/c",					/* 01 */ \
	"intrusion line#1 c/c",                 /* 02 */ \
	"intrusion line#2 c/c",                 /* 03 */ \
	"intrusion line#3 c/c",                 /* 04 */ \
	"system-reset 1->0",                    /* 05 */ \
	"system-reset 0->1",                    /* 06 */ \
	"watchdog reset host",                  /* 07 */ \
	"IDLP internal reset: power up",	/* 08 */ \
	"intrusion line#0 o/c",                 /* 09 */ \
	"intrusion line#1 o/c",                 /* 10 */ \
	"intrusion line#2 o/c",                 /* 11 */ \
	"intrusion line#3 o/c",                 /* 12 */ \
	"intrusion line#4 o/c",                 /* 13 */ \
	"intrusion line#5 o/c",                 /* 14 */ \
	"intrusion line#4 c/c",                 /* 15 */ \
	"intrusion line#5 c/c",                 /* 16 */ \
	"current date & time",                  /* 17 */ \
	"new date & time",                      /* 18 */ \
	"IDLP internal reset: WDT",		/* 19 */ \
	"IDLP internal reset: MCLR",		/* 20 */ \
	"Diagnostic port enabled",              /* 21 */ \
	"Battery 1 too low",                    /* 22 */ \
	"Battery 1 correct",                    /* 23 */ \
	"Battery 2 too low",                    /* 24 */ \
	"Battery 2 correct",                    /* 25 */ \
	"Battery 3 too low",                    /* 26 */ \
	"Battery 3 correct",                    /* 27 */ \
	"IDLP internal reset: no log EEPROM",	/* 28 */ \
	"intrusion line#6 o/c",                 /* 29 */ \
	"intrusion line#7 o/c",                 /* 30 */ \
	"intrusion line#6 c/c",                 /* 31 */ \
	"intrusion line#7 c/c",                 /* 32 */ \
	"IDLP internal reset: BOR",		/* 33 */ \
	"Firmware Checksum Error",		/* 34 */ \
	}



#ifdef __cplusplus
}
#endif

#endif


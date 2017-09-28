/******************************************************************************
 *
 * $Id: dpci_api_types.h 12273 2015-12-23 15:23:30Z aidan $
 *
 * Copyright 2011-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * DirectPCI common data structures and types for core module 
 *
 *****************************************************************************/
#ifndef _DPCIAPITYPES_H
#define _DPCIAPITYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Some constants useful for people.
 */
#define MAX_INPUT_PORTS		8
#define MAX_OUTPUT_PORTS	16
#define MAX_I2C_PORTS		8
#define MAX_OW_PORTS		8
#define MAX_BATTERIES		8
#define MAX_GPIO_LINES		2
#define MAX_UARTS		8

/******************************************************************************/

/*
 * This type define is used hold offsets of registers within DPCI I/O space.
 */
typedef unsigned char dpci_off_t;

/*
 * DPCI direct commands - for debugging only.
 */
struct dpci_idlp_cmdbuf
{
	int dic_n_tx_bytes;
	int dic_n_rx_bytes;
	unsigned char dic_tx_bytes[64];
	unsigned char dic_rx_bytes[64];
};

/*
 * Used for get-time, set-time and read-event commands.
 */
typedef struct idevent {
   unsigned char eventcode;
   unsigned char year;
   unsigned char month;
   unsigned char day;
   unsigned char hour;
   unsigned char min;
   unsigned char sec;
} dpci_idlp_event_t;

/*
 * Filled in by dpci_drv_hw_profile()
 */
struct dpci_hardware_profile
{
	unsigned short	dpci_vendor_id;
	unsigned short	dpci_device_id;
	unsigned short	dpci_rev;
	unsigned short	rb_vendor_id;
	unsigned short	rb_device_id;
	unsigned int	board_code;
};

/*
 * Used for all dpci wait-event command.
 */

#define EVENT_NONE_ANY	0
#define EVENT_DIG_IP	1
#define EVENT_IDLP		2
#define EVENT_PFD		4
#define EVENT_TS		8

typedef struct dio_port_event
{
	unsigned char change_mask;      // Digital i/p port event
	unsigned char dio_value;
} dpci_io_portevent_t;

typedef struct dio_event
{
	unsigned long change_mask;      // Digital i/p event
	unsigned long dio_value;
} dpci_io_event_t;

#ifdef __linux
typedef unsigned long long dpci_timestamp_t;
#elif defined(WIN32)
typedef unsigned __int64 dpci_timestamp_t;
#endif
typedef struct dpci_event 
{
	unsigned int		de_type;
	union
	{
		dpci_io_event_t		dio_event;
		dpci_idlp_event_t	idlp_event;        // IDLP event
		//unsigned int		no_data;
	}			de_data;
	dpci_timestamp_t	de_ts;		// Timestamp for this event
	unsigned char		de_ts_str[32];	//unused
	dpci_timestamp_t	de_ts_delta;	// difference in time since the 
					// previous event; (or) for the first
					// event, time since the start of 
					// event_wait
} dpci_event_t;
#define	idlp_event(evp)	(evp)->de_data.idlp_event
#define	dio_event(evp)	(evp)->de_data.dio_event

struct event_mask_t {
	unsigned int er_type;
	unsigned long int_mask;
	unsigned long edge_state;
	unsigned long autoconfig_edge;
};

/*
 * Used for user-initiated I2C transactions
 */
#define MAX_I2C_COMMAND_SEGS        8
struct dpci_i2c_cmdbuf
{
	int		bus;		/* bus to use */
	int		speed_hz;	/* speed to use - see docs. */
	int		options;	/* options for the transactions */
	int		attempts;	/* times to try */
	int		result;		/* result of operation */
	unsigned char	segment_lengths[MAX_I2C_COMMAND_SEGS]; /* sizes */
	unsigned char	buffer[1];	/* buffer of all data */
};

#define	DEFAULT_I2C_ATTEMPTS	3
#define	I2C_OPTIONS_NORESTART	1	/* Don't restart - always stop-start */
#define	I2C_OPTIONS_UNLOCK		2	/* attempt bus unlock */
#define	I2C_OPTIONS_FORCEACK	4	/* force ACK even on last byte */

/*
 * These options determine how the word address for the device's memorry is
 * encoded when addressing the slave device.  In some devices with 9, 10, 11
 * or 17, 18, 19 address bits for the internal memory address, the extra bits
 * are moved into the lower bits of the I2C slave address.
 *
 * Note that for such devices we offer two options only:
 *
 * - 11BITADDR which addresses slaves with 9, 10 or 11 bit memory addresses
 * - 19BITADDR which addresses slaves with 17, 18 or 19 bit memory addresses
 *
 * These options are only recognised by the dpci_i2c_read and dpci_i2c_write
 * API entries.
 *
 * Note that the extra bits of memory address are extracted and then logic-OR'd
 * into the slave address.
 */
#define I2C_OPTIONS_ADDRMASK	0x18
#define I2C_OPTIONS_1BYTEADDR	0x00	/* Address is one byte (rwbuf only) */
#define I2C_OPTIONS_11BITADDR	0x08	/* Address is 9-11 bits (rwbuf only) */
#define I2C_OPTIONS_2BYTEADDR	0x10	/* Address is two bytes (rwbuf only) */
#define I2C_OPTIONS_19BITADDR	0x18	/* Address is 17-19 bits (rwbuf only) */


#define I2C_RESULT_INPROGRESS	-1	/* still running - should never see */
#define I2C_RESULT_SUCCESS		0	/* everything went fine. */
#define I2C_RESULT_NOSLAVE		1	/* Slave didn't ACK in address byte */
#define I2C_RESULT_TXNAK		2	/* Slave sent NAK after data byte */
#define I2C_RESULT_LOSTARB		3	/* Lost arbitration to other master */
#define I2C_RESULT_TIMEDOUT		4	/* Transaction took too long */
#define I2C_RESULT_INTERNALERR	5	/* Internal error */
#define I2C_RESULT_STUCKBUS		6	/* Bus is stuck */
#define I2C_RESULT_BUSBUSY		7	/* Bus is busy - couldn't get in */
#define I2C_RESULT_NOBUS		8	/* Numbered bus does not exist */
#define I2C_RESULT_INVALIDBUF	9	/* Request is somehow invalid */

/*
 * Sleep times for Intrusion Detection processor on DPX116 and newer boards.
 */
#define	INTRUS_SLEEP_1SEC		0x31
#define	INTRUS_SLEEP_500MSEC	0x32
#define	INTRUS_SLEEP_250MSEC	0x33
#define	INTRUS_SLEEP_125MSEC	0x34

/*
 * Defines for returns from dpci_idlp_readevent_psw(), dpci_id_setdate_psw()
 * and dpci_id_wait_event_psw().
 */
#ifdef __linux
# define ERROR_PSW_REQ		2	/* Password required for operation */
# define ERROR_PSW_INVALID	3 	/* Password entered is incorrect */
# define ERROR_PSW_SHA1		4 	/* error during SHA1 hash of password */
#endif

/******************************************************************************
 *
 * Serial number type for the DS28CM00 serial chip
 *
 */
typedef union
{
	unsigned char chars[8];
	struct
	{
		unsigned char family;
		unsigned char serial[6];
		unsigned char check;
	} data;
#ifdef WIN32
	__int64 number;
#else
	unsigned long long number;
#endif /* WIN32 */

} serial_t;

/*
 * DirectPCI API library debug levels.
 *
 * Max 32 levels - one bit each.
 *
 * For debug (checked) versions of libdpci, the variable DPCI_DEBUG_LEVEL will
 * be read from the environment.  If not present the default level is 1.
 */
#define DPCI_DEBUG_SEVERITY_MASK			0x00000003
#define DPCI_DEBUG_SEVERITY_TRACE			0x00000000
#define DPCI_DEBUG_SEVERITY_INFO			0x00000001
#define DPCI_DEBUG_SEVERITY_WARNING			0x00000002
#define DPCI_DEBUG_SEVERITY_ERROR			0x00000003
#define DPCI_DEBUG_API_MASK				0x0ffffffc
#define DPCI_DEBUG_CORE_INIT_HANDLE			0x00000004
#define	DPCI_DEBUG_IDLP_API				0x00000008
#define	DPCI_DEBUG_TS_API				0x00000010
#define	DPCI_DEBUG_I2C_API				0x00000020
#define	DPCI_DEBUG_EEPROM_API				0x00000040
#define DPCI_DEBUG_EVENT_API				0x00000080
#define DPCI_DEBUG_EVENT_DEBOUNCE			0x00000100
#define DPCI_DEBUG_BIOS_API				0x00000200
#define DPCI_DEBUG_BAT_API				0x00000400
#define	DPCI_DEBUG_IDPROMIDENT_API			0x00000800
#define DPCI_DEBUG_DIGINOUT_API				0x00001000
#define	DPCI_DEBUG_QMGPIO_API				0x00002000
#define	DPCI_DEBUG_MISC_API				0x00004000
#define	DPCI_DEBUG_BOARDS_API				0x00008000
#define	DPCI_DEBUG_WD_API				0x00010000
#define DPCI_DEBUG_SRAM_INIT_HANDLE			0x04000000
#define DPCI_DEBUG_SRAM_API				0x08000000
#define DPCI_DEBUG_SRAM_MMAP				0x10000000
#define DPCI_DEBUG_ROM_INIT_HANDLE			0x20000000
#define DPCI_DEBUG_ROM_API				0x40000000
#define DPCI_DEBUG_ROM_MMAP				0x80000000

#ifdef __cplusplus
}
#endif /*end __cplusplus*/
#endif /*end _DPCIAPITYPES_H*/

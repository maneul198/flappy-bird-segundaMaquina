/******************************************************************************
 *
 * $Id: dpci_core_api.h 12286 2016-01-13 09:37:16Z amar $
 *
 * Copyright 2003-2015 Advantech Co. Limited.
 * All rights reserved.
 *
 * Description:
 * DPCI API header file for core access.
 *
 *****************************************************************************/

#ifndef _DPCI_CORE_API_H_
#define _DPCI_CORE_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dpci_api_decl.h"
#include <dpci_api_types.h>
#include "dpci_bios_api.h"
#include "dpci_error.h"

/****************************************************************************
 *
 * IDPROM (DPX-S and DPX-C series)
 */
#define	DPCI_IDPROM_SIZE	0x80
#define	DPCI_IDPROM_ID_SIZE	0x8

/*
 * Lock types for IDPROM pages
 */
enum
{
	LOCK_NONE = 0,
	LOCK_WRITEPROTECT = 0x55,
	LOCK_EEPROM = 0xaa
};

DPCI_API int APIENTRY dpci_idprom_size(void);
DPCI_API int APIENTRY dpci_idprom_readid(unsigned char *buf);
DPCI_API int APIENTRY dpci_idprom_read8(unsigned int offs, 
										unsigned char *byte);
DPCI_API int APIENTRY dpci_idprom_read(unsigned int offs, 
					   unsigned char *data, 
					   unsigned int len);

DPCI_API int APIENTRY dpci_idprom_page_status(unsigned int page);
DPCI_API int APIENTRY dpci_idprom_lock_page(unsigned int page, int lock_type);


DPCI_API int APIENTRY dpci_idprom_get_copyprotect(void);
DPCI_API int APIENTRY dpci_idprom_set_copyprotect(void);
DPCI_API int APIENTRY dpci_idprom_write_mfid(unsigned short mfid);
DPCI_API int APIENTRY dpci_idprom_read_mfid(void);

DPCI_API int APIENTRY dpci_idprom_write8(unsigned int offs, unsigned char data);
DPCI_API int APIENTRY dpci_idprom_write(unsigned int offs, 
					   unsigned char *data, 
					   unsigned int len);

/*
 * The macros to determine the DPCI version.
 */
#define	DPCI_VERSION_MAJOR(ver)		(((ver) & 0xf0000000) >> 28)
#define	DPCI_VERSION_MINOR(ver)		(((ver) & 0x0ff00000) >> 20)
#define	DPCI_VERSION_MICRO(ver)		(((ver) & 0x000ff000) >> 12)
#define	DPCI_VERSION_BUILD(ver)		(((ver) & 0x00000fff))


/****************************************************************************
 *
 * API version detection.
 */
DPCI_API int		APIENTRY dpci_api_version(void);
DPCI_API const char *	APIENTRY dpci_api_version_string(void);
DPCI_API int		APIENTRY dpci_drv_version(void);
DPCI_API const char *	APIENTRY dpci_drv_version_string(void);
DPCI_API int		APIENTRY dpci_drv_hw_version(unsigned int *device_idp,
                                                     unsigned int *revp);
DPCI_API int		APIENTRY dpci_drv_hw_profile(struct dpci_hardware_profile *dhwp);

/*
* Ensure that DPCI is installed correctly
*/
DPCI_API int APIENTRY dpci_core_open(void);

/****************************************************************************
 *
 * DPCI Events and Callbacks
 */
DPCI_API int APIENTRY dpci_ev_wait_all(struct dpci_event *pEvent,
                                       unsigned long timeout);
DPCI_API int APIENTRY dpci_ev_wait(struct dpci_event *pEvent,
				   struct event_mask_t event_mask,
                                   unsigned long timeout);
typedef void (*dpci_ev_callback_t)(struct dpci_event *evp, void *data);
DPCI_API int APIENTRY dpci_ev_register_callback(dpci_ev_callback_t func,
						void *data,
						struct dpci_event *evp);
DPCI_API int APIENTRY dpci_ev_unregister_callback(dpci_ev_callback_t func,
						  void *data,
						  struct dpci_event *evp);
DPCI_API int APIENTRY dpci_ev_set_debounce(unsigned long mask, int ms);
DPCI_API dpci_timestamp_t APIENTRY dpci_ev_get_timestamp(void);
DPCI_API int APIENTRY dpci_ev_close_stream(void* thread_id);


/****************************************************************************
 *
 * Functions to get and set debug level bitmask
 *
 * For driver debug level macros, please refer to the programmers' reference
 * manual.
 *
 * For API debug level macros, refer to dpci_api_types.h.
 */
DPCI_API int		APIENTRY dpci_core_set_debug_level(unsigned int level);
DPCI_API unsigned int	APIENTRY dpci_core_get_debug_level(void);
DPCI_API int		APIENTRY dpci_api_set_debug_level(unsigned int level);
DPCI_API int		APIENTRY dpci_api_modify_debug_level(unsigned int set,
								unsigned int clear,
								unsigned int toggle);
DPCI_API unsigned int	APIENTRY dpci_api_get_debug_level(void);
typedef int (*dpci_debug_callback_t)(unsigned int flags, const char *message);
#define DPCI_DEBUG_CALLBACK_RET_NOLOG	0		/* don't log message to file (if set) */
#define DPCI_DEBUG_CALLBACK_RET_LOG	1		/* log message to file (only if set) */
DPCI_API void		APIENTRY dpci_api_set_debug_message_callback(dpci_debug_callback_t func);
DPCI_API int		APIENTRY dpci_api_set_debug_output_file(const char *path);
#ifdef WIN32
DPCI_API void		APIENTRY dpci_api_set_debug_output_handle(HANDLE handle);
#elif __linux
DPCI_API void		APIENTRY dpci_api_set_debug_output_handle(int fd);
#endif


/****************************************************************************
 *
 * Watchdog manipulation functions.  These pertain to the main system watch-dog
 * implemented in the IDLP.
 */
DPCI_API int APIENTRY dpci_wd_enable(int interval);
DPCI_API int APIENTRY dpci_wd_disable(void);
DPCI_API int APIENTRY dpci_wd_reset(void);


/****************************************************************************
 *
 * The functions to read logging processor events
 */
DPCI_API int	APIENTRY dpci_id_numevents(void);
DPCI_API int   APIENTRY dpci_id_readchecksum(void);
DPCI_API int	APIENTRY dpci_id_readevent(struct idevent *inevent);
DPCI_API int	APIENTRY dpci_id_readevent_instance(int which,
						struct idevent *inevent);
DPCI_API int	APIENTRY dpci_id_readevent_psw(struct idevent *inevent,
						   char *psw);
DPCI_API const char *	APIENTRY dpci_id_event_name(int code);
DPCI_API int	APIENTRY dpci_id_wait_event(struct idevent *pEvent, 
						unsigned long timeout_ms);
DPCI_API int	APIENTRY dpci_id_wait_event_psw(struct idevent *pEvent,
						unsigned long timeout_ms,
						char *psw);
DPCI_API int	APIENTRY dpci_id_setdate(struct idevent *inevent);
DPCI_API int	APIENTRY dpci_id_setdate_psw(struct idevent *inevent,
						 char *psw);
DPCI_API int	APIENTRY dpci_id_getdate(struct idevent *inevent);

DPCI_API int	APIENTRY dpci_id_fwversion(void);
DPCI_API int	APIENTRY dpci_id_fwversion_full(void);
DPCI_API int	APIENTRY dpci_id_setsleeptime(int sleepcode);
DPCI_API int	APIENTRY dpci_id_intrusionstatus(void);

DPCI_API int	APIENTRY dpci_id_select_event(int which, 
						 struct idevent *event);

#ifdef WIN32
/*
 * These deprecated calls are no longer available as of v3.1.
 * The following functions are stubs that exist only to return an error code.
 */
DPCI_DEPRECATED_API int APIENTRY dpci_id_request_pending_event(struct idevent *pEvent);
DPCI_DEPRECATED_API int APIENTRY dpci_id_cancel_pending_event (struct idevent *pEvent);
#endif

/****************************************************************************
 *
 * Battery API
 */
DPCI_API int APIENTRY dpci_bat_num_batteries(void);
DPCI_API int APIENTRY dpci_bat_name(int bat, char *buf, 
					 unsigned int bufsiz);
DPCI_API int APIENTRY dpci_bat_number(char *name);
DPCI_API int APIENTRY dpci_bat_get_status_mask(void);
DPCI_API int APIENTRY dpci_bat_get_level(int batt);
DPCI_API int APIENTRY dpci_bat_get_status(int batt);
DPCI_API int APIENTRY dpci_bat_set_check_period(int code);
DPCI_API int APIENTRY dpci_bat_set_error_level(int batt, int millivolts);
DPCI_API int APIENTRY dpci_bat_get_errorlevel(int batt);

/* For Backward Compatibility */
DPCI_DEPRECATED_API int APIENTRY dpci_bat_read	(void);

/****************************************************************************
 *
 * The function to read the DIP switches status.
 */
DPCI_API int APIENTRY dpci_dis_read(void);


/****************************************************************************
 *
 * Functions for quiet mode status get and set.
 */
DPCI_API int APIENTRY dpci_qm_get(int*, int*);
DPCI_API int APIENTRY dpci_qm_set(int, int);


/****************************************************************************
 *
 * The functions allow access to the digital I/O functions.
 */
DPCI_API int APIENTRY dpci_io_board_supported(void);
DPCI_API int APIENTRY dpci_io_getboard_id(void);
DPCI_API int APIENTRY dpci_io_getboard_rev(void);

DPCI_API int APIENTRY dpci_io_wait_port(int iPort,
					unsigned char byMask, 
					unsigned char byEdgeState, 
					unsigned long timeout_ms);

DPCI_API int APIENTRY dpci_io_wait_int(unsigned char int_mask,
					   unsigned char edge_state,
					   unsigned long timeout_ms);

DPCI_API int APIENTRY dpci_io_wait_iport(int iPort,
					 unsigned char int_mask, 
					 unsigned char edge_state, 
					 unsigned char autoconfig_edge, 
					 unsigned long timeout_ms,
					 struct dio_port_event *dio_data);

DPCI_API int APIENTRY dpci_io_wait_iports(unsigned long int_mask, 
					  unsigned long edge_state, 
					  unsigned long autoconfig_edge, 
					  unsigned long timeout_ms,
					  struct dio_event *dio_data);

DPCI_API int APIENTRY dpci_io_wait_pfd(unsigned long timeout_ms);
DPCI_API int APIENTRY dpci_io_read_port(int port, unsigned char *value);
DPCI_API int APIENTRY dpci_io_write_port(int port, unsigned char value);

DPCI_API int APIENTRY dpci_io_change_port(int iPort, 
					  unsigned char set, 
					  unsigned char clear, 
					  unsigned char toggle);

DPCI_API int APIENTRY dpci_io_read_outport(int port, unsigned char *value);

DPCI_API int APIENTRY dpci_io_numiports(int irqports);
DPCI_API int APIENTRY dpci_io_numoports(void);

#ifdef __linux
int dpci_io_config_input(int port,
			 unsigned char int_mask,
			 unsigned char pol_mask);
#endif


/****************************************************************************
 *
 * The functions allow direct access to the DirectPCI hardware register space.
 * We specifically recommend these not be used except in extreme circumstances
 */
DPCI_API int APIENTRY dpci_io_read8(dpci_off_t offset, unsigned char *value);
DPCI_API int APIENTRY dpci_io_read16(dpci_off_t offset, unsigned short *value);
DPCI_API int APIENTRY dpci_io_read32(dpci_off_t offset, unsigned int *value);
DPCI_API int APIENTRY dpci_io_write8(dpci_off_t offset, unsigned char value);
DPCI_API int APIENTRY dpci_io_write16(dpci_off_t offset, unsigned short value);
DPCI_API int APIENTRY dpci_io_write32(dpci_off_t offset, unsigned int value);


/****************************************************************************
 *
 * GPIO Port access.
 *
 * These functions pertain to the two GPIO ports available as of the DPX-112
 * and which double up for OneWire+iButton communications.
 */
DPCI_API int APIENTRY dpci_gpio_read_ip(int gpio_line, unsigned char *value);
DPCI_API int APIENTRY dpci_gpio_write_op(int gpio_line, unsigned char value);
DPCI_API int APIENTRY dpci_gpio_read_op(int gpio_line, unsigned char *value);


/****************************************************************************
 *
 * I/O Watchdog access.
 *
 * This is mainly for use with the 80-0062 and 80-1003 I/O boards where the
 * I/O watchdog is separate from the system watchdog implemented in the IDLP.
 */
DPCI_API int APIENTRY dpci_iowd_enable(void);
DPCI_API int APIENTRY dpci_iowd_disable(void);
DPCI_API int APIENTRY dpci_iowd_pat(void);


/****************************************************************************
 *
 * EEPROM access.
 *
 * These functions are available to access the EEPROM on the DPX-series
 * mainboards.  Only the DPX-114 and DPX-115 do not have an EEPROM.
 *
 * To access other eeproms use the dpci_i2c_read/write() functions.
 */
DPCI_API int APIENTRY dpci_e2_read8(unsigned int offset, 
					unsigned char *pvalue);
DPCI_API int APIENTRY dpci_e2_read(unsigned int offset, 
				   unsigned char *data, 
				   unsigned int len);
DPCI_API int APIENTRY dpci_e2_write8(unsigned int offset, 
					 unsigned char value);
DPCI_API int APIENTRY dpci_e2_write(unsigned int offset, 
					unsigned char *data, 
					unsigned int len);
DPCI_API int APIENTRY dpci_e2_size(void);

/****************************************************************************
 *
 * I2C Serial number / identity
 *
 * This function attempts to read a serial number from a DS28CM00 type
 * device.
 *
 */
DPCI_API int APIENTRY dpci_core_get_serial(serial_t *buffer);


/****************************************************************************
 *
 * The I2C read/write buffer 
 *
 * Struct dpci_i2c_rwbuf:
 *
 * bus:		the logical I2c bus number as support by the driver.
 * slave:	the physical slave number as determined by the hardware arch.
 * speed_hz:	the bus speed in Hz.  If 0 default of 1kHz is used.
 * options: 	any options pertinent to the operation.
 * attempts:	number of times to attempt operations.
 * result:	the result of the last i2c operation.
 * pagesize:	the size of the slave's pages (if supported).  0 means 1 byte.
 * offset:	offset in slave's space to access.
 * count:	number of bytes to transfer.
 * buf:		where to get the data.
 */
struct dpci_i2c_rwbuf
{
	int	bus;		/* bus to use */
	int	slave;		/* slave to contact */
	int	speed_hz;	/* speed to use - see docs. */
	int	options;	/* options for the transactions */
	int	attempts;	/* times to try */
	int	result;		/* result of last i2c operation */
	int	pagesize;	/* size of pages for multi-byte operations*/
	int	offset;		/* where in the slave's space to access */
	int	count;		/* how much data */
	unsigned char *buf;	/* buffer */
};

/*
 * For DirectPCI boards with an I2C interface, these functions send 
 * one or more I2C packets out on the bus.  
 * See dpci_api_types.h for more information about the structures.
 */
DPCI_API int APIENTRY dpci_i2c_numbuses(void);
DPCI_API int APIENTRY dpci_i2c_bus_name(int busno, 
										char *buf, 
										unsigned int bufsiz);
DPCI_API int APIENTRY dpci_i2c_bus_number(char *name);
DPCI_API int APIENTRY dpci_i2c_command(struct dpci_i2c_cmdbuf *pI2cCmdB);
DPCI_API int APIENTRY dpci_i2c_read(struct dpci_i2c_rwbuf *pI2cRWB);
DPCI_API int APIENTRY dpci_i2c_write(struct dpci_i2c_rwbuf *pI2cRWB);
DPCI_API int APIENTRY dpci_i2c_read8(int bus, 
					 int slave, 
					 unsigned int offset, 
					 unsigned char *pvalue);
DPCI_API int APIENTRY dpci_i2c_write8(int bus, 
					  int slave, 
					  unsigned int offset, 
						  unsigned char value);
DPCI_API const char * APIENTRY dpci_i2c_error_string(int code);
DPCI_API int  APIENTRY dpci_i2c_last_error(void);
DPCI_API const char * APIENTRY dpci_i2c_last_error_string(void);


/****************************************************************************
 *
 * I/O Board 2 temperature sensor support.
 */
DPCI_API int APIENTRY dpci_ts_init(void);
DPCI_API int APIENTRY dpci_ts_getsensor(int sensor, 
										int *temp, 
										int *connected);
DPCI_API int APIENTRY dpci_ts_configure(int sensor, 
					int high, 
					int low, 
					int crit, 
					int offs, 
					int alarm);
DPCI_API int APIENTRY dpci_ts_getconfig(int sensor, 
					int *high, 
					int *low, 
					int *crit, 
					int *offs, 
					int *alarm);
DPCI_API int APIENTRY dpci_ts_check_state(unsigned int sensor_mask, ...);
DPCI_API int APIENTRY dpci_ts_wait_alarm(unsigned long timeout_ms);
DPCI_API int APIENTRY dpci_ts_numsensors(void);
DPCI_API int APIENTRY dpci_ts_sensor_name(int sensor, 
					  char *buf, 
					  unsigned int bufsiz);
DPCI_API int APIENTRY dpci_ts_sensor_number(char *name);
DPCI_API const char* APIENTRY dpci_ts_state_name(int state);
DPCI_API int APIENTRY dpci_ts_state_number(char *name);

#ifdef __linux
int dpci_ts_signal_alarm(int signal);
#endif

#define	IOBD2_SENSOR_ONBOARD		0x00
#define	IOBD2_SENSOR_REMOTE			0x01
#define	IOBD2_SENSOR_MASK_ONBOARD	(1 << IOBD2_SENSOR_ONBOARD)
#define	IOBD2_SENSOR_MASK_REMOTE	(1 << IOBD2_SENSOR_REMOTE)
#define	IOBD2_SENSOR_MASK_ALL		(IOBD2_SENSOR_MASK_ONBOARD | \
						 IOBD2_SENSOR_MASK_REMOTE)

#define TS_ALARM_NONE	0x00		/* Do not report any alarms */
#define TS_ALARM_CRIT	0x01		/* Report for critical levels */
#define TS_ALARM_HIGHLO	0x02		/* Report for high and low levels */

#define	TS_STATE_INVALID -1		/* temperature state is invalid */
#define	TS_STATE_NORMAL	0x00		/* temperature is in normal levels */
#define	TS_STATE_LOW	0x01		/* temperature is too low */
#define	TS_STATE_HIGH	0x02		/* temperature is too high */
#define	TS_STATE_CRIT	0x03		/* temperature is critical */


/****************************************************************************
 *
 * DPX-E130 back-up power supply (super-caps) hold up system for PFDs.
 */
DPCI_API int APIENTRY dpci_e130_bps_enable(int enable);
DPCI_API int APIENTRY dpci_e130_bps_is_enabled(void);
DPCI_API int APIENTRY dpci_e130_bps_is_charged(void);

/*
 * To wait for the BPS to charge, wait for a 0->1 transition on DI15 (port 1,
 * bit 7).
 */
#define DPX_E130_BPS_CHARGE_STATUS_DI	15
#define DPX_E130_BPS_CHARGE_STATUS_PORT ((DPX_E130_BPS_CHARGE_STATUS_DI) / 8)
#define DPX_E130_BPS_CHARGE_STATUS_MASK (1 << ((DPX_E130_BPS_CHARGE_STATUS_DI) & 7))

#ifdef __cplusplus
}
#endif

#endif /* _DPCI_CORE_API_H */

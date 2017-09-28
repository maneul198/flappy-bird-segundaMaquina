/******************************************************************************
 *
 * $Id: dpci_core_priv2.h 11939 2015-09-14 22:31:13Z aidan $
 *
 * Copyright 2011-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * Advantech-Innocore DirectPCI common defines for core driver
 *
 *****************************************************************************/
#ifndef _DPCICOREPRIV2_H
#define _DPCICOREPRIV2_H

#ifdef __cplusplus
extern "C" {
#endif


/*
 * The speed and level numbers are identical in all cases below.
 */
#define DOW_CONFIG_SPEED_MASK   0x01
#define DOW_CONFIG_LEVEL_MASK   0x06

#define DOWP_FLAGS_HAVE_ODRIVE  0x01
#define DOWP_FLAGS_HAVE_POWER   0x02
#define DOWP_FLAGS_HAVE_PROGRAM 0x04

#define DOWP_SPEED_NORMAL       0x00
#define DOWP_SPEED_OVERDRIVE    0x01

#define DOWP_POWER_NORMAL       0x00
#define DOWP_POWER_STRONG5      0x02
#define DOWP_POWER_PROGRAM      0x04

/******************************************************************************
 *
 * I2C State machine stuff.
 */
typedef enum
{
        I2C_INACTIVE = 0,       // inactive - nothing doing
        I2C_START_SDALO,        // SDA going low as 1st stage of start condition
        I2C_START_SCLLO,        // SCL going low as 2nd stage of start condition
        I2C_START_CLKWAIT,      // SCL low period after start condition given.
        I2C_DATA_TX,            // Transmission of data with SCL going high
        I2C_DATA_RX,            // Reception of data with SCL going high
        I2C_DATA_CLKWAIT,       // Wait for clock high (in case slave stretches)
        I2C_DATA_CLKLO,         // SCL low period after bit data exchange
        I2C_ACK_TX,             // Transmission of ACK (for Rx) with SCL going high
        I2C_ACK_RX,             // Reception of ACK (for Tx) with SCL going high
        I2C_ACK_CLKWAIT,        // Wait for clock high (in case slave stretches)
        I2C_ACK_CLKLO,          // SCL low period after ack exchange
        I2C_STOP_SCLLO,         // SCL going low as 1st stage of stop condition
        I2C_STOP_SCLHI,         // SCL going high as 2nd stage of stop condition
        I2C_STOP_SDAHI,         // SDA going high as 3rd stage of stop condition
        I2C_COMPLETED           // Transaction is completed.
} dpci_i2c_state_t;


/*
 * These macros define the features supported by the core hardware.
 */
#define HAVE__UNUSED4           0x00000001 /* was Separate INTEN for PIC */
#define HAVE_TWOBITI2C          0x00000002 /* Bit-level i2c interface */
#define HAVE__UNUSED3           0x00000004 /* was Byte-level i2c interface */
#define HAVE_NODIPSW            0x00000008 /* DIP not switches present */
#define HAVE__UNUSED            0x00000010 /* WAS IOWDT */
#define HAVE_DENSITRON_IOEXP    0x00000020 /* Can have separate IO board */
#define HAVE_CUSTOMER_IOEXP     0x00000040 /* Can have customer IO board */
#define HAVE_IOSHAREWDT         0x00000080 /* IO & watchdog share wdt */
#define HAVE_EXINTEN		0x00000100 /* separate INTEN for external IO */
#define HAVE_IDLPPLUS		0x00000200 /* New IDLP for C605/705,S410/305 */
#define HAVE_EXINT0		0x00000400 /* EXINT bit is bit0*/
#define HAVE_IOINTONEXINT	0x00000800 /* EXINT is really digital IOINT */
#define HAVE_NEWDIGIOREGS	0x00001000 /* New interrupt registers for I/O */
#define HAVE_POWERFAILDETECT	0x00002000 /* Power Fail Detect on S/C-series */
#define HAVE_QUIETMODE		0x00004000 /* Quiet Mode on S and C-series */
#define HAVE_CLRWDT		0x00008000 /* Watchdog clear mechanism */
#define HAVE_MBDISINREG		0x00010000 /* IDLP fail disables registers */
#define HAVE_NOGPIO		0x00020000 /* GPIO line access */
#define	HAVE_IDLP_QUICK		0x00040000 /* faster FPGA-based IDLP comms */
#define HAVE_QMODE_PASSTHRU	0x00080000 /* Quiet mode control on external input available */
#define HAVE_1ROMSOCKET		0x00100000 /* Board only has 1 PLCC OTPROM socket */


/******************************************************************************
 *
 * OW (one-wire) Bus stuff
 */

struct dpci_ow_port
{
	char			*dowp_desc;
	int			(*dowp_init)(struct dpci_ow_port *dowp);

	unsigned char		(*dowp_touchbit)(struct dpci_ow_port *, unsigned char);
	unsigned char		(*dowp_resetpulse)(struct dpci_ow_port *);
	unsigned char		(*dowp_programpulse)(struct dpci_ow_port *);
	int		(*dowp_setlevel)(struct dpci_ow_port *, int);
	int		(*dowp_setspeed)(struct dpci_ow_port *, int);

	int			dowp_flags;
	dpci_off_t		dowp_reg;
	unsigned char		dowp_rbit;
	unsigned char		dowp_wbit;
	unsigned char		dowp_pbit;
#ifdef __linux
	struct semaphore	dowp_sem;
	pid_t			dowp_acquired;
#else
	FAST_MUTEX		dowp_mutex;
	PEPROCESS		dowp_acquired;
#endif
	struct dpci_device	*dowp_ddp;
#ifdef __linux
#if !defined(USE_DGL_W1_API) && defined(CONFIG_W1)
	struct w1_bus_master	dowp_master;
#endif
#endif
	void			*dowp_private;
};

/******************************************************************************
 *
 * Internal I2C stuff.
 */
struct dpci_i2c_port
{
	char	*dip_desc;
	void	(*dip_init)(struct dpci_i2c_port *i2c_port_p);
	int	(*dip_command)(struct dpci_device *ddp,
			struct dpci_i2c_cmdbuf *i2c_buf_p);
	dpci_off_t	 		dip_reg;
	unsigned char		dip_sda_w;	// normal write bit
	unsigned char		dip_scl_w;	// normal write bit
	unsigned char		dip_sda_r;	// normal read bit
	unsigned char		dip_scl_r;	// normal read bit
	unsigned char		dip_sda_iw;	// inverting write mask
	unsigned char		dip_scl_iw;	// inverting write mask
	unsigned char		dip_sda_ir;	// inverting read mask
	unsigned char		dip_scl_ir;	// inverting read mask
#ifdef __linux
	struct semaphore	dip_sem;
#else
	FAST_MUTEX		dip_mutex;
#endif
	struct dpci_device	*dip_ddp;
};


struct dpci_i2c_context
{
	struct dpci_i2c_port	*port;
#ifndef __linux
	KTIMER			timer;
#endif
	dpci_i2c_state_t	i2c_state;
	struct dpci_i2c_cmdbuf	*i2c_cmdbuf_p;
#ifdef __linux
	int			usec_delay;
#endif
	int			current_segment;
	int			current_segment_length;
	int			current_byte_no;
	unsigned char		*current_byte_p;
	unsigned char		current_bit_mask;
	unsigned char		current_address;
	int			clocks;
	int			total_clocks;
};


/******************************************************************************
 *
 * This stuff is all to do with detecting the type of hardware cleanly.
 *
 * name:		just for display reasons
 * tag:			the ID (PCI derive or I/O board type) for this board.
 * min_rev:		the minimum revision of DPCI f/w supported.
 * base_features:	the features this board supports.
 * ip_data_regs:	input port actual data registers
 * ip_stat_regs:	input port interrupt detection registers
 * ip_pol_regs:		input port edge configuration registers
 * ip_inte_regs:	input port interrupt enable registers.
 * op_regs:		output port actual data registers.
 * uart_regs:		addresses of uarts for this board.
 * spare_start:		start of customer defined range
 * spare_end:		end (last reg + 1) of customer defined range
 * i2c_ports:		details of the I2C ports
 * ow_ports:		details of the OneWire ports
 * batteries:		details of the batteries
 *
 * When feature HAVE_NEWDIGIOREGS is set, then db_ip_pol_regs and the following
 * db_ip_inte_regs are overloaded to be the new db_ip_dic_regs.
 */

#ifdef __linux
typedef irqreturn_t (*IRQ_HANDLER)(struct dpci_device *, unsigned char reg0);
#else
typedef int (*IRQ_HANDLER)(struct dpci_device *);
#endif

struct dpci_board
{
	const char 	*db_name;
	int		db_board_id;		/* DPX-board ID */
	unsigned char	db_num_bat;			/* No of IDLP monitored batteries actually supported on this board */
	unsigned long	bios_len;			/* BIOS size */

	unsigned char	db_min_rev;
	int		db_base_features;
	dpci_off_t	db_sys_start;
	dpci_off_t	db_sys_size;
	dpci_off_t	db_spare_start;
	dpci_off_t	db_spare_size;
	dpci_off_t	db_ip_data_regs[MAX_INPUT_PORTS];
	dpci_off_t	db_ip_stat_regs[MAX_INPUT_PORTS];
	dpci_off_t	db_ip_pol_regs[MAX_INPUT_PORTS];
	dpci_off_t	db_ip_inte_regs[MAX_INPUT_PORTS];
#define db_ip_dicevn_regs db_ip_pol_regs
#define db_ip_dicodd_regs db_ip_inte_regs
	dpci_off_t	db_op_regs[MAX_OUTPUT_PORTS];
#ifdef __linux
	dpci_off_t	db_uart_regs[MAX_UARTS];
#else
	int		db_has_serial;
#endif
	struct dpci_i2c_port
			db_i2c_ports[MAX_I2C_PORTS];
	struct dpci_ow_port
			db_ow_ports[MAX_OW_PORTS];
	struct dpci_battery
			db_batteries[MAX_BATTERIES];
        IRQ_HANDLER db_irq_handler;
};

/*
 * Performance counters
 */
struct dpci_counter
{
	unsigned int	*dc_counter;
	const char	*dc_desc;
};


/*
 * Event Queuing related stuff
 */
#define MAX_EVENT_QUEUE_SIZE	32
#define WAIT_EVENT_THREAD_TIMEOUT_MS	100
#define MAX_OUTPUT_QUEUE_SIZE	256
typedef enum 
{
	NEW = 0,
	ENABLED,
	SIGNALLED,
	DISABLED,
} ER_STATE_TYPE;

typedef enum
{
	SIGNAL_NONE	= 0,	// nothing happened
	SIGNAL_EVENT,		// an event was detected
	SIGNAL_PARAMS,		// need to re-collate event request queue.
	SIGNAL_KILL
} ER_SIGNAL_TYPE;

struct digip_param
{
	unsigned long int_mask;
	unsigned long edge_state;
	unsigned long autoconfig_edge;
};
/* 
 * This stucture defines a list if output events
 */
struct op_event_list
{
	struct dpci_event buf[MAX_OUTPUT_QUEUE_SIZE];
	int buf_head;
	int buf_tail;
};

/*
 * This structure is used to hold flags to signal occurrence of an interrupt
 * If a flag is set, interrupts of that type are disabled until the event is
 * copied to all event queues of user threads waiting on that interrupt
 */
struct event_flags
{
	unsigned char idlp_flag;
	unsigned char pfd_flag;
	unsigned long digip_flags;
	unsigned char ts_flag;
};


#ifdef __cplusplus
}
#endif /*end __cplusplus*/
#endif /*end _DPCICOREPRIV2_H*/


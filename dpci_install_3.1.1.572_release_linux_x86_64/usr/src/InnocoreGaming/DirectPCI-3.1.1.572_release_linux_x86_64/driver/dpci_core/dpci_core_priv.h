/******************************************************************************
 *
 * $Id: dpci_core_priv.h 11955 2015-09-22 22:07:03Z aidan $
 *
 * Copyright 2006-2014 Advantech Corporation Limited.
 * All rights reserved.
 *
 * License:	GPLv2
 *
 * Description:
 * Internal macros for the DirectPCI Core I/O kernel driver.
 *
 *****************************************************************************/
#ifndef _DPCI_CORE_PRIV_H
#define _DPCI_CORE_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif
#include "dpci_core_types.h"
#include "linux/dpci_multi.h"
#include "dpci_core_priv2.h"

/******************************************************************************/
/*
 * The macro defines the DPCICORE module name
 */
#define DPCI_MODULE_NAME	"dpci"
#define DPCI_DEVICE_NAME	"dpci_core"
#define DPCI_DEVICE_NAME_IO	"dpci_io"

#define DPCI_CORE_IO_BAR	0
#define DPCIE_CORE_SRAM_BAR	1
#define DPCIE_CORE_ROM_BAR	2

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#define init_MUTEX(sem) sema_init(sem, 1)
#endif


/******************************************************************************
 *
 * OW (one-wire) Bus stuff
 */

/*
 * One-wire (OW/W1) support.
 *
 * You must choose either the Linux API (owfs, netlink etc.) or the DGL API but
 * not both.  It's not impossible they'll work together and at the same time
 * but it can't be guaranteed and testing all the error cases will be very
 * difficult.
 *
 * If you choose to keep USE_DGL_OW_API defined then you get the DGL API.  If
 * you undefine it then you'll get the Linux API *IF* CONFIG_W1 is set in your
 * .config.
 *
 * If you choose the Linux API One-wire API then understand that you're pretty
 * much on your own.  If you have trouble getting it to work, then first revert
 * to use of the DGL API to test the hardware before requesting support.
 */
#define USE_DGL_OW_API

#if defined(USE_DGL_OW_API)
#undef CONFIG_W1
#elif defined(CONFIG_W1) || defined(CONFIG_W1_MODULE)
#define CONFIG_W1
#include <linux/w1.h>
#endif

#define	DOWP_CONFIG(dowp)	*((int *)(&(dowp)->dowp_private))

/*
 * Timings for various things in One-wire land.  See app-note #126 at
 * http://www.maxim-ic.com/appnotes.cfm/appnote_number/126 for more information
 * about these.  Some are in nanoseconds, others in microseconds; this is to
 * avoid having __bad_ndelay() undefined in some of the kernels we support.
 */
#define	OWTM_A_STD_US		6	// Write 1 low period
#define	OWTM_A_OVR_NS		1500	//
#define	OWTM_B_STD_US		64	// Write 1 high period
#define	OWTM_B_OVR_NS		7500	//
#define	OWTM_C_STD_US		60	// Write 0 low period
#define	OWTM_C_OVR_NS		7500	//
#define	OWTM_D_STD_US		10	// Write 0 high period
#define	OWTM_D_OVR_NS		2500	//
#define	OWTM_E_STD_US		9	// Read pre-sample period
#define	OWTM_E_OVR_NS		750	//
#define	OWTM_F_STD_US		55	// Read post-sample period
#define	OWTM_F_OVR_US		7	//
#define	OWTM_G_STD_US		3	// Reset initial high period
#define	OWTM_G_OVR_US		3	//
#define	OWTM_H_STD_US		480	// Reset low period
#define	OWTM_H_OVR_US		70	//
#define	OWTM_I_STD_US		70	// Reset high period pre-sample
#define	OWTM_I_OVR_NS		8500	//
#define	OWTM_J_STD_US		410	// Reset high period
#define	OWTM_J_OVR_US		40	//

#define	OWTM_PROGPLS_US		480	// Program pulse timing

void dpci_ow_setup(struct dpci_device *ddp);
void dpci_ow_cleanup(struct dpci_device *ddp);
int dpci_ow_set_port_speed(struct dpci_ow_port *, int speed);
int dpci_ow_init_port(struct dpci_ow_port *);
int dpci_ow_set_port_level(struct dpci_ow_port *, int level);
u8 dpci_ow_touch_bit(struct dpci_ow_port *data, u8 bit);
u8 dpci_ow_reset_pulse(struct dpci_ow_port *data);
u8 dpci_ow_program_pulse(struct dpci_ow_port *data);
int dpci_ow_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


/******************************************************************************
 *
 * I2C-related stuff
 */
#define	DEFAULT_CLK_PERIOD_US	2 /* low period in us, *2 = 4us -> 250kHz */


void dpci_i2c_setup(struct dpci_device *ddp);
int user_i2c_command(struct dpci_device *ddp, unsigned long arg);
void i2c_bitbang_init(struct dpci_i2c_port *i2c_port_p);
int i2c_bitbang_command(struct dpci_device *ddp, struct dpci_i2c_cmdbuf *i2c_cmdbuf_p);



/******************************************************************************/
/*
 * IDLP (PIC) stuff.
 */
#define CLRWDT_DELAY_MS		4
#define SPI_BYTE_DELAY_MS	4
#define SPI_COMMAND_DELAY_MS	15
#define SPI_COMMAND_MAX_TRIES	4
#define IDLP_COMMAND_MAX_TRIES	8


void idlp_event_irq_handler(struct dpci_device *ddp);
irqreturn_t ip_irq_handler(struct dpci_device *ddp);
void idlp_ack_irq_handler(struct dpci_device *ddp);
void dpci_id_setup(struct dpci_device *ddp);
void dpci_id_cleanup(struct dpci_device *ddp);
int dpci_id_setbattcheckperiod(struct dpci_device *ddp, int code);
int dpci_id_getbattstatus(struct dpci_device *ddp);
int dpci_id_getbattlevel(struct dpci_device *ddp, int battno);
int dpci_id_setbatterrorlevel(struct dpci_device *ddp, int batt, int mV);
int dpci_id_getbatt_errorlevel(struct dpci_device *ddp, int battno);
int dpci_id_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int dpci_id_wait_event(struct dpci_device *ddp, 
		struct idevent_timeout *idevent_tm, 
		int wait_cont);
int dpci_id_readevent(struct dpci_device *ddp, struct idevent *idevent);

/******************************************************************************/
/*
 * These macros define the default major, minor numbers and the total number of
 * DPCI devices.  I doubt actually we'll ever have more than 1.  But for test
 * purposes.
 */
#define DPCI_DEVICE_COUNT	8
#define	DPCI_MAX_IO_REGS	256

/*
 * Event Queuing related stuff
 */

/* 
 * This structure defines an element of a event queue
 */
struct event_request
{
	ER_STATE_TYPE er_state;
	unsigned int er_type_mask;
	struct digip_param er_param;	// 32-bit bitmask for Digital i/p
	dpci_timestamp_t last_ts;

	wait_queue_head_t er_wqh;
	int er_signal;

	struct dpci_event op_event;	// To be removed
	struct op_event_list event_list;
	int num_events;
	spinlock_t event_list_lock;
	pid_t process_id;
	pid_t thread_group;
	
	int cont_wait;

	struct event_request *prev_er;
	struct event_request *next_er;
};

/*
 * Functions for event-queuing model
 */
void dpci_event_queue_dump(struct dpci_device *ddp);
void dpci_event_queue_setup(struct dpci_device *ddp);
void dpci_event_queue_cleanup(struct dpci_device *ddp);
struct event_request *init_event_request(unsigned int er_type, 
					struct digip_param er_param,
					int cont_wait);
void destroy_event_request(struct event_request *pevent);
int er_enable_interrupts(struct dpci_device *ddp, struct event_request *pevent);
int er_disable_interrupts(struct dpci_device *ddp, 
			unsigned int event_type);
int queue_insert_tail(struct dpci_device *ddp, 
		struct event_request *pevent);
int queue_remove_er(struct dpci_device *ddp, 
		struct event_request *pevent);
int dpci_wait_events(struct dpci_device *ddp, 
		unsigned int event_type_mask,
		unsigned long timeout,
		int cont_wait,
		struct dpci_event *op_event,
		unsigned long int_mask,
		unsigned long edge_state,
		unsigned long autoconfig_edge);

int op_event_list_add(struct op_event_list *plist, 
			struct dpci_event opevent);
int op_event_list_remove(struct op_event_list *plist, 
			struct dpci_event *pevent);
void update_er_thread_event(struct dpci_device *ddp);
void set_global_event_queue_flags(struct dpci_device *ddp, 
				struct dpci_event opevent);
void unset_global_event_queue_flags(struct dpci_device *ddp, 
				struct dpci_event opevent);
void get_timestamp(dpci_timestamp_t *tsp);

/*
 * This structure holds per-instance data for DPCI devices.
 */
struct dpci_driver;
struct dpci_device
{
	int board_no;
	struct dpci_driver *driver;
	struct dpci_device *next;
	struct pci_dev *pci_dev;
	struct cdev cdev;
	dev_t dev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	struct device *device;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
	struct class_device *class_dev;
#endif

/*
 * dpci_address refers to an Intel I/O port.  The Linux kernel uses the "int"
 * type in its prototypes for the in[bwl]() and out[bwl]() functions.
 *
 * We also set up the dpci revision and device ID (from the PCI header).
 */
	int address;
	int disabled;
	resource_size_t nv_address, nv_size;
	resource_size_t rom_address, rom_size;
	unsigned char revision;
	u8 irq_line;
	int iobd_users;
	int digio_users;

	/*
	 * EXINT interurpt handler.
	 */
	exint_handler_t exint_handler;
	void *exint_handler_data;

	/*
	 * Register caching: Because many registers don't return on read what
	 * you write to them, we keep a shadow copy of them at all times.
	 */
	u8 val_regs[DPCI_MAX_IO_REGS];
	spinlock_t val_regs_lock;

	/*
	 * Digital input stuff.
	 */
	dpci_off_t *reg_op;
	dpci_off_t *reg_ip;
	dpci_off_t *reg_ip_status;
	dpci_off_t *reg_ip_pol;
	dpci_off_t *reg_ip_enable;
	dpci_off_t *reg_ip_cfgeven;
	dpci_off_t *reg_ip_cfgodd;
	u8 val_ip_status[MAX_INPUT_PORTS];

	/*
	 * I2C stuff.
	 * The ddp->i2c_wqh wait queue head is used only for delays between
	 * attempts at an I2c command.
	 */
	struct dpci_i2c_port i2c_ports[MAX_I2C_PORTS];
	wait_queue_head_t i2c_wqh;

	/*
	 * watchdog stuff.	
	 */
	wait_queue_head_t clrwdt_wqh;
	/*
	 * One-wire stuff.
	 */
	struct dpci_ow_port ow_ports[MAX_OW_PORTS];

	/*
	 * This word is used to determine how to handle various hardware
	 * features that change between versions/revisions of the CPLD.
	 */
	int features;

	/*
	 * dpci_mainboard: the descriptor for the main CPLD.
	 * dpci_ioboard:   the descriptor for the I/O board expansion (if any).
	 */
	const struct dpci_board_id *board_id;
	const struct dpci_board *mainboard;
	const struct dpci_board *ioboard;
	struct dpci_hardware_profile dhwp;

	/*
	 * IDLP & IDLP+ access.
	 *
	 * idlp_cmd_sem protects IDLP transport media (whichever is used).
	 *
	 * idlp_data_wqh is used only to manage queued IDLP commands.  It is
	 * used slightly differently between the byte-wide and SPI interfaces.
	 * On the SPI-bus it implements the timing delay between bits and bytes.
	 * On the IDLP+ byte-wide bus, it is used to wake up the communicating
	 * task as each byte is acknowledged.
	 *
	 * idlp_command is a shortcut to finding the correct function to call
	 * when communication with the IDLP/IDLP+ - whatever type it is.  This
	 * makes it much easier to code - especially for the commands where the
	 * data format is not different.
	 *
	 * idlp_tx_buf stores all the tx bytes of the current command.  Likewise
	 * idlp_rx_buf points to where received bytes must be stored.  tx_count
	 * includes the command byte. Idlp_data_idx counts 0..tx_len for
	 * transmit bytes and then tx_len..tx_len+rx_len-1 for receive
	 * bytes.
	 * idlp_irq_status is the status of the last interrupt driven command
	 * as reported by the IRQ handler.
	 */
	unsigned int idlp_fwversion;
	struct semaphore idlp_cmd_sem;
	wait_queue_head_t idlp_data_wqh;
	int (*idlp_command)(struct dpci_device *, u8, u8 *, int, u8 *, int);
#define	MAX_IDLP_DATA	64
	unsigned char idlpplus_mode;
#define IDLP_PLUS_INTR	0
#define IDLP_PLUS_QUICK	1
	unsigned char idlp_irq_status;
#define IIS_INACTIVE	0
#define IIS_WORKING	1
#define IIS_FAILED	2
#define IIS_REJECTED_CMD	3
#define IIS_REJECTED_DATA	4
#define IIS_TOO_LITTLE_DATA	5
#define IIS_TOO_MUCH_DATA	6
#define IIS_EXEC_ERROR	7
#define IIS_COMPLETED	8
	unsigned char idlp_cmd;
	unsigned char *idlp_tx_buf;
	unsigned char *idlp_rx_buf;
	unsigned int idlp_tx_len;
	unsigned int idlp_rx_len;
	unsigned int idlp_aa_len;
	unsigned int idlp_data_idx;
	spinlock_t idlp_buf_spinlock;

	/*
	 * Temporary variables for backward compatibility of ID_WAITEVENTS
	 */
	struct idevent idevent_temp;
	unsigned char idevent_temp_flag;

	/*
	 * Temperature sensor stuff.
	 */
	pid_t ts_pid;
	int ts_signal;
	
	/*
	 * Password-related stuff
	 */
	int flag_date_psw_active;
	int flag_event_psw_active;
	
	/*
	 * Event Queue-related stuff
	 * 1: queue of event requests
	 */
	struct event_request *er_queue_head;
	struct event_request *er_queue_tail;
	int er_queue_len;
	spinlock_t event_queue_lock;

	/*
	 * Event Queue-related stuff
	 * 2: other data for event queue thread
	 */
	struct task_struct *er_thread;
	spinlock_t intr_enable_lock;
	wait_queue_head_t er_thread_wqh;
	int er_thread_signal;		// wake up er thread to do something
	struct event_request thread_event;	// all ERs collated herein
	int event_flag;
	int er_stopped;
	
	/*
	 * Output queues related stuff (one for each user thread)
	 */
	struct op_event_list global_event_list;
	spinlock_t global_event_list_lock;
	struct event_flags global_queue_flags;
	spinlock_t global_queue_flags_lock;
	
	/* 
	 * User-queues processing thread
	 */
	struct task_struct *uq_thread;
	wait_queue_head_t uq_thread_wqh;
	int uq_thread_signal;		// wake up uq thread to do something
	int uq_stopped;

	/*
	 * Flag to check if a PFD event has been logged yet.
	 */
	unsigned char pfd_event_logged;
};


/*
 * The structure contains data relevant to the dpci driver generally.
 */
struct dpci_driver
{
	dev_t dev;
	int dev_count;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
	struct class *dpci_class;
#else
	struct class_simple *dpci_class;
#endif
	struct dpci_device *devices;
};


/******************************************************************************/
/*
 * PRINT_INFO is printk(LOG_INFO...) with our driver name prepended to output
 * PRINT_ERR is printk(LOG_ERR...) with our driver name prepended to output
 * PRINT_WARN is printk(LOG_WARN...) with our driver name prepended to output
 */
#define PRINT_INFO(...) printk(KERN_INFO DPCI_DEVICE_NAME ": " __VA_ARGS__)
#define PRINT_ERR(...) printk(KERN_ERR DPCI_DEVICE_NAME ": " __VA_ARGS__)
#define PRINT_WARN(...) printk(KERN_WARNING DPCI_DEVICE_NAME ": " __VA_ARGS__)

/*
 * Support for debugging output.
 *
 * Set the debug variable during module load.
 *
 * The remaining PRINT_* all do nothing unless debug is non-zero.
 *
 * PRINT_DBG is the same as printk if debug non-zero.  It is for
 * general debugging use.
 *
 * PRINT_IOC is the same as printk if debug has bit 0 set.  It is
 * reserved for ioctl handling functions.
 *
 * PRINT_RA is the same as printk if debug has bit 1 set.  It is
 * reserved for the dpci_io_{read,write}{byte,word,dword} functions.
 *
 * PRINT_IDLP is the same as printk if debug has bit 2 set.  It is
 * reserved for the SPI functions.
 *
 * PRINT_REG0 is the same as printk if debug has bit 3 set.  It is
 * reserved for functions changing register 0.
 *
 * PRINT_EE is the same as printk if debug has bit 4 set.  It is
 * reserved for functions concerning EEPROM access.
 *
 * PRINT_IO is the same as printk if debug has bit 5 set.  It is
 * reserved for functions concerning digital I/O access.
 *
 * PRINT_I2C is the same as printk if debug has bit 6 set.  It is
 * reserved for functions concerning I2C access.
 *
 * PRINT_OW is the same as printk if debug has bit 7 set.  It is
 * reserved for functions concerning W1 access.
 *
 * DEBUG_USERREGS - if set this allows user-land access to system registers
 */
#define	DEBUG_IOCTL	0x0001
#define	DEBUG_RA	0x0002
#define	DEBUG_IDLP	0x0004
#define	DEBUG_REG0	0x0008
#define	DEBUG_EE	0x0010
#define	DEBUG_IO	0x0020
#define	DEBUG_I2C	0x0040
#define	DEBUG_OW	0x0080
#define	DEBUG_IRQ	0x0100
#define	DEBUG_USERREGS	0x8000

#ifdef DEBUG
#define	LOG_LEVEL	KERN_INFO
# define PRINT_DBG(fmt, ...) \
	{ \
		if (debug & ~DEBUG_USERREGS) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
	}
# define PRINT_IOC(fmt, ...) \
	{ \
		if (debug & DEBUG_IOCTL) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
	}
# define PRINT_RA(fmt, ...) \
	{ \
		if (debug & DEBUG_RA) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
	}
# define PRINT_IDLP(fmt, ...) \
	{ \
		if (debug & DEBUG_IDLP) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
	}
# define PRINT_REG0(fmt, ...) \
	{ \
		if (debug & DEBUG_REG0) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
	}
# define PRINT_EE(fmt, ...) \
	{ \
		if (debug & DEBUG_EE) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
	}
# define PRINT_IO(fmt, ...) \
	{ \
		if (debug & DEBUG_IO) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
	}
# define PRINT_I2C(fmt, ...) \
	{ \
		if (debug & DEBUG_I2C) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__,  ##__VA_ARGS__); \
	}
# define PRINT_OW(fmt, ...) \
	{ \
		if (debug & DEBUG_OW) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__,  ##__VA_ARGS__); \
	}
# define PRINT_IRQ(fmt, ...) \
	{ \
		if (debug & DEBUG_IRQ) \
		printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__,  ##__VA_ARGS__); \
	}
#else
# define PRINT_DBG(...)		do {} while (0)
# define PRINT_IOC(...)		do {} while (0)
# define PRINT_RA(...)		do {} while (0)
# define PRINT_IDLP(...)	do {} while (0)
# define PRINT_REG0(...)	do {} while (0)
# define PRINT_EE(...)		do {} while (0)
# define PRINT_IO(...)		do {} while (0)
# define PRINT_I2C(...)		do {} while (0)
# define PRINT_OW(...)		do {} while (0)
# define PRINT_IRQ(...)		do {} while (0)
#endif

/*
 * Useful function prototypes.
 */
irqreturn_t multiboard_irq_handler(struct dpci_device *ddp, u8 reg0);
irqreturn_t oneboard_irq_handler(struct dpci_device *ddp, u8 reg0);
irqreturn_t ioboard_80_1003_irq_handler(struct dpci_device *ddp, u8 reg0);
irqreturn_t ioboard_80_0062_irq_handler(struct dpci_device *ddp, u8 reg0);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
irqreturn_t irq_handler(int irq, void *dev_id);
#else
irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs);
#endif
const struct dpci_board *dpci_find_board(int tag);

void dpci_disable_digio_intr(struct dpci_device *ddp);
void dpci_enable_digio_intr(struct dpci_device *ddp);
u8 dpci_reg_set(struct dpci_device *ddp, dpci_off_t reg, u8 val);
u8 dpci_reg_chg(struct dpci_device *ddp, dpci_off_t reg, u8 set, u8 clear, u8 xor);
u8 dpci_reg_get(struct dpci_device *ddp, dpci_off_t reg);
u8 dpci_reg_get_saved(struct dpci_device *ddp, dpci_off_t reg);
int dpci_io_modifyint(struct dpci_device *ddp, int port, u8 int_setmask, u8 int_clrmask, u8 pol_mask);
int dpci_io_wait_int(struct dpci_device *ddp, 
		struct waitint_port *waitint_port,
		int cont_wait);
int dpci_io_wait_int32(struct dpci_device *ddp, struct waitint_32 *waitint_32);
int dpci_io_wait_pfd(struct dpci_device *ddp, unsigned long arg);
int dpci_ts_wait_alarm(struct dpci_device *ddp, unsigned long arg);
void dpci_ow_relinquish(struct dpci_device *ddp);
int dpci_iowd_enable(struct dpci_device *ddp);
int dpci_iowd_disable(struct dpci_device *ddp);
int dpci_iowd_pat(struct dpci_device *ddp);
int dpci_gpio_read_ip(struct dpci_device *ddp, struct memloc *mem_loc);
int dpci_gpio_write_op(struct dpci_device *ddp, struct memloc mem_loc);
int dpci_gpio_read_op(struct dpci_device *ddp, struct memloc *mem_loc);

int dpci_id_wdenable(struct dpci_device *ddp, int value);
int dpci_id_wdreset(struct dpci_device *ddp);
int dpci_id_intenable(struct dpci_device *ddp);
int dpci_id_intdisable(struct dpci_device *ddp);
int dpci_wait_allevents(struct dpci_device *ddp, struct dpci_event_timeout *op_event_timeout);
void dpci_event_release_requests(struct dpci_device *ddp);
void dpci_event_release_current_request(struct dpci_device *ddp, pid_t thr_id);

extern int debug;
extern int disable_idlp;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DPCI_CORE_PRIV_H */


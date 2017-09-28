/******************************************************************************
 *
 * $Id: dpci_mem_priv.h 11737 2015-06-26 11:01:30Z aidan $
 *
 * Copyright 2008 Inncoore Gaming Limited.
 * All rights reserved.
 *
 * License:	GPLv2
 *
 * Description:
 * Internal macros for the DirectPCI Core I/O kernel driver.
 *
 *****************************************************************************/
#ifndef _DPCI_MEM_PRIV_H
#define _DPCI_MEM_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#define BYTE                                    1
#define WORD                                    2
#define DWORD                                   4

#define DPCI_SRAMROM_MEMORY_BAR			0

#define MEM_SECTOR_SIZE                         512
#define MEM_SECTOR_SHIFT                        9


const struct mem_board
{
	const char name[8];
	int writable;
} mem_boards[] = {
	{"sram", 1},
	{"rom", 0},
};

struct mem_device
{
	/*
	 * Internal meta data.
	 */
	char name[8];
	struct mem_board *mem_dev;	/* info on device this is */
	struct mem_device *next;	/* next in driver's list */

	/*
	 * Kernel meta data.
	 */
	struct cdev cdev;			/* for file interface */
	dev_t dev;				/* major:minor */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	struct device *device;
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
	struct class_device *class_dev;
#endif

	/*
	 * Access management
	 */
	struct rw_semaphore rwsem;		/* concurrency protection */

	/*
	 * Physical characteristics
	 */
	struct pci_dev *pci_dev;		/* pci_dev from probe time */
	unsigned short device_id;		/* PCI device id */
	resource_size_t startAddress;		/* start address from header */
	resource_size_t length;			/* length */
	unsigned long flags;			/* flags from pci header */
	void* p_remapAddr;			/* address from ioremap */

	/*
	 * MEM block mode fields.
	 */
	struct gendisk *disk;			/* gen hard disk struct */
	struct block_device *bdev;		/* block device struct */
	struct request_queue *rd_queue;		/* request queue */
	spinlock_t b_spinlock;
	int block_minor;

	unsigned char *io_buffer;		/* read/write buffer */
};

/*
 * This structure will store the module's data.
 */
struct mem_driver 
{
	dev_t cdev;
	int block_major;
	int block_minor;
	int sram_count;
	int rom_count;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
	struct class *mem_class;
#else
	struct class_simple *mem_class;
#endif
	struct mem_device *devices;
};

/*
 * Support for debugging output.
 */
#define DPCI_DEVICE_NAME "dpci_mem"
#define PRINT_INFO(...) printk(KERN_INFO DPCI_DEVICE_NAME ": " __VA_ARGS__)
#define PRINT_ERR(...) printk(KERN_ERR DPCI_DEVICE_NAME ": " __VA_ARGS__)
#define PRINT_WARN(...) printk(KERN_WARNING DPCI_DEVICE_NAME ": " __VA_ARGS__)
#ifdef DEBUG
# define LOG_LEVEL              KERN_INFO
# define PRINT_DBG(fmt, ...) \
	{ \
		if (debug) \
		{ \
			printk(LOG_LEVEL DPCI_DEVICE_NAME ": %s: " fmt, __func__, ##__VA_ARGS__); \
		} \
	}
#else
# define PRINT_DBG(...)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DPCI_MEM_PRIV_H */

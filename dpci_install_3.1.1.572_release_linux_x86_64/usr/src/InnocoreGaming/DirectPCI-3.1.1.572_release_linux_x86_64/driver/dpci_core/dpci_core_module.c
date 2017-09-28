/******************************************************************************
 *
 * $Id: dpci_core_module.c 11955 2015-09-22 22:07:03Z aidan $
 *
 * Copyright 2003-2015 Advantech Co Limited.
 * All rights reserved.
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
 * Description:
 * DirectPCI core module for kernel-mode
 *
 *****************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/input.h>
#include <linux/cdev.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

/*include the file that defines the constants for the dpci-core driver.*/

#include <linux/dpci_core_ioctl.h>
#include <linux/dpci_multi.h>
#include "dpci_version.h"
#include "dpci_core_hw.h"
#include "dpci_core_types.h"
#include "dpci_core_priv.h"

#include "dpci_core_boards.h"

#ifndef CONFIG_PCI
# error "CONFIG_PCI must be defined in .config for this driver to build."
#endif

/*
 * Debug control word - always available - even for release builds.
 */
int debug = 0;
short use_board_code = 0;
#ifdef CONFIG_PCI_MSI
unsigned int nomsi = 1;
#endif

/*
 * IDLP disable control
 */
int disable_idlp = 0;

static int n_boards = 0; /* how many boards we found */

/******************************************************************************/
/*
 * Function declarations
 */
static void __exit dpcicore_cleanup_resource(struct dpci_device *ddp);
static void __exit dpcicore_cleanup_all_resources(void);
static void __exit dpcicore_cleanup_static(void);
static int __init dpcicore_init(void);
static void __exit dpcicore_exit(void);
static int dpci_probe(struct pci_dev *device_to_probe, const struct pci_device_id *id_to_probe);
static void dpci_remove(struct pci_dev *dev_to_remove);

/******************************************************************************/

/*
 * This variable stores driver's data
 */
static struct dpci_driver dpci_driver;

/*
 * Define the PCI ID structure that contains Innocore's IDs.
 * The last field is the driver_data field - we use the to indicate customer-
 * specific driver options.
 */
#define	DENSITRON_PCI_DEVICE_CLASS(dev_class) \
	.class = (dev_class) << 8, \
	.class_mask = 0xffffff00, \
	.vendor = DENSITRON_VENDOR_ID, \
	.device = PCI_ANY_ID, \
	.subvendor = PCI_ANY_ID, \
	.subdevice = PCI_ANY_ID
static struct pci_device_id dpci_ids[] =
{
	{DENSITRON_PCI_DEVICE_CLASS(0x0880)},
	{DENSITRON_PCI_DEVICE_CLASS(0x8080)},
	{ 0 }
};


/*
 * Define the stucture that exposes the driver to the PCI core
 */
static struct pci_driver dpci_pci_driver =
{
	.name = DPCI_MODULE_NAME,
	.id_table = dpci_ids,
	.probe = dpci_probe,
	.remove = dpci_remove,
};

extern struct file_operations dpci_fops;

/*******************************************************************************
 *
 * Function:    dpci_get_device()
 *
 * Parameters:  no - the device number to get.
 *
 * Returns:     struct dpci_device * - 
 *
 * Description: Returns the ID byte as described, assuming the I/O card's 1st
 *		register behaves as the standard I/O board does (ID 0x03).
 *
 ******************************************************************************/
struct dpci_device *dpci_get_device(int board)
{
	struct dpci_device *ddp;

	for (ddp = dpci_driver.devices; board-- && ddp; ddp = ddp->next)
		;
	return ddp;
}


/*******************************************************************************
 *
 * Function:    dpci_init_params()
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Find the exact dpx-series board we are running on and set  
 *		additional features and parameters specific to the board.
 *
 ******************************************************************************/
static void dpci_init_params(struct dpci_device *ddp)
{
	int board_id;

	/*
	 * If it's a DPX-S430 with firmware 59.1 (which has the ability to
	 * check intrusion 6/7) then we assume it's really a DPX-C710.
	 */
	if ((ddp->idlp_fwversion > 255) &&
		(IDLP_VERSION_CONF2(ddp->idlp_fwversion) & IDLP_VERSION_CONF2_INTR67) &&
		(ddp->mainboard->db_board_id == DPX_S430))
	{
		const struct dpci_board *mbp;

		board_id = DPX_C710;
		mbp = dpci_find_board(board_id);
		if (mbp)
		{
			ddp->mainboard = mbp;
		}
		else
		{
			PRINT_ERR("couldn't get DPX-C710 resource definitions.\n");
		}
	}
}


/*******************************************************************************
 *
 * Function:    dpcicore_init()
 *
 * Parameters:  none
 *
 * Returns:     int - 0 for success, -ve for error.
 *
 * Description: Initialise the device driver.  The function is called when the
 *		driver is first loaded into memory,  It just registers the
 *		driver with the PCI core and creates a simple class for the
 *		sysfs interface.
 *
 ******************************************************************************/
static int __init dpcicore_init(void)
{
	int res;

	PRINT_INFO("Advantech Innocore DirectPCI core driver "
		"v%s $Revision: 11955 $\n",
		DPCI_VERSION);
	PRINT_INFO("Advantech Innocore DirectPCI core driver "
		"compiled " B__DATE__ ", " B__TIME__ "\n");
	PRINT_INFO("(C) 2003-2015, Advantech Co Ltd.\n");
	PRINT_DBG("debug=0x%x\n", debug);
#ifndef DEBUG
	if (debug)
	{
		PRINT_WARN("debug=0x%x but this is not a DEBUG driver: "
				"please load the debug driver if you want debug output.\n", debug);
	}
#endif

	/*
	 * Create char-device and let the kernel define the charmajor.
	 * Check if the device file has been created successfully.
	 */
	res=alloc_chrdev_region(&dpci_driver.dev,
					0,
					DPCI_DEVICE_COUNT,
					DPCI_DEVICE_NAME);
	if (res < 0)
	{
		PRINT_ERR("can't create DirectPCI device.\n");
	  	/*
		 * Set the major variable to 0 so the clean up function does
		 * not call unregister_chrdev_region
		 */
		goto done;
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
	dpci_driver.dpci_class =
			class_create(THIS_MODULE, DPCI_MODULE_NAME);
#else
	dpci_driver.dpci_class =
			class_simple_create(THIS_MODULE, DPCI_MODULE_NAME);
#endif
	if(IS_ERR(dpci_driver.dpci_class))
	{
		PRINT_ERR("Failed to create DPCI class.\n");
		res = -ENOMEM;
		goto done;
	}

	/*
	 * Register the DirectPCI PCI driver.
	 */
	res = pci_register_driver(&dpci_pci_driver);
	if (res)
	{
		PRINT_ERR("couldn't load the driver: error %d\n",
			res);
	}
	else
	{
		PRINT_DBG("Advantech Innocore DirectPCI core driver loaded\n");
	}
done:
	return res;
}


/*******************************************************************************
 *
 * Function:    dpci_init_features()
 *
 * Parameters:  dev - the PCI device instance we are requested to handle.
 *
 * Returns:     nothing
 *
 * Description: Work out what features this version of the hardware supports
 *		and setup the features word accordingly.
 *
 ******************************************************************************/
static void dpci_init_features(struct dpci_device *ddp)
{
	ddp->features = ddp->mainboard->db_base_features;
	if (ddp->ioboard)
	{
		ddp->features |= ddp->ioboard->db_base_features;
	}
}


/*******************************************************************************
 *
 * Function:    dpci_hw_setup()
 *
 * Parameters:  dev - the PCI device instance we are requested to handle.
 *
 * Returns:     int - 0 for success, -ve for error.
 *
 * Description: Work out what resources are needed to attach to the PCI device
 *		instance in question and then allocate them.  It is a fatal and
 *		unrecoverable error if any resource cannot be allocated.
 *
 ******************************************************************************/
static int dpci_hw_setup(struct dpci_device *ddp)
{
	int res;

	/*
	 * Get DPCI base address register 0 setting.
	 */
	pci_read_config_dword(ddp->pci_dev, PCI_BASE_ADDRESS_0, &ddp->address);

	if ((ddp->address & PCI_BASE_ADDRESS_SPACE) !=
						PCI_BASE_ADDRESS_SPACE_IO)
	{
		PRINT_ERR("Direct PCI space is not I/O!\n");
		return -EIO;
	}

	ddp->address = pci_resource_start(ddp->pci_dev, DPCI_CORE_IO_BAR);
	PRINT_DBG("DirectPCI I/O space is at 0x%x\n", ddp->address);

	if (ddp->board_id->dpci_did == DPCI_DEVICE_CORE_PCIE_ID)
	{
		ddp->nv_address = pci_resource_start(ddp->pci_dev, DPCIE_CORE_SRAM_BAR);
		ddp->nv_size = pci_resource_len(ddp->pci_dev, DPCIE_CORE_SRAM_BAR);
		ddp->rom_address = pci_resource_start(ddp->pci_dev, DPCIE_CORE_ROM_BAR);
		ddp->rom_size = pci_resource_len(ddp->pci_dev, DPCIE_CORE_ROM_BAR);
	}

	/*
	 * Get interrupt line, disable interrupts and then request interrupt.
	 */
	dpci_reg_chg(ddp, DPCI_REG0, 0, 0xff, 0);
#ifdef CONFIG_PCI_MSI
	/*
	 * See if MSI (message-signalled interrupts) are available and the
	 * user hasn't disabled them.  We need to enable bus-mastering mode to
	 * use MSI as that has to be enabled for a MSI-capable PCI target to
	 * perform the inbound write that signals the actual interrupt.
	 *
	 * MSI is only available on PCIe-based designs.
	 */
	if (!nomsi && ddp->board_id->dpci_did == DPCI_DEVICE_CORE_PCIE_ID)
	{
		int got;

		pci_set_master(ddp->pci_dev);
		got = pci_enable_msi(ddp->pci_dev);
		if (got != 0)
		{
			printk(KERN_WARNING "Couldn't enable MSI for %s\n",
				pci_name(ddp->pci_dev));
			nomsi = 1;
		}
	}
#endif
	ddp->irq_line = ddp->pci_dev->irq;
	PRINT_DBG("irq line is %d\n", ddp->irq_line);
#ifdef IRQF_SHARED
#ifdef IRQF_SAMPLE_RANDOM /* Removed in Linux 3.6rc1 */
# define DPCI_IRQ_FLAGS (IRQF_SAMPLE_RANDOM | IRQF_SHARED)
#else
# define DPCI_IRQ_FLAGS (IRQF_SHARED)
#endif
#else
# define DPCI_IRQ_FLAGS (SA_SHIRQ | SA_INTERRUPT | SA_SAMPLE_RANDOM)
#endif
	res = request_irq(ddp->irq_line,
				irq_handler,
				DPCI_IRQ_FLAGS,
				"dpci",
				ddp);
	if (res != 0)
	{
		PRINT_ERR("request for irq %d failed: error %d\n",
			ddp->irq_line, res);
		ddp->irq_line = 0;
		return -EBUSY;
	}

	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_probe()
 *
 * Parameters:  device_to_probe - the pci device instance to probe
 *		id_to_probe - the vendor/device details for the device
 *
 * Returns:     int - 0 for success, -ve for error.
 *
 * Description: called by the PCI device manager when the device is discovered,
 *		the probe checks the device is the correct device, enables it
 *		and then tries to set it up.  If this can all be done, the
 *		appropriate device node admninistrative sturctures are enabled
 *		and a set up occurs for I/O card devices too.
 *
 *		If any error occurs, then a complete back out occurs and the
 *		driver does not attempt to support any functionality at all.
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static int __devinit dpci_probe(struct pci_dev *device_to_probe,
			    const struct pci_device_id *id_to_probe)
#else
static int dpci_probe(struct pci_dev *device_to_probe,
			    const struct pci_device_id *id_to_probe)
#endif
{
	int res;
	struct dpci_device *ddp = NULL;
	const struct dpci_board *mbp;
	const struct dpci_board_id *bidp;
	int board_code;
	u8 io_board_code;
	struct pci_dev *rb_dev = NULL;

	if(!id_to_probe || !device_to_probe)
	{
		return (-EFAULT); /*NULL reference*/
	}

	/*
	 * Check we support the board.  If the use_board word has been set then
	 * allow that setting to override the PCI configuration word device_id.
	 */

	/*
	 * Get the root-bridge device and vendor IDs; with the FPGA/CPLD device
	 * IDs, these help us identify the exact board type.
	 */
	rb_dev = pci_get_class(PCI_CLASS_BRIDGE_HOST << 8, rb_dev);
	if (rb_dev == NULL)
		return -ENODEV;

	bidp = dpci_board_find_board_by_ids(
				device_to_probe->vendor,
				device_to_probe->device,
				(unsigned short)(rb_dev->vendor & 0xffff), 
				(unsigned short)(rb_dev->device & 0xffff));
	if (!bidp)
	{
		PRINT_ERR("PCI device %04x:%04x on root-bridge %04x:%04x not supported\n",
			(unsigned short)device_to_probe->vendor,
			(unsigned short)device_to_probe->device,
			(unsigned short)(rb_dev->vendor & 0xffff), 
			(unsigned short)(rb_dev->device & 0xffff));
		res = -ENODEV;
		goto clean_quit;
	}

	if (use_board_code)
	{
		board_code = use_board_code;
		PRINT_DBG("Board type overriden by user: was %04x now %04x\n",
				bidp->board_code, use_board_code);
	}
	else
	{
		board_code = bidp->board_code;
	}

	mbp = dpci_find_board(board_code);
	if (!mbp)
	{
		PRINT_ERR("PCI device %04x:%04x not supported although board_code=%d\n",
			device_to_probe->vendor,
			device_to_probe->device,
			board_code);
		res = -ENODEV;
		goto clean_quit;
	}

	ddp = kmalloc(sizeof(*ddp), GFP_KERNEL);
	if (!ddp)
	{
		PRINT_ERR("Failed to acquire memory for device instance\n");
		return -ENOMEM;
	}

	memset(ddp, 0, sizeof(*ddp));
	ddp->board_no = n_boards++;
	ddp->board_id = bidp;
	ddp->mainboard = mbp;
	ddp->driver = &dpci_driver;
	ddp->pci_dev = device_to_probe;
	ddp->dev = MKDEV(MAJOR(dpci_driver.dev),
			MINOR(dpci_driver.dev) + dpci_driver.dev_count);
	spin_lock_init(&ddp->val_regs_lock);

	ddp->dhwp.dpci_vendor_id = device_to_probe->vendor;	
	ddp->dhwp.dpci_device_id = device_to_probe->device;	
	ddp->dhwp.rb_vendor_id = rb_dev->vendor;	
	ddp->dhwp.rb_vendor_id = rb_dev->device;	
	ddp->dhwp.board_code = bidp->board_code;	
	pci_read_config_byte(ddp->pci_dev, PCI_REVISION_ID, &ddp->revision);
	ddp->dhwp.dpci_rev = ddp->revision;	

	/*
	 * Enable the dpci device
	 */
	if(pci_enable_device(device_to_probe))
	{
		PRINT_ERR("Failed to enable device.\n");
		res = -EIO;  /*Replace with enum error value!!*/
		goto clean_quit;
	}

	/*
	 * Get the address and irq of the device
	 */
	if (dpci_hw_setup(ddp) != 0)
	{
		PRINT_ERR("failed to set up device.\n");
		res = -EIO;
		goto clean_quit;
	}
	PRINT_INFO("board%d: %s (type #%d DPCI VIDs/DIDs=%04x:%04x rev.%02x, on root-bridge %04x:%04x)\n",
			ddp->board_no,
			ddp->board_id->board_name,
			ddp->board_id->board_code,
			ddp->board_id->dpci_vid,
			ddp->board_id->dpci_did,
			ddp->revision,
			rb_dev->vendor & 0xffff,
			rb_dev->device & 0xffff);

	/*
	 * Check the hardware revision meets the minimum requirement.
	 */
	if (ddp->revision < mbp->db_min_rev)
	{
		PRINT_ERR("Hardware revision %02x is earlier than minimum "
				"supported revision %02x for this board\n"
				"Some features may not work as expected or "
				"performance may be substandard.\n",
				ddp->revision,
				mbp->db_min_rev);
	}

	/*
	 * Register the character device interface with the kernel.  This
	 * simply links this module and the file operations to the character
	 * device number allocated above.
	 */
	cdev_init(&ddp->cdev, &dpci_fops);
	ddp->cdev.owner = THIS_MODULE;
	ddp->cdev.ops = &dpci_fops;
	res = cdev_add(&ddp->cdev, ddp->dev, 1);
	if(res < 0)
	{
		PRINT_ERR("failed to register DirectPCI device.");

		/*
		 * Set the owner to NULL so the clean up function knows that
		 * the initialisation failed.
		 */
		ddp->cdev.owner = NULL;
		goto clean_quit;
	}

	/*
	 * Register the DPCI class
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	ddp->device = device_create(dpci_driver.dpci_class,
					NULL,
					ddp->dev,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
                                        NULL,
#endif
					"dpci%d",
					dpci_driver.dev_count);
	if(IS_ERR(ddp->device))
	{
		PRINT_ERR("Failed to add DPCI device to DPCI class.");
		res = -ENOMEM;
		goto clean_quit;
	}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
	ddp->class_dev = class_device_create(dpci_driver.dpci_class,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14)
					NULL,
#endif
					ddp->dev,
					&device_to_probe->dev,
					"dpci%d",
					dpci_driver.dev_count);
	if(IS_ERR(ddp->class_dev))
	{
		PRINT_ERR("Failed to add DPCI device to DPCI class.");
		res = -ENOMEM;
		goto clean_quit;
	}
#else
	res = (int)class_simple_device_add(dpci_driver.dpci_class,
						ddp->dev,
						&device_to_probe->dev,
						"dpci%d",
						dpci_driver.dev_count);						
	if (IS_ERR((struct class_device *)res))
	{
		PRINT_ERR("Failed to add DPCI device to DPCI class.");
		goto clean_quit;
	}
#endif
	dpci_driver.dev_count++;

	/*
	 * Make sure we can allocate the I/O space for our main-board device.
	 * This should always pass if the PCI bus is set up correctly.  But we
	 * still code for failure handling.
	 */
	if (!request_region(ddp->address +
				ddp->mainboard->db_sys_start,
				ddp->mainboard->db_sys_size,
				ddp->mainboard->db_name))
	{
		PRINT_ERR("failed to acquire io-board region "
			"%x-%x.\n",
			ddp->address +
				ddp->mainboard->db_sys_start,
			ddp->address +
				ddp->mainboard->db_sys_start +
				ddp->mainboard->db_sys_size);

		/*Reset the ddp->address variable*/
		res = -EBUSY;
		goto failed;
	}

	/*
	 * See if an I/O board is inserted.
	 *
	 * There's an implicit assumption here that if a main board can have an
	 * external I/O board connected then that main board cannot itself have
	 * its own digital I/O.  Theoretically this is possibly but at present
	 * there are no examples of having on-board and off-board digital I/O.
	 */
	ddp->ioboard = NULL;
	if (ddp->mainboard->db_base_features & HAVE_DENSITRON_IOEXP)
	{
		io_board_code = dpci_get_ioboard_id(ddp);
		ddp->ioboard = dpci_find_board(io_board_code);
		if (ddp->ioboard)
		{
			PRINT_INFO("I/O board ID 0x%02x rev %02x "
				"(%s) attached\n",
				io_board_code,
				dpci_get_ioboard_rev(ddp),
				ddp->ioboard->db_name);

			/*
			 * Check the hardware revision meets the
			 * minimum requirement.
			 */
			if (dpci_get_ioboard_rev(ddp) < ddp->ioboard->db_min_rev)
			{
				PRINT_ERR("Hardware revision %02x is earlier than minimum "
						"supported revision %02x for this board.\n"
						"Some features may not work as expected or "
						"performance may be substandard.\n",
						dpci_get_ioboard_rev(ddp),
						ddp->ioboard->db_min_rev);
			}

			if (!request_region(ddp->address +
						ddp->ioboard->db_sys_start,
						ddp->ioboard->db_sys_size,
						ddp->ioboard->db_name))
			{
				PRINT_ERR("failed to acquire io-board region "
					"%x-%x.\n",
					ddp->address +
						ddp->ioboard->db_sys_start,
					ddp->address +
						ddp->ioboard->db_sys_start +
						ddp->ioboard->db_sys_size);

				/*Reset the ddp->address variable*/
				res = -EBUSY;
				goto failed;
			}
			if (ddp->ioboard->db_ip_data_regs[0])
			{
				ddp->reg_op = (dpci_off_t *)
						ddp->ioboard->db_op_regs;
				ddp->reg_ip = (dpci_off_t *)
						ddp->ioboard->db_ip_data_regs;
				ddp->reg_ip_status = (dpci_off_t *)
						ddp->ioboard->db_ip_stat_regs;
				if (ddp->ioboard->db_base_features & HAVE_NEWDIGIOREGS)
				{
					ddp->reg_ip_cfgeven = (dpci_off_t *)
						ddp->ioboard->db_ip_dicevn_regs;
					ddp->reg_ip_cfgodd = (dpci_off_t *)
						ddp->ioboard->db_ip_dicodd_regs;
				}
				else
				{
					ddp->reg_ip_pol = (dpci_off_t *)
						ddp->ioboard->db_ip_pol_regs;
					ddp->reg_ip_enable = (dpci_off_t *)
						ddp->ioboard->db_ip_inte_regs;
				}
			}
		}
		else if (DPCI_IOBOARD_PRESENT(io_board_code))
		{
			PRINT_ERR("I/O board ID %02x rev %02x "
				"not supported\n",
				io_board_code,
				dpci_get_ioboard_rev(ddp));
		}
		else
		{
			PRINT_INFO("no I/O board attached\n");
		}
	}
	else if (ddp->mainboard->db_ip_data_regs[0])
	{
		ddp->reg_op = (dpci_off_t *)
				ddp->mainboard->db_op_regs;
		ddp->reg_ip = (dpci_off_t *)
				ddp->mainboard->db_ip_data_regs;
		ddp->reg_ip_status = (dpci_off_t *)
				ddp->mainboard->db_ip_stat_regs;
		if (ddp->mainboard->db_base_features & HAVE_NEWDIGIOREGS)
		{
			ddp->reg_ip_cfgeven = (dpci_off_t *)
					ddp->mainboard->db_ip_dicevn_regs;
			ddp->reg_ip_cfgodd = (dpci_off_t *)
					ddp->mainboard->db_ip_inte_regs;
		}
		else
		{
			ddp->reg_ip_pol = (dpci_off_t *)
					ddp->mainboard->db_ip_pol_regs;
			ddp->reg_ip_enable = (dpci_off_t *)
					ddp->mainboard->db_ip_inte_regs;
		}
	}
	PRINT_DBG("reg_ip_pol=%p reg_ip_enable=%p "
			"reg_ip_cfgeven=%p reg_ip_cfgodd=%p\n",
			ddp->reg_ip_pol,
			ddp->reg_ip_enable,
			ddp->reg_ip_cfgeven,
			ddp->reg_ip_cfgodd);

	/*
	 * Initialise the features word now we know if there's an I/O board.
	 */
	dpci_init_features(ddp);
	PRINT_DBG("features word %08x\n", ddp->features);

	/*
	 * See if we have MBDIS register disabling and if so, if MBDIS is
	 * flagged.  On the PCIe implementation, the FPGA attempts to talk to
	 * the IDLP.  If it fails then write access to SRAM and I/O registers
	 * is disabled.  This continues until the next reset, and the condition
	 * is checked once again.
	 *
	 * We test this condition by seeing if we can change the QMPT bit.
	 */
	if (ddp->features & HAVE_MBDISINREG)
	{
		unsigned char data, data2, data3;

		data = dpci_io_readbyte(ddp, DPCI_REG1);
		dpci_io_writebyte(ddp, DPCI_REG1, data ^ DPCI_REG1_QMPT);
		data2 = dpci_io_readbyte(ddp, DPCI_REG1);
		dpci_io_writebyte(ddp, DPCI_REG1, data);
		data3 = dpci_io_readbyte(ddp, DPCI_REG1);
		if ((((data ^ data3) & DPCI_REG1_QMPT) != 0) ||
			(((data ^ data2) & DPCI_REG1_QMPT) != DPCI_REG1_QMPT))
		{
			PRINT_DBG("data=%02x data2=%02x data3=%02x\n",
					data, data2, data3);
			ddp->disabled = 1;
			PRINT_ERR("It appears the IDLP is not working and thus the remaining gaming I/O functions are disabled.\n");
			disable_idlp = 1;
		}
	}

	/*
	 * Check for a board with only 1 ROM socket.  Patch up the ROM size
	 * in that case.
	 */
	if (ddp->features & HAVE_1ROMSOCKET)
	{
		ddp->rom_size = 0x100000;
	}

	/*
	 * Check for the SRAM size.  The PCIe core BARs are not modifiable so
	 * the SRAM size is represented in bits 18-19 of register 2.
	 */
	{
		unsigned char bits = dpci_reg_get(ddp, 2);

		switch (bits & (DPCI_REG2_SRAM0 | DPCI_REG2_SRAM1))
		{
		case 0:
			ddp->nv_size = 0x100000;
			break;
		case DPCI_REG2_SRAM0:
			ddp->nv_size = 0x200000;
			break;
		case DPCI_REG2_SRAM1:
			ddp->nv_size = 0x400000;
			break;
		case DPCI_REG2_SRAM0 | DPCI_REG2_SRAM1:
			ddp->nv_size = 0x800000;
			break;
		}
	}

	/*
	 * Initialise the IDLP now in case we need to hack our idea of which
	 * board we're running on.
	 */
	if (!disable_idlp)
	{
		dpci_id_setup(ddp);
	}

	dpci_init_params(ddp);

	/*
	 * If we have quiet mode, then make sure we disable it so that things
	 * work straight away and so that com ports can be found correctly.
	 */
	if (ddp->features & HAVE_QUIETMODE)
	{
		dpci_reg_chg(ddp,
				DPCI_REG1,
				DPCI_REG1_QMPT | DPCI_REG1_QMLV,
				0,
				0);
	}

	dpci_i2c_setup(ddp);
#if defined(USE_DGL_OW_API) || defined(CONFIG_W1)
	dpci_ow_setup(ddp);
#endif
	/* pfd setup - init flag to mark 'no pfd events logged' */
	ddp->pfd_event_logged = 0;
	dpci_event_queue_setup(ddp);
	init_waitqueue_head(&ddp->clrwdt_wqh);

	/*
	 * Exit with zero - all's fine.
	 */
	{
		struct dpci_device **ddpp;

		for (ddpp = &dpci_driver.devices; *ddpp; ddpp = &(*ddpp)->next)
			;
		*ddpp = ddp;
	}
	return 0;

failed:
	dpci_driver.dev_count--;
clean_quit:	
	if (ddp)
	{
		dpcicore_cleanup_resource(ddp);
	}
	return res;
}


/*******************************************************************************
 *
 * Function:    dpci_remove()
 *
 * Parameters:  dev_to_remove - the PCI device instance we are vacating
 *
 * Returns:     nothing
 *
 * Description: This is called when the PCI asks us to discontinue handling the
 *		device in question (dev_to_remove).  Usually this happens only
 *		in response to the device going away physically (hotplugging)
 *		or because the module is requested to be unloaded.
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static void __devexit dpci_remove(struct pci_dev *dev_to_remove)
#else
static void dpci_remove(struct pci_dev *dev_to_remove)
#endif
{
	struct dpci_device **ddp_respp, *ddp_next;

	if (!dev_to_remove)
	{
		return;
	}
	PRINT_DBG("request to remove pci_dev %p\n", dev_to_remove);
	for (ddp_respp = &dpci_driver.devices;
		*ddp_respp;
		ddp_respp = &(*ddp_respp)->next)
	{
		if ((*ddp_respp)->pci_dev == dev_to_remove)
		{
			break;
		}
	}
	if (!*ddp_respp)
	{
		PRINT_DBG("device to remove %p not in reosurce list.\n",
			dev_to_remove);
		return;
	}
	ddp_next = (*ddp_respp)->next;
	dpcicore_cleanup_resource(*ddp_respp);
	*ddp_respp = ddp_next;

	PRINT_DBG("Advantech Innocore DirectPCI core driver removed from PCI core.\n");
}


/*******************************************************************************
 *
 * Function:    dpcicore_exit
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Clean up remaining resources used by the module in preparation
 *		for module unloading.
 *
 ******************************************************************************/
static void __exit dpcicore_exit(void)
{
	/*Delete the class*/
	pci_unregister_driver(&dpci_pci_driver);
	dpcicore_cleanup_all_resources();
	dpcicore_cleanup_static();
	PRINT_DBG("Advantech Innocore DirectPCI core driver unloaded.\n");
}


/*******************************************************************************
 *
 * Function:    dpcicore_cleanup_resource
 *
 * Parameters:  dev - the PCI device instance structure we use
 *
 * Returns:     nothing
 *
 * Description: This function performs the module cleanup operation, doing
 *		de-allocation, deregistration and freeing as needed.
 *
 ******************************************************************************/
static void dpcicore_cleanup_resource(struct dpci_device *ddp)
{
	if (!ddp->mainboard)
	{
		return;
	}

#if defined(USE_DGL_OW_API) || defined(CONFIG_W1)
	/*
	 * Clean up one-wire resources.
	 */
	dpci_ow_cleanup(ddp);
#endif

	/*
	 * Clean up the IDLP.
	 */
	if(!disable_idlp)
	{
		dpci_id_cleanup(ddp);
	}

	/*
	 * Free the irq and ensure interrupts are disabled.  Don't bother with
	 * interrupt count registers etc. since we're shutting down for good
	 * with no intention of recovering from this position.  Only clear
	 * those bits that we know the register supports.
	 */
	dpci_reg_chg(ddp, DPCI_REG0, DPCI_REG0_SPISS, DPCI_REG0_IOINTEN, 0);
	if (ddp->features & HAVE_EXINTEN)
		dpci_reg_chg(ddp, DPCI_REG0, 0, DPCI_REG0_EXINTEN, 0);
	dpci_set_ioboard_intcfg(ddp, 0, DPCI_IOBOARD_INTCFG_IO);
	dpci_io_modifyint(ddp, 0, 0, 0xff, 0);
	dpci_event_queue_cleanup(ddp);
	if (ddp->irq_line != 0)
	{
		free_irq(ddp->irq_line, ddp);
#ifdef CONFIG_PCI_MSI
		/*
		 * See if MSI (message-signalled interrupts) are available and the
		 * user hasn't disabled them.  If so disable them to free up the
		 * vector.
		 */
		if (!nomsi)
		{
			pci_disable_msi(ddp->pci_dev);
			pci_clear_master(ddp->pci_dev);
		}
#endif
		ddp->irq_line = 0;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	device_destroy(ddp->driver->dpci_class, ddp->dev);
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
	class_device_destroy(ddp->driver->dpci_class, ddp->dev);
#else
	class_simple_device_remove(ddp->dev);
#endif
	/*
	 * Remove the chardevice and free all resources.
	 */
	if(ddp->address != 0)
	{
		/*
		 * Release I/O board memory if there is any allocated.
		 */
		if (ddp->ioboard)
		{
			release_region(ddp->address +
				ddp->ioboard->db_sys_start,
				ddp->ioboard->db_sys_size);
		}
		release_region(ddp->address +
				ddp->mainboard->db_sys_start,
				ddp->mainboard->db_sys_size);
		ddp->address = 0;
	}
	kfree(ddp);
	ddp = NULL;
	return;
}


/*******************************************************************************
 *
 * Function:    dpcicore_cleanup_all_resources
 *
 * Parameters:  dev - the PCI device instance structure we use
 *
 * Returns:     nothing
 *
 * Description: This function performs the clean up of all resources.  Usually
 *		nothing needs to be done as the pci driver should already have
 *              released everything.  THis is just a cross-check.
 *
 ******************************************************************************/
static void dpcicore_cleanup_all_resources(void)
{
	struct dpci_device *ddp;
	struct dpci_device *ddp_next;

	for (ddp = dpci_driver.devices; ddp; ddp = ddp_next)
	{
		PRINT_ERR("device %04x was not removed by PCI driver\n",
			ddp->board_id->dpci_did);
		ddp_next = ddp;
		dpcicore_cleanup_resource(ddp);
	}
}


/*******************************************************************************
 *
 * Function:    dpcicore_cleanup_static
 *
 * Parameters:  none.
 *
 * Returns:     nothing
 *
 * Description: This function performs static once-only cleanup when the module
 *		is about to be removed.
 *
 ******************************************************************************/
static void dpcicore_cleanup_static(void)
{
	if (dpci_driver.dev)
	{
		unregister_chrdev_region(dpci_driver.dev, DPCI_DEVICE_COUNT);
		dpci_driver.dev = 0;
	}
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
	class_destroy(dpci_driver.dpci_class);
#else
	class_simple_destroy(dpci_driver.dpci_class);
#endif
	return;
}

int dpci_get_nvram_config(struct dpci_device *ddp, resource_size_t *addr, resource_size_t *len)
{
	if (ddp->pci_dev->device != DPCI_DEVICE_CORE_PCIE_ID)
	{
		return -ENODEV;
	}
	*addr = ddp->nv_address;
	*len = ddp->nv_size;
	return 0;
}


int dpci_get_rom_config(struct dpci_device *ddp, resource_size_t *addr, resource_size_t *len)
{
	if (ddp->pci_dev->device != DPCI_DEVICE_CORE_PCIE_ID)
	{
		return -ENODEV;
	}
	if (dpci_io_readbyte(ddp, DPCI_REG2) & DPCI_REG2_PCIE_ROM)
	{
		return -ENODEV;
	}
	*addr = ddp->rom_address;
	*len = ddp->rom_size;
	return 0;
}


int dpci_get_board_features(struct dpci_device *ddp, unsigned int *features_p)
{
	if (!features_p)
	{
		return -EFAULT;
	}
	*features_p = ddp->features;
	return 0;
}


module_init(dpcicore_init);
module_exit(dpcicore_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advantech Innocore");
MODULE_DESCRIPTION("DirectPCI Core Gaming I/O");
MODULE_DEVICE_TABLE(pci, dpci_ids);

MODULE_PARM_DESC(debug, "determines level of debugging output.");
module_param(debug, int, 0);

MODULE_PARM_DESC(disable_idlp, "Disable IDLP if set [Do not use without "
			"prior advice from Advantech-Innocore support]");
module_param(disable_idlp, int, 0);

MODULE_PARM_DESC(use_board_code, "Override DPCI board definition.");
module_param(use_board_code, short, 0);
#ifdef CONFIG_PCI_MSI
MODULE_PARM_DESC(nomsi, "non-zero value disables MSI-based interrupt delivery.");
module_param(nomsi, int, 0);
#endif
EXPORT_SYMBOL(dpci_get_device);
EXPORT_SYMBOL(dpci_ioboard_supported);
EXPORT_SYMBOL(dpci_get_ioboard_id);
EXPORT_SYMBOL(dpci_get_ioboard_rev);
EXPORT_SYMBOL(dpci_get_ioboard_intcfg);
EXPORT_SYMBOL(dpci_set_ioboard_intcfg);
EXPORT_SYMBOL(dpci_get_ioaddress);
EXPORT_SYMBOL(dpci_get_irq);
EXPORT_SYMBOL(dpci_get_uart_offsets);
EXPORT_SYMBOL(dpci_enable_ioboard_intr);
EXPORT_SYMBOL(dpci_disable_ioboard_intr);
EXPORT_SYMBOL(dpci_io_readbyte);
EXPORT_SYMBOL(dpci_io_readword);
EXPORT_SYMBOL(dpci_io_readdword);
EXPORT_SYMBOL(dpci_io_writebyte);
EXPORT_SYMBOL(dpci_io_writeword);
EXPORT_SYMBOL(dpci_io_writedword);
EXPORT_SYMBOL(dpci_get_nvram_config);
EXPORT_SYMBOL(dpci_get_rom_config);
EXPORT_SYMBOL(dpci_get_board_features);

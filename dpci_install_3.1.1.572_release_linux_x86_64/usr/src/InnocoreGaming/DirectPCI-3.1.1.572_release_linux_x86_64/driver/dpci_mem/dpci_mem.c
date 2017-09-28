/******************************************************************************
 *
 * $Id: dpci_mem.c 11906 2015-09-07 15:39:38Z aidan $
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
 * Users' own modifications to this driver are allowed but not supportable.
 *
 * Description:
 * DirectPCI kernel-mode driver for static RAM and expansion ROM feature.
 *
 * To keep this file in sync with the Linux kernel interfaces it uses, the
 * best file in the kernel tree to check is drivers/block/z2ram.c, which is as
 * simple a block-device as there appears to be.
 *
 *****************************************************************************/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/dpci_mem_ioctl.h> 
#include <linux/dpci_multi.h>
#include "dpci_mem_priv.h"

int sram_ro = 0;	// if non-zero then SRAM is read-only!
int no_pci = 0;		// if non-zero then do NOT register the pci driver.
int debug = 0;
static unsigned int board_features = 0;

static int mem_pci_probe(struct pci_dev *, const struct pci_device_id *);
static int mem_core_attach(unsigned short id, const struct mem_board *mem_dev, resource_size_t addr, resource_size_t len);
static void mem_remove(struct pci_dev *dev_to_remove);
static void mem_cleanup_static(void);
static void mem_cleanup_all_resources(void);
static void mem_cleanup_resource(struct mem_device *mem_device);

// There's no need to have the SRAM (let alone the ROM) disks partitioned but
// one customer asked to be able to do it and the change is small compared to
// non-partitionable block devices so we did it anyway.
//
// We allow a maximum of 7 partitions although I'd be surprised if it's ever
// got more than 2.  This means 8 minor devices since the first one is always
// the whole disk itself.
//
#define	MEMDISK_MINORS_PER_DISK	8

/*Declare MEM pci device ids structure array*/
static const struct pci_device_id mem_ids[] = \
{
	{PCI_DEVICE(DENSITRON_VENDOR_ID, SRAM_DEVICE_ID),
		.driver_data=(kernel_ulong_t)&mem_boards[0]},
	{PCI_DEVICE(DENSITRON_VENDOR_ID, SRAM16_DEVICE_ID),
		.driver_data=(kernel_ulong_t)&mem_boards[0]},
	{PCI_DEVICE(DENSITRON_VENDOR_ID, ROM_DEVICE_ID),
		.driver_data=(kernel_ulong_t)&mem_boards[1]},
	{PCI_DEVICE(DENSITRON_VENDOR_ID, ROM2_DEVICE_ID),
		.driver_data=(kernel_ulong_t)&mem_boards[1]},
	{ 0 }
};

MODULE_DEVICE_TABLE(pci, mem_ids);

/*Declare the structure that exposes the functions to the PCI core*/
static struct pci_driver mem_pci_driver =
{
	.name = MEM_MODULE_NAME,
	.id_table = mem_ids,
	.probe = mem_pci_probe,
	.remove = mem_remove,
};

/*Declare the varibale that stores the modules data*/
static struct mem_driver mem_driver;


/*******************************************************************************
 *
 * Function:    mem_blk_request()
 *
 * Parameters:  q - the queue on which to perform the request.
 *
 * Returns:     nothing
 *
 * Description: Perform a read or write block access to or from the MEM.  This
 *              is always just a simple block copy between main memory and MEM.
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
static void mem_blk_request(struct request_queue *q)
#else
static void mem_blk_request(request_queue_t *q)
#endif
{
	struct request *req;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	req = blk_fetch_request(q);
	while (req)
	{
		unsigned long start = blk_rq_pos(req) << MEM_SECTOR_SHIFT;
		unsigned long len = blk_rq_cur_bytes(req);
		int err = 0;
#else
	while ((req = elv_next_request(q)) != NULL)
	{
		unsigned long start = req->sector << MEM_SECTOR_SHIFT;
		unsigned long len = req->current_nr_sectors << MEM_SECTOR_SHIFT;
#endif
		struct mem_device *mdp = (struct mem_device *)q->queuedata;
		void *buffer;

		PRINT_DBG("mem_blk_request: %s %ld bytes at ofs %lx\n",
			rq_data_dir(req) == READ ? "read" : "write",
			len,
			start);

		/*
		 * Check the request isn't beyond the limit of the device.
		 */
		if (start + len > mdp->length)
		{
			PRINT_DBG("mem_blk_request: access invalid.\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
			err = -EIO;
			goto done;
#else
			end_request(req, 0);
			continue;
#endif
		}

		/*
		 * Now perform the access.  Switch on the kind of access it is:
		 * this determines how we get the semaphore granting access to
		 * the real MEM.  rq_data_dir() will panic if the request type
		 * is invalid (not READ or WRITE) so we don't have to do that
		 * here.
		 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0)
		buffer = bio_data(req->bio);
#else
		buffer = req->buffer;
#endif
		if (rq_data_dir(req) == READ)
		{
			memcpy_fromio(buffer, mdp->p_remapAddr + start, len);
		}
		else
		{
			memcpy_toio(mdp->p_remapAddr + start, buffer, len);
		}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
done:
		if (!__blk_end_request_cur(req, err))
			req = blk_fetch_request(q);
#else
		end_request(req, 1);
#endif
	}
}


/*******************************************************************************
 *
 * Function:    mem_dump_resources()
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Lists all the device nodes supported by the driver.
 *
 ******************************************************************************/
static void mem_dump_resources(void)
{
	struct mem_device *mdp;

	PRINT_DBG("Resources list:\n");
	for (mdp = mem_driver.devices; mdp; mdp = mdp->next)
	{
		PRINT_DBG("%s@%p: did=%04x "
			"%08llx..%08llx->%p pcidev=%p min=%d\n",
			mdp->name,
			mdp,
			mdp->device_id,
			mdp->startAddress,
			(mdp->startAddress + mdp->length),
			mdp->p_remapAddr,
			mdp->pci_dev,
			MINOR(mdp->dev));
	}
}

/*******************************************************************************
 *
 * Function:    mem_writeop()
 *
 * Parameters:  file - the file the user is using to access the inode
 *              buf - the buffer from which to obtain data
 *              count - the number of bytes to tranfer
 *              ppos - pointer to integer containing start offset in device
 *
 * Returns:     -errno or number of bytes transferred.
 *
 * Description: Performs a write operation to the MEM device and transfers
 *              the data from the user's buffer.
 *
 ******************************************************************************/
static ssize_t mem_writeop(struct file *file, const char *buf, 
				size_t count, loff_t *ppos) 
{
	loff_t startPos;
	int lsb_count;
	struct mem_device *mdp = file->private_data;
	size_t total_count = 0;
	
	/*Check the the parameters.*/
	if (!file || !buf || !ppos || count < 0)
	{
		return (-EINVAL);
	}

	/*
	 * Check if writes are pertinent to this device.  At present, this
	 * logic works because we cannot get write-ops for ROM devices (where
	 * writable=0 always) and there's no other type of device but SRAM.
	 */
	if (!mdp->mem_dev->writable || sram_ro)
	{
		return -EIO;
	}
	
	startPos=*ppos;
	/*Check that the pointer is within the range.*/
	if (startPos<0 || startPos>=mdp->length)
	{
		return 0;
	}
	
	if (count + startPos > mdp->length)
	{
		count = mdp->length - startPos;
	}
	lsb_count = (4 - (startPos & 0x3)) & 0x3;
	down_write(&mdp->rwsem);
	if (lsb_count)
	{
		if (lsb_count > count)
			lsb_count = count;
		if(copy_from_user(mdp->io_buffer, buf, lsb_count))
		{
			up_write(&mdp->rwsem);
			return -EFAULT;
		}
		memcpy_toio(mdp->p_remapAddr+startPos, mdp->io_buffer, lsb_count);
		count -= lsb_count;
		buf += lsb_count;
		startPos += lsb_count;
		total_count += lsb_count;
	}
	while(count)
	{
		size_t writecount = count;
		if(writecount > MEM_SECTOR_SIZE)
		{
			writecount = MEM_SECTOR_SIZE;
		}
		if(copy_from_user(mdp->io_buffer, buf, writecount))
		{
			up_write(&mdp->rwsem);
			return -EFAULT;
		}
		memcpy_toio(mdp->p_remapAddr+startPos, mdp->io_buffer, writecount);
		count -= writecount;
		buf += writecount;
		startPos += writecount;
		total_count += writecount;
	}
		
	up_write(&mdp->rwsem);
   
	*ppos += total_count;
	return total_count;
}


/*******************************************************************************
 *
 * Function:    mem_readop()
 *
 * Parameters:  file - the file the user is using to access the inode
 *              buf - the buffer into which to place data
 *              count - the number of bytes to tranfer
 *              ppos - pointer to integer containing start offset in device
 *
 * Returns:     -errno or number of bytes transferred.
 *
 * Description: Performs a read operation from the MEM device and transfers
 *              the data to the user's buffer.
 *
 ******************************************************************************/
static ssize_t mem_readop(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	loff_t startPos;
	struct mem_device *mdp = file->private_data;
	int lsb_count;
	size_t total_count = 0;
	
	/*Check the the parameters.*/
	if (!file || !buf || !ppos || count < 0)
	{
		return (-EINVAL);
	}

	startPos = *ppos;
	/*Check that the pointer is within the range.*/
	if (startPos<0 || startPos >= mdp->length)
	{
		return 0;
	}

	if (count + startPos > mdp->length)
	{
		count = mdp->length - startPos;
	}

	lsb_count = (4 - (startPos & 0x3)) & 0x3;

	/*
	 * The alternative to using an exclusive lock is to
	 * implement buffering on a per-file instance.
	 */
	down_write(&mdp->rwsem);
	if (lsb_count)
	{
		if (lsb_count > count)
			lsb_count = count;
		memcpy_fromio(mdp->io_buffer, mdp->p_remapAddr+startPos, lsb_count);
		if(copy_to_user(buf, mdp->io_buffer, lsb_count))
		{
			up_write(&mdp->rwsem);
			return -EFAULT;
		}
		count -= lsb_count;
		buf += lsb_count;
		startPos += lsb_count;
		total_count += lsb_count;
	}

	while(count)
	{
		size_t readcount = count;
		if(readcount > MEM_SECTOR_SIZE)
		{
			readcount = MEM_SECTOR_SIZE;
		}
		
		memcpy_fromio(mdp->io_buffer, mdp->p_remapAddr+startPos, readcount);
		if(copy_to_user(buf, mdp->io_buffer, readcount))
		{
			up_write(&mdp->rwsem);
			return -EFAULT;
		}
		count -= readcount;
		buf += readcount;
		startPos += readcount;
		total_count += readcount;
	}
	up_write(&mdp->rwsem);

	*ppos += total_count;
	return total_count;
}


/*******************************************************************************
 *
 * Function:    mem_lseek()
 *
 * Parameters:  file - the file the user is using to access the inode
 *              newOffset - the requested new offset, relative to *somewhere*
 *              whence - from what point the new offset is relative
 *
 * Returns:     -errno or the newly validated position.
 *
 * Description: takes the user's desired new position in the device's memory
 *		and validates it for the kernel.  It update the file's f_pos
 *		member and returns the new position.
 *
 ******************************************************************************/
static loff_t mem_lseek(struct file *file, loff_t newOffset, int whence) 
{
	loff_t newPos;
	struct mem_device *mdp = file->private_data;
	
	if(!file) return (-EINVAL);

	switch(whence)
	{
	case 0: /*SEEK_SET*/
		newPos=newOffset;
		break;

	case 1: /*SEEK_CUR*/
		newPos = file->f_pos+newOffset;
		break;

	case 2: /*SEEK_END*/
		newPos = mdp->length-1+newOffset;
		break;

	default:
		return (-EINVAL);
	}
		
	if(newPos>mdp->length-1 || newPos<0) return (-ESPIPE);
   
	file->f_pos = newPos;
   //force_successful_syscall_return();-?

	return newPos;
}


/*******************************************************************************
 *
 * Function:    mmap_mem()
 *
 * Parameters:  file - the file the user is using to access the inode
 *              vma - the VM area into which MEM pages are to be mapped.
 *
 * Returns:     -errno or zero.
 *
 * Description: takes the user's desired new position in the device's memory
 *		and validates it for the kernel.  It update the file's f_pos
 *		member and returns the new position.
 *
 ******************************************************************************/
static int mmap_mem(struct file *file, struct vm_area_struct *vma)
{
	struct mem_device *mem_device = file->private_data;
	pgprot_t vm_page_prot = vma->vm_page_prot;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	vma->vm_flags |= VM_RESERVED;
#endif
	if (vma->vm_end - vma->vm_start > mem_device->length)
	{
		return -EINVAL;
	}
	if (vma->vm_pgoff > mem_device->length)
	{
		return -EINVAL;
	}

	/*
	 * Check if writes are pertinent to this device.  Note,
	 * only SRAM and ROM are supported and ROM is never writable.
	 */
	if (!mem_device->mem_dev->writable || sram_ro)
	{
		vma->vm_flags &= ~(VM_WRITE | VM_MAYWRITE);
		pgprot_val(vm_page_prot) &= ~_PAGE_RW;
	}
	PRINT_DBG("%s mmap_mem: vma->vm_start=%lx\n",
		mem_device->name,
		vma->vm_start);
	PRINT_DBG("%s mmap_mem: vma->vm_end=%lx\n",
		mem_device->name,
		vma->vm_end);
	PRINT_DBG("%s mmap_mem: vma->vm_pgoff=%lx\n",
		mem_device->name,
		vma->vm_pgoff);
	PRINT_DBG("%s mmap_mem: vma->vm_page_prot=%lx\n",
		mem_device->name,
		(unsigned long)vm_page_prot.pgprot);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
	if (remap_pfn_range(vma,
			vma->vm_start,
			(mem_device->startAddress >>
						PAGE_SHIFT) + vma->vm_pgoff,
			vma->vm_end-vma->vm_start,
			vm_page_prot))
#else
	if (remap_page_range(vma,
			vma->vm_start,
			mem_device->startAddress +
						(vma->vm_pgoff << PAGE_SHIFT),
			vma->vm_end-vma->vm_start,
			vm_page_prot))
#endif
	{
		return -EAGAIN;
	}
	return 0;
}


/*******************************************************************************
 *
 * Function:    mem_read()
 *
 * Parameters:  offset - where in MEM-space to read
 *              valuep - where to put the read data
 *              type - size of data to access (in bytes)
 *
 * Returns:     -errno for an error, otherwise 0.
 *
 * Description: Perform a read access from the MEM.
 *
 ******************************************************************************/
static int mem_read(struct mem_device *mem_device, unsigned int offset, unsigned int *valuep, int type) 
{
	void *address;
	unsigned int iResult = 0;
	
	if (offset >= mem_device->length)
	{
		return (-EINVAL);
	}
	address = mem_device->p_remapAddr;
	down_read(&mem_device->rwsem);
	switch(type)
	{
	case BYTE:
		*valuep=readb(address + offset);
		break;
	case WORD:
		offset &= ~1;	/* avoid mis-aligned bus accesses */
		*valuep=readw(address + offset);
		break;
	case DWORD:
		offset &= ~3;	/* avoid mis-aligned bus accesses */
		*valuep=readl(address + offset);
		break;
	default:
		iResult=-EINVAL;
	}
	up_read(&mem_device->rwsem);
	PRINT_DBG("MEM_READ:  Base=%p offs=%08x value=%08x size=%d\n",
		mem_device->p_remapAddr,
		(unsigned int)offset,
		(unsigned int)*valuep,
		type);
		
	return iResult;
}


/*******************************************************************************
 *
 * Function:    mem_write()
 *
 * Parameters:  offset - where in MEM-space to write
 *              value - the data to write
 *              type - size of data to access (in bytes)
 *
 * Returns:     -errno for an error, otherwise 0.
 *
 * Description: Perform a read access from the MEM.
 *
 ******************************************************************************/
static int mem_write(struct mem_device *mem_device, unsigned int offset, unsigned int value, char type) 
{
	void *address;
	unsigned int iResult = 0;
	
	if (offset >= mem_device->length)
	{
		return (-EINVAL);
	}
	address = mem_device->p_remapAddr;
	down_write(&mem_device->rwsem);
	switch(type)
	{
	case BYTE:
		writeb(value, address + offset);
		break;
	case WORD:
		offset &= ~1;	/* avoid mis-aligned bus accesses */
		writew(value, address + offset);
		break;
	case DWORD:
		offset &= ~3;	/* avoid mis-aligned bus accesses */
		writel(value, address + offset);
		break;
	default:
		iResult=(-EINVAL);
	}
	up_write(&mem_device->rwsem);
	PRINT_DBG("MEM_WRITE: Base=%p offs=%08x value=%08x size=%d\n",
		mem_device->p_remapAddr,
		(unsigned int)offset,
		(unsigned int)value,
		type);
	
	return iResult;
}


/*******************************************************************************
 *
 * Function:    mem_char_ioctl()
 *
 * Parameters:  inode - the inode upon which the command is requested
 *              file - the file the user is using to access the inode
 *              cmd - the command being requested
 *              arg - additioinal data for the command.
 *
 * Returns:     status and or return data for the command.
 *
 * Description: Performs all i/o control operations specific to the MEM
 *              character device node.
 *
 ******************************************************************************/
#ifndef HAVE_UNLOCKED_IOCTL
static int mem_char_ioctl(
			struct inode *inode,
#else
static long mem_char_ioctl(
#endif
			struct file *file,
			unsigned int cmd,
			unsigned long arg)
{
	struct mem_memloc buf;
	struct mem_device *mem_device = file->private_data;
	int iResult=0;
	
	/*
	 * Check the passed arguments
	 */
	if(!file)
	{
		return (-EINVAL);
	}
	
	switch (cmd)
	{
	case IOCTL_MEM_WRITE_BYTE:
	case IOCTL_MEM_WRITE_WORD:
	case IOCTL_MEM_WRITE_DWORD:
		/*
		 * Check we're open for write-access.
		 */
		if ((file->f_flags & O_ACCMODE) == O_RDONLY)
		{
			return -EACCES;
		}

		/*
		 * Check if writes are pertinent to this device.  Note,
		 * only SRAM and ROM are supported and ROM is never writable.
		 */
		if (!mem_device->mem_dev->writable || sram_ro)
		{
			return -EINVAL;
		}
		/* fall through */

	case IOCTL_MEM_READ_BYTE:
	case IOCTL_MEM_READ_WORD:
	case IOCTL_MEM_READ_DWORD:
		if (copy_from_user(&buf, (struct mem_memloc *)arg, 
						sizeof(struct mem_memloc)))
		{
			return -EFAULT;
		}
		break;
	}
		
	switch(cmd) 
	{
	case IOCTL_MEM_READ_BYTE:
		iResult=mem_read(mem_device, buf.offset, &buf.value, BYTE);
		if (iResult == 0)
		{
			 iResult = copy_to_user((struct mem_memloc*)arg,
							&buf, sizeof(buf));
		}
		break;

	case IOCTL_MEM_READ_WORD:
		iResult=mem_read(mem_device, buf.offset, &buf.value, WORD);
		if (iResult == 0)
		{
			 iResult = copy_to_user((struct mem_memloc*)arg,
							&buf, sizeof(buf));
		}
		break;

	case IOCTL_MEM_READ_DWORD:
		iResult= mem_read(mem_device, buf.offset, &buf.value, DWORD);
		if (iResult == 0)
		{
			 iResult = copy_to_user((struct mem_memloc*)arg,
							&buf, sizeof(buf));
		}
		break;

	case IOCTL_MEM_WRITE_BYTE:
		return mem_write(mem_device, buf.offset,buf.value,BYTE);

	case IOCTL_MEM_WRITE_WORD:
		return mem_write(mem_device, buf.offset,buf.value,WORD);

	case IOCTL_MEM_WRITE_DWORD:
		return mem_write(mem_device, buf.offset,buf.value,DWORD);

	case IOCTL_MEM_SIZE:
		PRINT_DBG("IOCTL_MEM_SIZE size is %llx bytes\n",
			mem_device->length);
		iResult = copy_to_user((void*)arg,
					&mem_device->length,
					sizeof(unsigned long));
		break;

	case IOCTL_MEM_SET_DEBUG:
		{
			int old_debug = debug;

			debug = arg;
			PRINT_INFO("debug level 0x%x -> 0x%x\n", old_debug, debug);
#ifndef DEBUG
			if (debug)
			{
				PRINT_WARN("debug=0x%x but this is not a DEBUG driver: "
						"please load the debug driver if you want debug output.\n", debug);
			}
#endif
			iResult = 0;
		}
		break;

	case IOCTL_MEM_GET_DEBUG:
		iResult = debug;
		PRINT_INFO("debug level = 0x%x\n", debug);
		break;

	default:
		iResult=-ENOTTY;
	}

	/*The operation was successful?*/
	
	return iResult;
}


#ifdef HAVE_COMPAT_IOCTL
/*******************************************************************************
 *
 * Function:    dpci_compatioctl()
 *
 * Parameters:  file - the file the user is using to access the inode
 *		cmd - the command being requested
 *		arg - additioinal data for the command.
 *
 * Returns:     status and or return data for the command.
 *
 * Description: Performs all i/o control operations specific to the DPCI device
 *		node.  This is for 32-bit compatbility only and allows binaries
 *		compiled with 32-bit libdpci.so to work with 64-bit driver.
 *		We fudge the inode parameter to NULL; we can do this safely (I
 *		think) because none of our ioctl handlers yet use this.
 *
 ******************************************************************************/
static long mem_char_compatioctl(struct file *file,
			unsigned int cmd,
			unsigned long arg)
{
#ifndef HAVE_UNLOCKED_IOCTL
	return mem_char_ioctl(NULL, file, cmd, arg);
#else
	return mem_char_ioctl(file, cmd, arg);
#endif
}
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)
/*******************************************************************************
 *
 * Function:    mem_blk_ioctl()
 *
 * Parameters:  inode - the inode upon which the command is requested
 *              file - the file the user is using to access the inode
 *              cmd - the command being requested
 *              arg - additioinal data for the command.
 *
 * Returns:     status and or return data for the command.
 *
 * Description: Performs block-mode i/o control.  For the time being we support
 *              only the HDIO_GETGEO command.
 *
 ******************************************************************************/
static int mem_blk_ioctl(struct inode *inode,
			struct file *file, 
			unsigned int cmd,
			unsigned long arg) 
{
	struct hd_geometry hg;
	struct mem_device *mdp = file->private_data;
	int res = 0;

	printk("mem-res=%p\n", mdp);
	switch(cmd) 
	{
	case HDIO_GETGEO:
		hg.heads = 2;
		hg.sectors = 16;
		hg.cylinders = mdp->length / (MEM_SECTOR_SIZE * 32);
		hg.start = 0;
		if (copy_to_user((void __user *)arg, &hg, sizeof(hg)) != 0)
			return -EFAULT;
		res = 0;
		break;

	default:
		res=-ENOTTY;
	}

	/*The operation was successful?*/
	
	return res;
}
#endif


/*******************************************************************************
 *
 * Function:    mem_blk_getgeo()
 *
 * Parameters:  bdev - containing block device structure from kernel
 *              geo - geometry structure to be filled in.
 *
 * Returns:     0 - always
 *
 * Description: Returns data about the geometry of the disk.  This is for
 *              kernels 2.6.16 and newer.  Older ones use the ioctl HDIO_GETGEO.
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,15)
static int mem_blk_getgeo(struct block_device *bdev, struct hd_geometry *hgp)
{
	struct mem_device *mdp = bdev->bd_disk->private_data;

	hgp->heads = 2;
	hgp->sectors = 16;
	hgp->cylinders = mdp->length / (MEM_SECTOR_SIZE * 32);
	hgp->start = 0;
	return 0;
}
#endif


/*******************************************************************************
 *
 * Function:    mem_blk_open()
 *
 * Parameters:  inode - the inode upon which the open is requested
 *              file - the file the user is using to access the inode
 *
 * Returns:     0 - always
 *
 * Description: handles an open() request for the MEM device node.  We set up
 *              the file's private_data pointer to be to our memory resources
 *              structure.  We use this function for both the block and char
 *              mode devices.
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
static int mem_blk_open(struct block_device *bdev, fmode_t mode)
#else
static int mem_blk_open(struct inode *inode, struct file *file) 
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
	struct mem_device *mdp;

	mdp = inode->i_bdev->bd_disk->private_data;
	file->private_data = (void *)mdp;
#endif
	return 0;
}

/*******************************************************************************
 *
 * Function:    mem_blk_release()
 *
 * Parameters:  inode - the inode upon which the release is requested
 *              file - the file the user is using to access the inode
 *
 * Returns:     0 - always
 *
 * Description: handles an close() request for the DPCI device node.  Nothing
 *              need be done here for the time being.
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static void mem_blk_release(struct gendisk *disk, fmode_t mode)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
static int mem_blk_release(struct gendisk *disk, fmode_t mode)
#else
static int mem_blk_release(struct inode *inode, struct file *file) 
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	return 0;
#endif
}


/*******************************************************************************
 *
 * Function:    mem_char_open()
 *
 * Parameters:  inode - the inode upon which the open is requested
 *              file - the file the user is using to access the inode
 *
 * Returns:     0 - always
 *
 * Description: handles an open() request for the MEM device node.  We set up
 *              the file's private_data pointer to be to our memory resources
 *              structure.  We use this function for both the block and char
 *              mode devices.
 *
 ******************************************************************************/
static int mem_char_open(struct inode *inode, struct file *file) 
{
	struct mem_device *mem_device;

	mem_device = container_of(inode->i_cdev, struct mem_device, cdev);
	file->private_data = (void *)mem_device;
	return 0;
}


/*******************************************************************************
 *
 * Function:    read_pci_params()
 *
 * Parameters:  mem_board - the pci device to read resources for
 *              mem_device - where to put the information obtained.
 *
 * Returns:     0 for success, or -ERRNO for errors.
 *
 * Description: This reads the memory PCI device parameters from the PCI
 *              configuration space.
 *
 ******************************************************************************/


/*******************************************************************************
 *
 * Function:    mem_char_release()
 *
 * Parameters:  inode - the inode upon which the release is requested
 *              file - the file the user is using to access the inode
 *
 * Returns:     0 - always
 *
 * Description: handles an close() request for the DPCI device node.  Nothing
 *              need be done here for the time being.
 *
 ******************************************************************************/
static int mem_char_release(struct inode *inode, struct file *file) 
{
    /* Nothing to do on release, only on module shutdown */
    return 0;
}


static struct file_operations mem_fops = {
	.owner = THIS_MODULE,
#ifndef HAVE_UNLOCKED_IOCTL
	.ioctl = mem_char_ioctl, 
#else
	.unlocked_ioctl = mem_char_ioctl, 
#endif
#ifdef HAVE_COMPAT_IOCTL
	.compat_ioctl = mem_char_compatioctl, 
#endif
	.open = mem_char_open,
	.release = mem_char_release,
	.mmap = mmap_mem,
	.read = mem_readop,
	.llseek = mem_lseek,
	.write = mem_writeop,
};

static struct block_device_operations mem_bops = {
	.owner = THIS_MODULE,
	.open = mem_blk_open,
	.release = mem_blk_release,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	.getgeo = mem_blk_getgeo
#else
	.ioctl = mem_blk_ioctl
#endif
};


static struct mem_device *get_new_mem_device(void)
{
	struct mem_device *mdp;

	mdp = kmalloc(sizeof (*mdp), GFP_KERNEL);
	if (mdp)
	{
		memset(mdp, 0, sizeof(*mdp));
		init_rwsem(&mdp->rwsem);
	}
	return mdp;
}


static int attach_mem_device(struct mem_device *mdp)
{
	int res = 0;

	/*
	 * Request access to the MEM's memory region.
	 */
	if(!request_mem_region(mdp->startAddress,
				mdp->length,
				mdp->name))
	{
		PRINT_ERR("failed to request memory "
			"region %llx-%llx for %s.\n",
			mdp->startAddress,
			(mdp->startAddress + mdp->length),
			mdp->name);
		return (-ENODEV);
	}
	
	/*
	 * Map the IO memory into address space and check it happened.
	 */
	mdp->p_remapAddr = ioremap(mdp->startAddress, mdp->length);
	if (!mdp->p_remapAddr) 
	{
		PRINT_ERR("failed to map MEM into address space.\n");
		kfree(mdp);
		return (-EIO);
	}
	
	/*Initialise supported file operations.*/
	cdev_init(&mdp->cdev, &mem_fops);
	mdp->cdev.owner = THIS_MODULE;
	mdp->cdev.ops = &mem_fops;
	res = cdev_add(&mdp->cdev, mdp->dev, 1);
	if (res < 0)
	{
		PRINT_ERR("failed to add MEM device.\n");
		mdp->cdev.owner = NULL;
		goto done;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	mdp->device = device_create(mem_driver.mem_class,
					NULL,
					mdp->dev,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
					NULL,
#endif
					mdp->name);
	if (!mdp->device)
	{
		PRINT_ERR("failed to add %s to memory class.", mdp->name);
		res = -ENOMEM;
		goto done;
	}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14)
	mdp->class_dev = class_device_create(mem_driver.mem_class,
						NULL,
						mdp->dev,
						&device_to_probe->dev,
						mdp->name);
	if (IS_ERR(mdp->class_dev))
	{
		PRINT_ERR("failed to add MEM device to PCI core.\n");
		res = PTR_ERR(mdp->class_dev);
		goto done;
	}
#endif

	/*
	 * Sort out block device stuff.
	 *
	 * All that needs to be done to support a disk which can be partitioned
	 * is to allocate multiple minor numbers for it using alloc_disk() and
	 * not set the GENHD_FL_SUPPRESS_PARTITION_INFO bit in the flags word.
	 * The block device subsystem handles everything else automagically.
	 */
	spin_lock_init(&mdp->b_spinlock);
	mdp->disk = alloc_disk(MEMDISK_MINORS_PER_DISK);
	if (!mdp->disk)
	{
		PRINT_ERR("failed to allocate disk.\n");
		res = -ENOMEM;
		goto done;
	}
	mdp->rd_queue = blk_init_queue(mem_blk_request, &mdp->b_spinlock);
	if (!mdp->rd_queue)
	{
		PRINT_ERR("failed to allocate queue.\n");
		res = -ENOMEM;
		goto done;
	}

	mdp->io_buffer = kmalloc(MEM_SECTOR_SIZE, GFP_KERNEL);
	if(mdp->io_buffer == (unsigned char*)NULL)
	{
		PRINT_ERR("Failed to allocate device buffer\n");
		res = -ENOMEM;
		goto done;
	}

	mdp->rd_queue->queuedata = mdp;
	mdp->disk->private_data = mdp;
	mdp->disk->major = mem_driver.block_major;
	mdp->disk->minors = MEMDISK_MINORS_PER_DISK;
	mdp->disk->first_minor = MEMDISK_MINORS_PER_DISK * mem_driver.block_minor;
	mem_driver.block_minor++;
	mdp->disk->queue = mdp->rd_queue;
	mdp->disk->fops = &mem_bops;
	snprintf(mdp->disk->disk_name,
			sizeof(mdp->disk->disk_name),
			"%sd%i",
			mdp->mem_dev->name,
			MINOR(mdp->dev));
	set_capacity(mdp->disk, mdp->length >> MEM_SECTOR_SHIFT);
	add_disk(mdp->disk);
	if (!mdp->mem_dev->writable || sram_ro)
	{
		set_disk_ro(mdp->disk, 1);
	}

done:
	if (res == 0)
	{
		mdp->next = mem_driver.devices;
		mem_driver.devices = mdp;
		PRINT_DBG("new device %s (minor %d) "
			"mdp=%p mdp->pci_dev=%p mdp->next=%p\n",
			mdp->name,
			MINOR(mdp->dev),
			mdp,
			mdp->pci_dev,
			mdp->next);
	}
	return res;
}


static int mem_core_attach(unsigned short id,
				const struct mem_board *mem_dev,
				resource_size_t addr,
				resource_size_t len)
{
	int instance, minor;
	struct mem_device *mdp = NULL;
	int res = 0;

	PRINT_DBG("request to attach core memory at 0x%08llx..0x%08llx\n",
		addr, (addr + len));

	mdp = get_new_mem_device();
	if (!mdp)
	{
		PRINT_ERR("failed to alloc memory for core device 0x%08llx..0x%08llx.\n",
			addr, (addr + len));
		return -ENOMEM;
	}
	mdp->mem_dev = (struct mem_board*)mem_dev;
	mdp->device_id = id;
	mdp->pci_dev = NULL;
	minor = mem_driver.sram_count + mem_driver.rom_count;
	mdp->dev = MKDEV(MAJOR(mem_driver.cdev),
				MINOR(mem_driver.cdev) + minor);
	if (mem_dev->writable)
	{
		instance = mem_driver.sram_count++;
	}
	else
	{
		instance = mem_driver.rom_count++;
	}
	snprintf(mdp->name,
		sizeof(mdp->name),
		"%s%d",
		mem_dev->name,
		instance);
	mdp->startAddress = addr;
	mdp->length = len;
	mdp->flags = 0;
	PRINT_DBG("PCI Resource start address: %x\n",
		(unsigned int)mdp->startAddress);
	PRINT_DBG("PCI Resource flags: %x\n", (unsigned int)mdp->flags);
	PRINT_DBG("PCI Resource length: %x\n", (unsigned int)mdp->length);
	if(!mdp->length)
	{
		PRINT_ERR("failed to read PCI resources for %s.\n", mdp->name);
		kfree(mdp);
		return (-EFAULT);
	}

	res = attach_mem_device(mdp);
	if (res != 0)
	{
		mem_cleanup_resource(mdp);
	}

	return res;
}


/*******************************************************************************
 *
 * Function:    mem_pci_probe()
 *
 * Parameters:  device_to_probe - the pci device instance to probe
 *              id_to_probe - the vendor/device details for the device
 *
 * Returns:     int - 0 for success, -ve for error.
 *
 * Description: called by the PCI device manager when the device is discovered,
 *              the probe checks the device is the correct device, enables it
 *              and then tries to set it up.  If this can all be done, the
 *              appropriate device node administrative structures are enabled
 *              and a set up occurs for I/O card devices too.
 *
 *              If any error occurs, then a complete back out occurs and the
 *              driver does not attempt to support any functionality at all.
 *
 ******************************************************************************/
static int mem_pci_probe(struct pci_dev *device_to_probe,
			  const struct pci_device_id *id_to_probe)
{
	int res;
	int instance, minor;
	struct mem_device *mdp;
	struct mem_board *mem_dev;
	unsigned short config_word;

	/*Check the parameters*/
	if (!id_to_probe || !device_to_probe) 
		return (-EINVAL); /*NULL refences*/

	PRINT_DBG("request to probe pci_dev %p: ven=%04x dev=%04x\n",
		device_to_probe,
		id_to_probe->vendor,
                id_to_probe->device);

	/*Check parameter's values*/

	mem_dev = (struct mem_board *)(id_to_probe->driver_data);
	if (!mem_dev)
	{
		PRINT_ERR("device %04x:%04x not supported.\n",
			id_to_probe->vendor, id_to_probe->device);
		return (-EINVAL);
	}

	/*Enable the PCI device*/
	if (pci_enable_device(device_to_probe))
	{
		PRINT_ERR("could not enable MEM device %04x:%04x.\n",
			id_to_probe->vendor, id_to_probe->device);
		return (-EIO);
	}

	/*
	 * Double-check device has been enabled for memory
	 */
	res = pci_read_config_word(device_to_probe, PCI_COMMAND, &config_word);
	if(!res && !(config_word & PCI_COMMAND_MEMORY))
	{
		/*
		 * Enable memory *and* IO 
		 */
		pci_write_config_word(device_to_probe, PCI_COMMAND, config_word |
			(PCI_COMMAND_IO | PCI_COMMAND_MEMORY));
	}

	mdp = get_new_mem_device();
	if (!mdp)
	{
		PRINT_ERR("failed to alloc memory for device %04x:%04x.\n",
			id_to_probe->vendor, id_to_probe->device);
		return -ENOMEM;
	}
	mdp->mem_dev = mem_dev;
	mdp->device_id = id_to_probe->device;
	mdp->pci_dev = device_to_probe;
	minor = mem_driver.sram_count + mem_driver.rom_count;
	mdp->dev = MKDEV(MAJOR(mem_driver.cdev),
				MINOR(mem_driver.cdev) + minor);
	if (mem_dev->writable)
	{
		instance = mem_driver.sram_count++;
	}
	else
	{
		instance = mem_driver.rom_count++;
	}
	snprintf(mdp->name,
		sizeof(mdp->name),
		"%s%d",
		mem_dev->name,
		instance);
	mdp->startAddress = pci_resource_start(device_to_probe, DPCI_SRAMROM_MEMORY_BAR);
	mdp->length = pci_resource_len(device_to_probe, DPCI_SRAMROM_MEMORY_BAR);
	mdp->flags = pci_resource_flags(device_to_probe, DPCI_SRAMROM_MEMORY_BAR);
	PRINT_DBG("PCI Resource start address: %x\n",
		(unsigned int)mdp->startAddress);
	PRINT_DBG("PCI Resource flags: %x\n", (unsigned int)mdp->flags);
	PRINT_DBG("PCI Resource length: %x\n", (unsigned int)mdp->length);
	if(!(mdp->flags & IORESOURCE_MEM))
	{
		PRINT_ERR("resource region is not memory.\n");
		kfree(mdp);
		return (-EIO);
	}
	if(!mdp->length)
	{
		PRINT_ERR("failed to read PCI resources for %s.\n", mdp->name);
		kfree(mdp);
		return (-EFAULT);
	}

	/*
	 * This is a hack to bring ROM device down to a reasonable size on early
	 * DPCI CPLD downloads.
	 */
	if (id_to_probe->device == 0x0103)
	{
		mdp->length = 0x200000;
	}
	else if (id_to_probe->device == 0x0104 && (board_features & HAVE_1ROMSOCKET))
	{
		mdp->length = 0x100000;
	}

	res = attach_mem_device(mdp);
	if (res != 0)
	{
		mem_cleanup_resource(mdp);
	}

	return res;
}


/*******************************************************************************
 *
 * Function:    mem_remove()
 *
 * Parameters:  dev_to_remove - the PCI device instance we are vacating
 *
 * Returns:     nothing
 *
 * Description: This is called when the PCI asks us to discontinue handling the
 *              device in question (dev_to_remove).  Usually this happens only
 *              in response to the device going away physically (hotplugging)
 *              or because the module is requested to be unloaded.
 *
 ******************************************************************************/
static void mem_remove(struct pci_dev *dev_to_remove)
{
	struct mem_device **mdppp, *mem_next;

	if (!dev_to_remove)
	{
		return;
	}

	PRINT_DBG("request to remove pci_dev %p\n", dev_to_remove);
	mem_dump_resources();

	for (mdppp = &mem_driver.devices;
					*mdppp;
					mdppp = &(*mdppp)->next)
	{
		if ((*mdppp)->pci_dev == dev_to_remove)
		{
			break;
		}
	}
	if (!*mdppp)
	{
		PRINT_ERR("device to remove %p not in resource list.\n",
			dev_to_remove);
		mem_dump_resources();
		return;
	}
	PRINT_DBG("mem_remove: pci_dev %p is %s\n",
		dev_to_remove,
		(*mdppp)->name);
	mem_next = (*mdppp)->next;
	mem_cleanup_resource(*mdppp);
	*mdppp = mem_next;
}


/*******************************************************************************
 *
 * Function:    mem_init_module()
 *
 * Parameters:  none
 *
 * Returns:     int - 0 for success, -ve for error.
 *
 * Description: Initialise the device driver.  The function is called when the
 *              driver is first loaded into memory,  It just registers the
 *              driver with the PCI core and creates a simple class for the
 *              sysfs interface.
 *
 ******************************************************************************/
static int __init mem_init_module(void) 
{
	int res;

	PRINT_INFO("Advantech Innocore DirectPCI memory driver "
		"v%s $Revision: 11906 $\n",
		DPCI_VERSION); 
	PRINT_INFO("Advantech Innocore DirectPCI memory driver "
		"compiled " B__DATE__ ", " B__TIME__ "\n");
	PRINT_INFO("(C) 2005-2015, Advantech Co Ltd.\n");
	PRINT_DBG("debug=%d\n", debug);
#ifndef DEBUG
	if (debug)
	{
		PRINT_WARN("debug=0x%x but this is not a DEBUG driver: "
				"please load the debug driver if you want debug output.\n", debug);
	}
#endif
	if (sram_ro)
		PRINT_INFO("SRAM in read-only mode!\n");

	memset(&mem_driver, 0, sizeof(mem_driver));

	/*
	 * Allocate block device major number.
	 */
	mem_driver.block_major = register_blkdev(0, MEM_DEVICE_NAME);
	if (mem_driver.block_major < 0)
	{
		PRINT_ERR("couldn't obtain block major and minor numbers.\n");
		res = -ENOMEM;
		goto done;
	}

	res = alloc_chrdev_region(&mem_driver.cdev,
					0,
					MEM_DEVICE_COUNT,
					MEM_DEVICE_NAME);
	if (res < 0)
	{
		PRINT_ERR("couldn't obtain char major and minor numbers.\n");
		res = -ENOMEM;
		goto done;
	}

	/*Register the MEM module so it appeares in sysfs*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
	mem_driver.mem_class = class_create(THIS_MODULE, MEM_MODULE_NAME);
#else
	mem_driver.mem_class = class_simple_create(THIS_MODULE,
							MEM_MODULE_NAME);
#endif
	if(IS_ERR(mem_driver.mem_class))
	{
		PRINT_ERR("failed to create class.\n");
		unregister_chrdev_region(mem_driver.cdev,
						MEM_DEVICE_COUNT);
		res = PTR_ERR(mem_driver.mem_class);
		goto done;
	}

	/*
	 * If no PCI devices we're found then we look for devices where the
	 * SRAM/ROM function is lumped into the same PCI device/function as the
	 * core I/O.
	 */
	{
		int board_no = 0;
		int res;

		do {
			struct dpci_device *ddp;
			resource_size_t addr, len;
			
			ddp = dpci_get_device(board_no);
			if (!ddp)
			{
				break;
			}
			if (board_no == 0)
			{
				res = dpci_get_board_features(ddp, &board_features);
				if (res != 0)
				{
					PRINT_ERR("board %d: couldn't get board's features word.\n",
						board_no);
					break;
				}
				PRINT_INFO("board %d: features word %08x.\n",
						board_no,
						board_features);
			}
			res = dpci_get_nvram_config(ddp, &addr, &len);
			if (res == 0)
			{
				PRINT_INFO("board%d has %llu bytes nvram at 0x%08llx\n",
					   board_no, len, addr);
				res = mem_core_attach(0, &mem_boards[0], addr, len);
			}
			res = dpci_get_rom_config(ddp, &addr, &len);
			if (res == 0)
			{
				PRINT_INFO("board%d has %llu bytes ROM at 0x%08llx\n",
					   board_no, len, addr);
				res = mem_core_attach(1, &mem_boards[1], addr, len);
			}
			board_no++;
		} while (res == 0);
	}

	if (!no_pci && mem_driver.devices == NULL)
	{
		res = pci_register_driver(&mem_pci_driver);
		if (res)
		{
			PRINT_ERR("could not load driver.\n");
			goto done;
		}
	}
	else
	{
		no_pci = 1;
		PRINT_INFO("PCI driver registration disabled.\n");
	}

	mem_dump_resources();

done:
	if (res != 0)
	{
		mem_cleanup_static();
	}
	return res;
}


/*******************************************************************************
 *
 * Function:    mem_exit_module
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Clean up remaining resources used by the module in preparation
 *              for module unloading.
 *
 ******************************************************************************/
static void __exit mem_exit_module(void)
{
	/*Delete the MEM class from sysfs*/
	mem_dump_resources();
	if (!no_pci)
	{
		pci_unregister_driver(&mem_pci_driver);
	}
	mem_cleanup_all_resources();
	mem_cleanup_static();
	PRINT_DBG("driver unloaded.\n");
	return;
}


/*******************************************************************************
 *
 * Function:    mem_cleanup_resource
 *
 * Parameters:  mdp - the instance of a MEM device.
 *
 * Returns:     nothing
 *
 * Description: This function cleans up a single resource linked in to the
 *              module from the pci_probe stage.
 *
 ******************************************************************************/
static void mem_cleanup_resource(struct mem_device *mdp)
{
	if (mdp->startAddress)
	{
		if (mdp->disk)
		{
			del_gendisk(mdp->disk);
			put_disk(mdp->disk);
		}
		if (mdp->rd_queue)
		{
			blk_cleanup_queue(mdp->rd_queue);
		}

		/*Unmap the IO address space from the virtual memory*/
		if (mdp->p_remapAddr) 
		{
			iounmap(mdp->p_remapAddr);
			mdp->p_remapAddr=NULL;
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
		device_destroy(mem_driver.mem_class, mdp->dev);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14)
		class_device_destroy(mem_driver.mem_class, mdp->dev);
#endif
		if (mdp->cdev.owner != NULL)
		{
			cdev_del(&mdp->cdev);
			mdp->cdev.owner = NULL;
		}

		release_mem_region(mdp->startAddress, mdp->length);

	}
	if(mdp->io_buffer)
	{
		kfree(mdp->io_buffer);
		mdp->io_buffer = NULL;
	}
	kfree(mdp);
	return;
}


/*******************************************************************************
 *
 * Function:    mem_cleanup_all_resources
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: This function performs the module cleanup operation, doing
 *              de-allocation, deregistration and freeing as needed.
 *
 ******************************************************************************/
static void mem_cleanup_all_resources(void) 
{
	struct mem_device *mdp;
	struct mem_device *mdp_next;

	for (mdp = mem_driver.devices; mdp; mdp = mdp_next)
	{
		/*
		 * Our PCI device IDs start at 0x101 (earliest SRAM model) so
		 * anything less we will assume is directly attached to a
		 * memory segment made available via dpci_core.ko.
		 */
		if (mdp->device_id >= 0x100)
		{
			PRINT_ERR("device %s (id %04x) was not removed by PCI driver\n",
				mdp->name,
				mdp->device_id);
		}
		mdp_next = mdp->next;
		mem_cleanup_resource(mdp);
	}
	return;
}


/*******************************************************************************
 *
 * Function:    mem_cleanup_static
 *
 * Parameters:  none
 *
 * Returns:     nothing
 *
 * Description: Clean up general driver state data which isn't instance
 *		specific.
 *
 ******************************************************************************/
static void mem_cleanup_static(void)
{
	if (MAJOR(mem_driver.cdev) > 0)
	{
		unregister_chrdev_region(mem_driver.cdev,
						MEM_DEVICE_COUNT);
	}
	if (mem_driver.block_major > 0)
	{
		unregister_blkdev(mem_driver.block_major, MEM_DEVICE_NAME);
	}
	if (mem_driver.mem_class)
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
		class_destroy(mem_driver.mem_class);
#else
		class_simple_destroy(mem_driver.mem_class);
#endif
	}
}


module_init(mem_init_module);
module_exit(mem_exit_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advantech Innocore");
MODULE_DESCRIPTION("DirectPCI memory bus driver");
MODULE_PARM_DESC(debug, "determines level of debugging output.");
module_param(debug, int, 0);
MODULE_PARM_DESC(sram_ro, "Setting to non-zero makes SRAM read-only.");
MODULE_PARM_DESC(no_pci, "Setting to non-zero disables registering the PCI driver.");
module_param(sram_ro, int, 0);
module_param(no_pci, int, 0);

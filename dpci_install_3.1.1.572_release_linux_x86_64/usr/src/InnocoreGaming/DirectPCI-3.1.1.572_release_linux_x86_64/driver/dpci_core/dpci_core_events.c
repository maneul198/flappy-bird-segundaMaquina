/******************************************************************************
 *
 * $Id: dpci_core_events.c 11939 2015-09-14 22:31:13Z aidan $
 *
 * Copyright 2010-2011 Advantech Corporation Limited.
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
 * DirectPCI core events module for kernel-mode
 *
 *****************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/rtc.h>
#include <linux/time.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

/*include the file that defines the constants for the dpci-core driver.*/

#include <linux/dpci_core_ioctl.h>
#include "dpci_core_hw.h"
#include "dpci_core_priv.h"


/*******************************************************************************
 *
 * Function:    user_queues_thread_func()
 *
 * Parameters:  args - arguments to thread function.
 *
 * Returns:     int - status 0 for success;
 *
 * Description: Handler function for user queues processing thread
 *
 ******************************************************************************/
static int user_queues_thread_func(void *args)
{
	struct dpci_device *ddp = (struct dpci_device *)args;
	int ret = 0;
	struct dpci_event new_op_event;
	struct dpci_event op_event;
	struct dio_event dio_data;
	struct event_request *current_er;
	unsigned long flags;
	unsigned long change_mask;
	
	if (!ddp)
	{
		/*
		 * Terminate thread
		 */
		do_exit(ret);
	}

	ddp->uq_thread_signal = 0;
	
	while (!kthread_should_stop())
	{
		if (ddp->global_event_list.buf_head == -1)
		{
			/*
			 * No more events in global queue; goto sleep
			 */
			ddp->uq_thread_signal = 0;
			ret = wait_event_interruptible_timeout(
				ddp->uq_thread_wqh,
				ddp->uq_thread_signal,
				msecs_to_jiffies(WAIT_EVENT_THREAD_TIMEOUT_MS));	
			if (ret < 0)
			{
				/*
				 * Interrupted by signal
				 */
				do_exit(ret);
			}
			continue;
		}
		/*
		 * Pick the oldest event from the global_event_queue
		 * Check the event type against user event req threads
		 * Copy the event to the user event queue of each matching 
		 * event request type.
		 */
		spin_lock_irqsave(&ddp->global_event_list_lock, flags);
		op_event_list_remove(&ddp->global_event_list, &new_op_event);
		spin_unlock_irqrestore(&ddp->global_event_list_lock, flags);
		current_er = ddp->er_queue_head;
		while (current_er != NULL)
		{
			op_event = new_op_event;
			if ((current_er->er_state < ENABLED) ||
			(!(current_er->er_type_mask & op_event.de_type)))
			{
				goto next_item;
			}
			if (op_event.de_type == EVENT_DIG_IP)
			{
				dio_data = op_event.de_data.dio_event;
				dio_data.change_mask = 
					dio_data.change_mask & 
						current_er->er_param.int_mask;
				if (!dio_data.change_mask)
				{
					goto next_item;
				}
				/*
				 * Report only events on the edge the user
				 * thread is waiting on. 
				 * Filter the change mask based on edge_state 
				 * first and then based on auto_config_edge
				 */
				change_mask = ~(dio_data.dio_value ^ 
					current_er->er_param.edge_state);
				change_mask = change_mask | 
					current_er->er_param.autoconfig_edge;
				dio_data.change_mask = 
					dio_data.change_mask & 
						change_mask;
				if (!dio_data.change_mask)
				{
					goto next_item;
				}
				op_event.de_data.dio_event = dio_data;
			}
			/*
			 * Update timestamps
			 */
			op_event.de_ts_delta = 
				op_event.de_ts - current_er->last_ts;
			current_er->last_ts = op_event.de_ts;
			/*
			 * Add event to user queue
			 */
			spin_lock_irqsave(&current_er->event_list_lock, flags);
			if (op_event_list_add(&current_er->event_list, 
						op_event) == 0)
			{
				current_er->num_events++;
				if (!current_er->cont_wait)
					current_er->er_state = SIGNALLED;
				current_er->er_signal = SIGNAL_EVENT;
				if (waitqueue_active(&current_er->er_wqh))
					wake_up_interruptible(
						&current_er->er_wqh);
			}
			spin_unlock_irqrestore(&current_er->event_list_lock, 
						flags);
next_item:
			current_er = current_er->next_er;
		}
		/*
		 * Enable the interrupt flag corresponding to this event
		 */	
		unset_global_event_queue_flags(ddp, new_op_event);
	}
	ddp->uq_stopped = 1;
	return 0;
}


/*******************************************************************************
 *
 * Function:    collate_event_requests()
 *
 * Parameters:  ddp - dpci device pointer
 *
 * Returns:     nothing
 *
 * Description: goes through all event requests and collates them into a single
 *              request allowing us to determine what interrupts need to be
 *              enabled to satisfy all event consumers.
 *
 ******************************************************************************/
static void collate_event_requests(struct dpci_device *ddp)
{
	struct event_request *current_er;
	unsigned long dio_value = 0;
	unsigned long edge_state = 0;
	unsigned long flags;
	int port;
	u8 ip;
	int event_flag;

	/*
	 * Collate all event requests into a single request
	 * Enable all required interrupts
	 */

	do
	{
		spin_lock_irqsave(&ddp->intr_enable_lock, flags);

		event_flag = ddp->event_flag;
		
		// Release lock to permit progress
		if (event_flag == 0)
		{
			spin_unlock_irqrestore(&ddp->intr_enable_lock, flags);
		}
	} while(event_flag == 0);

	ddp->thread_event.er_type_mask = 0;
	ddp->thread_event.er_param.int_mask = 0;
	ddp->thread_event.er_param.edge_state = 0;
	ddp->thread_event.er_param.autoconfig_edge = 0;
	current_er = ddp->er_queue_head;
	ddp->thread_event.er_signal = SIGNAL_NONE;
	spin_lock_irqsave(&ddp->event_queue_lock, flags);
	while (current_er != NULL)
	{
		if (current_er->er_state <= ENABLED)
		{
			current_er->er_state = ENABLED;
			ddp->thread_event.er_type_mask |= 
					current_er->er_type_mask;
			if (current_er->er_type_mask & EVENT_DIG_IP)
			{
				ddp->thread_event.er_param.int_mask |= 
					current_er->er_param.int_mask;
				ddp->thread_event.er_param.edge_state |= 
					current_er->er_param.edge_state;
				ddp->thread_event.er_param.autoconfig_edge |=
					current_er->er_param.autoconfig_edge;
			}
		}
		current_er = current_er->next_er;
	}
	spin_unlock_irqrestore(&ddp->event_queue_lock, flags);
	//printk(KERN_INFO "im=%08lx es=%08lx ac=%08lx\n", ddp->thread_event.er_param.int_mask, ddp->thread_event.er_param.edge_state, ddp->thread_event.er_param.autoconfig_edge);

	/* 
	 * For inputs that are to be autoconfigured for both rising and
	 * falling edge interrupts, set the edge based on current value
	 * of the input line, so that the next interrupt occurs on 
	 * alternate state
	 */
	/*
	 * First mask out the edge_state for inputs that do not have 
	 * autoconfig set
	 */
	edge_state = ddp->thread_event.er_param.edge_state & 
		~(ddp->thread_event.er_param.autoconfig_edge);
	dio_value = 0;

	if(ddp->reg_ip != NULL)
	{
		for (port = 0; 
			port < MAX_INPUT_PORTS && ddp->reg_ip_status[port]; 
			port++)
		{
			ip = dpci_io_readbyte(ddp, ddp->reg_ip[port]);
			dio_value |= (ip << (port * 8));
		}
	}

	/*
	 * For inputs with autoconfig set, set the edge_state to next
	 * expected state
	 */
	dio_value = ~(dio_value) & ddp->thread_event.er_param.autoconfig_edge;
	ddp->thread_event.er_param.edge_state = edge_state | dio_value;
}


/*******************************************************************************
 *
 * Function:    er_thread_func()
 *
 * Parameters:  args - arguments to thread function.
 *
 * Returns:     int - status 0 for success;
 *
 * Description: Handler function for event request queueing thread
 *
 ******************************************************************************/
static int er_thread_func(void *args)
{
	struct dpci_device *ddp = (struct dpci_device *)args;
	int ret = 0;
	unsigned long flags = 0;
	struct dpci_event combined_op_event;
	struct dpci_event op_event;
	int wait_for_new_event = 0;

	if (!ddp)
	{
		/*
		 * Terminate thread
		 */
		do_exit(ret);

	}
	ddp->er_thread_signal = 0;
	
	while (!kthread_should_stop())
	{
		if ((ddp->er_queue_head == NULL) || (wait_for_new_event == 1))
		{
			/* 
			 * No more pending event requests 
			 */
			ddp->er_thread_signal = 0;
			ret = wait_event_interruptible_timeout(
				ddp->er_thread_wqh,
				ddp->er_thread_signal,
				msecs_to_jiffies(WAIT_EVENT_THREAD_TIMEOUT_MS));
			if (ret < 0)
			{
				do_exit(ret);
			}
			else if (ret > 0)
			{
				wait_for_new_event = 0;
			}
			continue;
		}

		/*
		 * Collate all event requests into a single request and enable
		 * all required interrupts
		 */
		collate_event_requests(ddp);

		spin_unlock_irqrestore(&ddp->intr_enable_lock, flags);

		/*
		 * This performs IO and should not be called in spinlock
		 */
		er_enable_interrupts(ddp, &ddp->thread_event);
		spin_lock_irqsave(&ddp->intr_enable_lock, flags);

		/*
		 * Wait for events 
		 */
		if (ddp->thread_event.er_type_mask == 0)
		{
			wait_for_new_event = 1;
			spin_unlock_irqrestore(&ddp->intr_enable_lock, flags);
			continue;
		}
		spin_unlock_irqrestore(&ddp->intr_enable_lock, flags);
		ret = wait_event_interruptible_timeout(
				ddp->thread_event.er_wqh, 
				ddp->thread_event.er_signal,
				msecs_to_jiffies(WAIT_EVENT_THREAD_TIMEOUT_MS));
		if (ret < 0)
		{
			/*
			 * Interrupted by signal
			 */
			do_exit(ret);
		}

		if (ddp->thread_event.er_signal != SIGNAL_EVENT)
		{
			ip_irq_handler(ddp);
		}

		/*
		 * Disable all interrupts previously enabled.
		 * Does IO and must be called prior to obtaining lock
		 */
		ret = er_disable_interrupts(ddp, 
				ddp->thread_event.er_type_mask);

		spin_lock_irqsave(&ddp->intr_enable_lock, flags);

		/*
		 * Event Occured
		 */		
		if (ddp->thread_event.er_signal != SIGNAL_EVENT)
		{
			spin_unlock_irqrestore(&ddp->intr_enable_lock, flags);
			continue;
		}
		ddp->thread_event.er_signal = SIGNAL_NONE;
		combined_op_event = ddp->thread_event.op_event;
		set_global_event_queue_flags(ddp, combined_op_event);
		
		get_timestamp(&combined_op_event.de_ts);
		spin_unlock_irqrestore(&ddp->intr_enable_lock, flags);
		
		/*
		 * If it is an IDLP event, read event from event log
		 */
		if (combined_op_event.de_type & EVENT_IDLP)
		{
			op_event = combined_op_event;
			op_event.de_type = EVENT_IDLP;
			dpci_id_readevent(ddp, &op_event.de_data.idlp_event);
		
			/*
			 * Add this new event to global event queue first
			 */
			spin_lock_irqsave(&ddp->global_event_list_lock, flags);
			op_event_list_add(&ddp->global_event_list, op_event);
			spin_unlock_irqrestore(&ddp->global_event_list_lock, flags);
		}
		/*
		 * If it is a Digital i/p event, read digital input lines and
		 * change_mask
		 */
		if (combined_op_event.de_type & EVENT_DIG_IP)
		{
			op_event = combined_op_event;
			op_event.de_type = EVENT_DIG_IP;
			if (op_event.de_data.dio_event.change_mask != 0)
			{
				/*
				 * Add this new event to global event queue
				 */
		
				spin_lock_irqsave(&ddp->global_event_list_lock,
						 flags);
				op_event_list_add(&ddp->global_event_list, 
						op_event);
				spin_unlock_irqrestore(
					&ddp->global_event_list_lock, flags);
			}
		}
		if (combined_op_event.de_type & EVENT_PFD)
		{
			op_event = combined_op_event;
			op_event.de_type = EVENT_PFD;
			/*
			 * A PFD event has been logged; remember this so we 
			 * don't enable PFD interrupt again.
			 */
			ddp->pfd_event_logged = 1;
			/*
			 * Add this new event to global event queue first
			 */
			spin_lock_irqsave(&ddp->global_event_list_lock, flags);
			op_event_list_add(&ddp->global_event_list, op_event);
			spin_unlock_irqrestore(&ddp->global_event_list_lock, 
						flags);
		}
		if (combined_op_event.de_type & EVENT_TS)
		{
			op_event = combined_op_event;
			op_event.de_type = EVENT_TS;
			/*
			 * Add this new event to global event queue first
			 */
			spin_lock_irqsave(&ddp->global_event_list_lock, flags);
			op_event_list_add(&ddp->global_event_list, op_event);
			spin_unlock_irqrestore(&ddp->global_event_list_lock, 
						flags);
		}
		if (waitqueue_active(&ddp->uq_thread_wqh))
		{
			ddp->uq_thread_signal = 1;
			wake_up_interruptible(&ddp->uq_thread_wqh);
		}
	}
	ddp->er_stopped = 1;
	
	return 0;
}


/*******************************************************************************
 *
 * Function:    get_timestamp()
 *
 * Parameters:  tsp - pointer to a dpci_timestamp_t
 *
 * Returns:     none
 *
 * Description: Updates the timestamp for a dpci_event.
 *
 ******************************************************************************/
void get_timestamp(dpci_timestamp_t *tsp)
{
	struct timeval timestamp;
	
	do_gettimeofday(&timestamp);
	*tsp = ((unsigned long long)timestamp.tv_sec * 1000) + (timestamp.tv_usec / 1000);
}


/*******************************************************************************
 *
 * Function:    dpci_event_queue_setup()
 *
 * Parameters:  ddp - the device instance.
 *
 * Returns:     none
 *
 * Description: Initialisation routine for event queue pointers and threads.
 *
 ******************************************************************************/
void dpci_event_queue_setup(struct dpci_device *ddp)
{
	struct dpci_device *args = ddp;
	ddp->er_queue_head = NULL;
	ddp->er_queue_tail = NULL;
	ddp->er_queue_len = 0;	
	spin_lock_init(&ddp->event_queue_lock);
	spin_lock_init(&ddp->intr_enable_lock);
	ddp->event_flag = 1;
	ddp->global_event_list.buf_head = -1; 
	ddp->global_event_list.buf_tail = -1;
	spin_lock_init(&ddp->global_event_list_lock);
	spin_lock_init(&ddp->global_queue_flags_lock);

	/*
	 * Start event requests thread
	 */
	ddp->er_thread_signal = 0;
	init_waitqueue_head(&ddp->er_thread_wqh);
	init_waitqueue_head(&ddp->thread_event.er_wqh);
	ddp->er_thread = kthread_run(er_thread_func, 
				(void *)args, 
				"dpci_core:evreq_mgmt");

	/*
	 * Start user-threads queue processor thread
	 */
	ddp->uq_thread_signal = 0;
	init_waitqueue_head(&ddp->uq_thread_wqh);
	ddp->uq_thread = kthread_run(user_queues_thread_func, 
				(void *)args, 
				"dpci_core:userq_dispatch");
}


/*******************************************************************************
 *
 * Function:    dpci_event_queue_cleanup()
 *
 * Parameters:  ddp - the device instance.
 *
 * Returns:     none
 *
 * Description: Cleanup routine for event queue pointers and threads.
 *
 ******************************************************************************/
void dpci_event_queue_cleanup(struct dpci_device *ddp)
{
	struct event_request *current_er;
	struct event_request *free_er;

	/*
	 * Stop er_thread and wait for it to complete
	 */
	ddp->er_stopped = 0;
	kthread_stop(ddp->er_thread);
	while (!ddp->er_stopped);
	/*
	 * Stop uq_thread and wait for it to complete
	 */
	ddp->uq_stopped = 0;
	kthread_stop(ddp->uq_thread);
	while (!ddp->uq_stopped);
	/*
	 * Free all event request pointers
	 */
	current_er = ddp->er_queue_head;
	while (current_er != NULL)
	{
		free_er = current_er;
		current_er = current_er->next_er;
		kfree(free_er);
	}
	/*
	 * Queue is empty. Reset queue pointers
	 */
	ddp->er_queue_head = NULL;
	ddp->er_queue_tail = NULL;
}


/*******************************************************************************
 *
 * Function:    op_event_list_add()
 *
 * Parameters:  plist - pointer to the list to be updated
 *		opevent - output event to be added to the list.
 *
 * Returns:     0 = success; -1 = error on buffer insert 
 *
 * Description: Adds an element to the output events list buffer. Used for both 
 *		global event queue and individual user thread event queues.
 *
 ******************************************************************************/
int op_event_list_add(struct op_event_list *plist, struct dpci_event opevent)
{
	int event_buf_len;
	event_buf_len = plist->buf_tail - plist->buf_head;
	if (event_buf_len < 0)
		event_buf_len += MAX_OUTPUT_QUEUE_SIZE;
	event_buf_len += 1;
	if (event_buf_len == MAX_OUTPUT_QUEUE_SIZE)
	{
		/*
		 * Event buf is full
		 */
		return -1;
	}
	plist->buf_tail++;
	if (plist->buf_tail == MAX_OUTPUT_QUEUE_SIZE)
		plist->buf_tail = 0;
	/*
	 * In case of event over-write, increment the read pointer to point to
	 * the oldest event
	 */
	if (plist->buf_head == plist->buf_tail)
	{
		plist->buf_head++;
		if (plist->buf_head == MAX_OUTPUT_QUEUE_SIZE)
			plist->buf_head = 0;
	}
	/*
	 * If this is the first event in this list, update read pointer
	 */
	if (plist->buf_head == -1)
		plist->buf_head = 0;
	plist->buf[plist->buf_tail] = opevent;
	return 0;
}


/*******************************************************************************
 *
 * Function:    op_event_list_remove()
 *
 * Parameters:  plist - pointer to the event list to be updated
 *		pevent - buffer to hold the output event removed from the list.
 *
 * Returns:     0 = success; -1 = error on queue remove
 *
 * Description: Removes and returns an element from the output events list. 
 *		Used for both global event queue and individual user thread 
 *		event queues.
 *
 ******************************************************************************/
int op_event_list_remove(struct op_event_list *plist, struct dpci_event *pevent)
{
	if (plist->buf_head == -1)
	{
		/*
		 * event buf is empty
		 */
		return -1;
	}
	*pevent = plist->buf[plist->buf_head];
	if (plist->buf_head == plist->buf_tail)
	{
		/*
		 * This was the only event in the buffer. Set new buffer 
		 * pointers to mark buffer empty.
		 */
		plist->buf_head = -1;
		plist->buf_tail = -1;
	}
	else
	{
		plist->buf_head++;
		if (plist->buf_head == MAX_OUTPUT_QUEUE_SIZE)
			plist->buf_head = 0;
	}
	return 0;
}
#if 0
/*******************************************************************************
 *
 * Function:    op_event_queue_add()
 *
 * Parameters:  op_queue_head - Pointer to first element in the queue
 * 		op_queue_tail - Pointer to last element in the queue
 *		opevent - output event to be added to the queue.
 *
 * Returns:     0 = success; -1 = error on queue insert 
 *
 * Description: Adds an element to the output events queue. Used for both 
 *		global event queue and individual user thread event queue.
 *
 ******************************************************************************/
int op_event_queue_add(struct op_event_elem *op_queue_head,
			struct op_event_elem *op_queue_tail, 
			struct dpci_event opevent)
{
	struct op_event_elem *new_elem = kmalloc(sizeof(struct op_event_elem), 
						GFP_KERNEL);
	if (new_elem == NULL)
		return -ENOMEM;
	new_elem->opevent = opevent;
	new_elem->prev = op_queue_tail;
	new_elem->next = NULL;
	if (op_queue_head == NULL)
		op_queue_head = new_elem;
	op_queue_tail = new_elem;
	return 0;
}


/*******************************************************************************
 *
 * Function:    op_event_queue_remove()
 *
 * Parameters:  op_queue_head - Pointer to first element in the queue
 * 		op_queue_tail - Pointer to last element in the queue
 *		pevent - buffer to hold output event deleted from the queue.
 *
 * Returns:     0 = success; -1 = queue empty. 
 *
 * Description: Removes an element from the output events queue. The deleted
 *		record is returned to the caller.
 *
 ******************************************************************************/
int op_event_queue_remove(struct op_event_elem *op_queue_head,
			struct op_event_elem *op_queue_tail, 
			struct dpci_event *pevent)
{
	struct op_event_elem *rem_elem;
	if (!pevent)
		return -EINVAL;
	if (op_queue_head == NULL)
		return -EINVAL;
	rem_elem = op_queue_head;
	*pevent = rem_elem->opevent;
	op_queue_head = rem_elem->next;
	if (op_queue_head == NULL)
		op_queue_tail = NULL;
	kfree(rem_elem);
	return 0;	
}
#endif

/*******************************************************************************
 *
 * Function:    set_global_event_queue_flags()
 *
 * Parameters:  ddp - the device instance
 *		opevent - output event.
 *
 * Returns:     none. 
 *
 * Description: Sets a flag for the type of interrupt logged onto the global 
 *		queue. This flag is cleared after the event is copied to all
 *		event queues of user threads on this event.
 *
 ******************************************************************************/
void set_global_event_queue_flags(struct dpci_device *ddp, struct dpci_event opevent)
{
	unsigned long change_mask;
	unsigned long flags;
	
	spin_lock_irqsave(&ddp->global_queue_flags_lock, flags);
	if (opevent.de_type & EVENT_IDLP)
	{
		ddp->global_queue_flags.idlp_flag = 1;
		ddp->thread_event.op_event.de_type &= ~EVENT_IDLP;
	}
	if (opevent.de_type & EVENT_PFD)
	{
		ddp->global_queue_flags.pfd_flag = 1;
		ddp->thread_event.op_event.de_type &= ~EVENT_PFD;
	}
	if (opevent.de_type & EVENT_TS)
	{
		ddp->global_queue_flags.ts_flag = 1;
		ddp->thread_event.op_event.de_type &= ~EVENT_TS;
	}
	if (opevent.de_type & EVENT_DIG_IP)
	{
		change_mask = opevent.de_data.dio_event.change_mask;	
		ddp->global_queue_flags.digip_flags |= change_mask ;
		ddp->thread_event.op_event.de_data.dio_event.change_mask &= ~change_mask;
	}
	update_er_thread_event(ddp);
	spin_unlock_irqrestore(&ddp->global_queue_flags_lock, flags);
}


/*******************************************************************************
 *
 * Function:    unset_global_event_queue_flags()
 *
 * Parameters:  ddp - the device instance
 *		opevent - output event.
 *
 * Returns:     none. 
 *
 * Description: Unsets a flag for the type of interrupt removed from the global 
 *		queue. This flag is cleared after the event is copied to all
 *		event queues of user threads on this event.
 *
 ******************************************************************************/
void unset_global_event_queue_flags(struct dpci_device *ddp, struct dpci_event opevent)
{
	unsigned long change_mask;
	unsigned long flags;
	
	spin_lock_irqsave(&ddp->global_queue_flags_lock, flags);
	if (opevent.de_type == EVENT_IDLP)
		ddp->global_queue_flags.idlp_flag = 0;
	else if (opevent.de_type == EVENT_PFD)
		ddp->global_queue_flags.pfd_flag = 0;
	else if (opevent.de_type == EVENT_TS)
		ddp->global_queue_flags.ts_flag = 0;
	else if (opevent.de_type == EVENT_DIG_IP)
	{
		change_mask = opevent.de_data.dio_event.change_mask;	
		ddp->global_queue_flags.digip_flags ^= change_mask ;
	}
	update_er_thread_event(ddp);
	spin_unlock_irqrestore(&ddp->global_queue_flags_lock, flags);
}


/*******************************************************************************
 *
 * Function:    update_er_thread_event()
 *
 * Parameters:  ddp - the device instance
 *
 * Returns:     none. 
 *
 * Description: Aborts er_thread wait. This causes the thread_event 
 *		int_mask to be recalculated.
 *
 ******************************************************************************/
void update_er_thread_event(struct dpci_device *ddp)
{
	if (waitqueue_active(&ddp->thread_event.er_wqh))
	{
		ddp->thread_event.er_signal = SIGNAL_PARAMS;
		wake_up_interruptible(&ddp->thread_event.er_wqh);
	}
}


/*******************************************************************************
 *
 * Function:    init_event_request()
 *
 * Parameters:  er_type - the mask of event types to be monitored
 *		er_param - additional info for event types
 *		cont_wait - should wait continue after one event
 *
 * Returns:     pointer to the new event request packet created; NULL otherwise
 *
 * Description: Initialises a new event request.
 *
 ******************************************************************************/
struct event_request *init_event_request(unsigned int er_type, 
					struct digip_param er_param,
					int cont_wait)
{
	struct event_request *pevent = NULL;
	// Create new event request record
	pevent = (struct event_request *)kmalloc(sizeof(struct event_request), 
						GFP_KERNEL);
	if (!pevent)
	{
		PRINT_ERR("Failed to acquire memory for new event request\n");
		return NULL;
	}
	pevent->er_state = NEW;
	pevent->er_type_mask = er_type;
	pevent->er_param = er_param;
	get_timestamp(&pevent->last_ts);
	pevent->next_er = NULL;
	pevent->prev_er = NULL;
	init_waitqueue_head(&pevent->er_wqh);
	pevent->er_signal = SIGNAL_NONE;
	pevent->event_list.buf_head = -1;
	pevent->event_list.buf_tail = -1;
	pevent->num_events = 0;
	pevent->cont_wait = cont_wait;
	spin_lock_init(&pevent->event_list_lock);
	pevent->process_id = current->pid;
	pevent->thread_group = current->group_leader->pid;
	return pevent;
}


/*******************************************************************************
 *
 * Function:    destroy_event_request()
 *
 * Parameters:	pevent - pointer to event request
 *
 * Returns:     none
 *
 * Description: Destroys the event request.
 *
 ******************************************************************************/
void destroy_event_request(struct event_request *pevent)
{
	if (pevent)
		kfree(pevent);
}


/*******************************************************************************
 *
 * Function:    er_enable_interrupts()
 *
 * Parameters:  ddp - device instance
 *		pevent - pointer to event request
 *
 * Returns:     0 on success; error code otherwise
 *
 * Description: Enables the corresponding interrupts and update interrupt type
 *		counters
 *
 ******************************************************************************/
int er_enable_interrupts(struct dpci_device *ddp, struct event_request *pevent)
{
	unsigned int event_type = pevent->er_type_mask;
	unsigned char set = 0;
	int port;
	u8 port_int_mask;
	u8 port_edge_state;
	int ret = 0;

	if ((event_type & EVENT_PFD) && (!ddp->global_queue_flags.pfd_flag))
	{
		/*
		 * Enable PFD interrupts only if we have not yet logged any 
		 * PFD events
		 */
		if (!ddp->pfd_event_logged)
			set |= DPCI_REG0_PFDIEN;
	}
	if ((event_type & EVENT_TS) && (!ddp->global_queue_flags.ts_flag))
	{
		dpci_set_ioboard_intcfg(ddp, DPCI_IOBOARD62_INTCFG_TS, 0);
	}
	if ((event_type & EVENT_IDLP) && (!ddp->global_queue_flags.idlp_flag))
	{
		dpci_id_intenable(ddp);
		set |= DPCI_REG0_PICINTEN;
	}
	if (event_type & EVENT_DIG_IP)
	{
		unsigned long int_mask;

		// For the time-being, do NOT mask out the global digip_flags
		// because this stops us taking another interrupt and correctly
		// getting the change data.  The way we do this needs to change
		// slightly in future.
		//
		int_mask = pevent->er_param.int_mask;
		//int_mask &= ~ddp->global_queue_flags.digip_flags;
		for (port = 0; ddp->reg_ip_status[port]; port++)
		{
			port_int_mask = (int_mask >> (port * 8)) & 0xff;
			if (port_int_mask == 0)
				continue;
			port_edge_state = 
				(pevent->er_param.edge_state >> (port * 8)) & 0xff;
			if ((ret = dpci_io_modifyint(ddp, port,
					port_int_mask,
					0,
					port_edge_state)) != 0)
			{
				return ret;
			}
		}
		//printk(KERN_INFO "im=%08lx es=%08lx gqf=%08lx ql=%d\n", int_mask, pevent->er_param.edge_state, ddp->global_queue_flags.digip_flags, ddp->er_queue_len);

		/*
		 * Enable I/O board interrupts.
		 */
		dpci_set_ioboard_intcfg(ddp, DPCI_IOBOARD_INTCFG_IO, 0);
	}
	if (set)
		dpci_reg_chg(ddp, DPCI_REG0, set, 0, 0);
	return 0;
}


/*******************************************************************************
 *
 * Function:    er_disable_interrupts()
 *
 * Parameters:  ddp - device instance
 *		event_type - mask of event types
 *
 * Returns:     0 on success; error code otherwise
 *
 * Description: Disables the corresponding interrupts and update interrupt type
 *		counters
 *
 ******************************************************************************/
int er_disable_interrupts(struct dpci_device *ddp, 
			unsigned int event_type)
{
	unsigned char clear = 0;
	int port;
	if (event_type & EVENT_PFD)
	{	
		clear |= DPCI_REG0_PFDIEN;
	}
	if (event_type & EVENT_TS)
	{
		dpci_set_ioboard_intcfg(ddp, 0, DPCI_IOBOARD62_INTCFG_TS);
	}
	if (event_type & EVENT_IDLP)
	{
		dpci_id_intdisable(ddp);
		clear |= DPCI_REG0_PICINTEN;
	}
	if (event_type & EVENT_DIG_IP)
	{
		for (port = 0; ddp->reg_ip_status[port]; port++)
		{
			dpci_io_modifyint(ddp, port, 0, 0xff, 0);
		}
		/*
		 * Disable I/O board interrupts.
		 */
		dpci_set_ioboard_intcfg(ddp, 0, DPCI_IOBOARD_INTCFG_IO);
	}
	if (clear)
		dpci_reg_chg(ddp, DPCI_REG0, 0, clear, 0);
	return 0;
}


/*******************************************************************************
 *
 * Function:    queue_insert_tail()
 *
 * Parameters:  ddp - the device instance.
 *		pevent - pointer to event request
 *
 * Returns:     int - 0 on success; error code otherwise
 *
 * Description: Adds a new event request to the event queue.
 *
 * Locking:	must be called with event_queue_lock held.
 *
 ******************************************************************************/
int queue_insert_tail(struct dpci_device *ddp, 
		struct event_request *pevent)
{
	/*
	 * We allow a maximum queue size of MAX_EVENT_QUEUE_SIZE
	 */
	if ((ddp->er_queue_len + 1) <= MAX_EVENT_QUEUE_SIZE)
	{
		if (ddp->er_queue_tail != NULL)
		{
			pevent->prev_er = ddp->er_queue_tail;
			ddp->er_queue_tail->next_er = pevent;		
		}
		ddp->er_queue_tail = pevent;
		if (ddp->er_queue_head == NULL)
			ddp->er_queue_head = pevent;
		ddp->er_queue_len++;
		return 0;
	}
	else
	{
		PRINT_ERR("Failed to add event request to queue."
			"Maximum queue size exceeded!\n");
		return -EINVAL;
	}
}


/*******************************************************************************
 *
 * Function:    queue_remove_er()
 *
 * Parameters:  ddp - the device instance.
 *		pevent - pointer to event request
 *
 * Returns:     int - 0 on success; error code otherwise
 *
 * Description: Adds a new event request to the event queue.
 *
 * Locking:	must be called with event_queue_lock held.
 *
 ******************************************************************************/
int queue_remove_er(struct dpci_device *ddp, 
		struct event_request *pevent)
{
	if (pevent == NULL)
		return -EINVAL;
	
	if (pevent->prev_er != NULL)
		pevent->prev_er->next_er = pevent->next_er;
	else
		ddp->er_queue_head = pevent->next_er;

	if (pevent->next_er != NULL)
		pevent->next_er->prev_er = pevent->prev_er;
	else
		ddp->er_queue_tail = pevent->prev_er;

	pevent->er_state = DISABLED;
	ddp->er_queue_len--;
	return 0;
}


/*******************************************************************************
 *
 * Function:    dpci_wait_events()
 *
 * Parameters:  ddp - the device instance.
 * 		er_type - the mask of event types to be monitored
 *		timeout - event-wait timeout
 *		cont_wait - should wait continue after one event
 *		op_event - buffer to hold the actual event triggered
 *		int_mask - interrupt mask for digital i/p event request
 *		edge_state - edge state mask for digital i/p event request
 *
 * Returns:     int - 1 on success; 0 on timeout; error code otherwise
 *
 * Description: Common routine to wait on a set of events. All other 
 *		wait-event routines call this function.
 *
 ******************************************************************************/
int dpci_wait_events(struct dpci_device *ddp, 
		unsigned int event_type_mask, 
		unsigned long timeout,
		int cont_wait,
		struct dpci_event *op_event,
		unsigned long int_mask,
		unsigned long edge_state,
		unsigned long autoconfig_edge)
{
	int ret = 0;
	struct event_request *pevent = NULL;
	struct digip_param event_params;
	unsigned long flags;
	int found_er = 0;
	int current_pid;
	struct event_request *curr_er;

	/*
	 * Mask off IDLP if it's disabled
	 */
	if(disable_idlp)
	{
		event_type_mask &= ~(EVENT_IDLP);
	}

	/*
	 * We have no devices from which to retrieve events
	 */
	if(!event_type_mask)
	{
		return -ENXIO;
	}

	/*
	 * If the event request waits only on pfd events, check whether 
	 * a pfd event has already occured and wait only if it has not.
	 */
	if (event_type_mask == EVENT_PFD)
	{
		if (ddp->pfd_event_logged)
			return -ENODATA;
		/*
		 * Wait for only one instance of pfd event
		 */
		cont_wait = 0;
	}
	event_params.int_mask = int_mask;
	event_params.edge_state = edge_state;
	event_params.autoconfig_edge = autoconfig_edge;

	if (cont_wait)
	{
		/*
		 * Check whether this process already has an active wait thread
		 */
		spin_lock_irqsave(&ddp->event_queue_lock, flags);
		current_pid = current->pid;
		curr_er = ddp->er_queue_head;
		while (curr_er != NULL)
		{
			if (curr_er->process_id != current_pid)
			{
				curr_er = curr_er->next_er;
				continue;
			}

			/*
			 * PID match found
			 */
			if ((curr_er->er_type_mask == event_type_mask)
				&& (curr_er->cont_wait == cont_wait))
			{
				found_er = 1;
				if (event_type_mask & EVENT_DIG_IP)
				{
					if ((curr_er->er_param.int_mask != int_mask) 
					|| (curr_er->er_param.edge_state != edge_state))
						found_er = 0;
				}
				if (found_er)
				{
					pevent = curr_er;
					break;
				}
			}
				
			/*
			 * Same PID but new event request. We destroy the 
			 * current request from the queue and add this new 
			 * request
			 */
			curr_er->er_state = SIGNALLED;
			if (waitqueue_active(&ddp->thread_event.er_wqh))
			{
				ddp->thread_event.er_signal = SIGNAL_PARAMS;
				wake_up_interruptible(
					&ddp->thread_event.er_wqh);
			}
			queue_remove_er(ddp, curr_er);
			destroy_event_request(curr_er);
			break;					
		}
		spin_unlock_irqrestore(&ddp->event_queue_lock, flags);
	}
	
	if (!found_er)
	{
		ddp->event_flag = 0;
		pevent = init_event_request(event_type_mask, event_params, cont_wait);
		if (pevent == NULL)
		{
			// could not create new event request
			ddp->event_flag = 1;
			return -ENOMEM;
		}
		/*
		 * add event request to queue
		 */
		spin_lock_irqsave(&ddp->event_queue_lock, flags);
		ret = queue_insert_tail(ddp, pevent);
		spin_unlock_irqrestore(&ddp->event_queue_lock, flags);
		if (ret != 0)
		{
			// could not insert into queue
			ddp->event_flag = 1;

			/*
			 * Free unused event request
			 */
			destroy_event_request(pevent);
			return ret;
		}
		ddp->event_flag = 1;
		if (waitqueue_active(&ddp->er_thread_wqh))
		{
			ddp->er_thread_signal = 1;
			wake_up_interruptible(&ddp->er_thread_wqh);
		}
		/*
		 * If the thread-wait is already active, we have to abort it now 
		 * so that this new event request gets included
		 */
		if (waitqueue_active(&ddp->thread_event.er_wqh))
		{
			ddp->thread_event.er_signal = SIGNAL_PARAMS;
			wake_up_interruptible(&ddp->thread_event.er_wqh);
		}
		/*
		 *  wait on new condition dedicated to this event request 
		 */
		pevent->er_signal = SIGNAL_NONE;
	}
	
	ret = wait_event_interruptible_timeout(pevent->er_wqh, 
			((pevent->er_signal == SIGNAL_KILL) || 
				(pevent->er_signal && pevent->num_events)), 
				msecs_to_jiffies(timeout));
	/*
	 * In case of timeout or cancellation of event-wait by user,
	 * wakeup thread event 
	 */
	if ((pevent->er_signal == SIGNAL_KILL) || (ret < 0) || ((ret == 0) && (!pevent->cont_wait)))
	{
		pevent->er_state = SIGNALLED;
		if (waitqueue_active(&ddp->thread_event.er_wqh))
		{
			ddp->thread_event.er_signal = SIGNAL_PARAMS;
			wake_up_interruptible(&ddp->thread_event.er_wqh);
		}
	}

	ret = 0;
	if (pevent->num_events > 0)
	{
		spin_lock_irqsave(&pevent->event_list_lock, flags);
		if (op_event_list_remove(&pevent->event_list, op_event) == 0)
		{
			pevent->num_events--;
			if (pevent->er_signal != SIGNAL_KILL)
				ret = 1;
		}
		spin_unlock_irqrestore(&pevent->event_list_lock, flags);
	}
	if (pevent->er_state == SIGNALLED)
	{
		spin_lock_irqsave(&ddp->event_queue_lock, flags);
		/*
		 * dequeue event request 
		 */
		queue_remove_er(ddp, pevent);
		spin_unlock_irqrestore(&ddp->event_queue_lock, flags);

		PRINT_INFO("dpci_core: dpci_wait_events(): event dequeued\n");
		/* 
		 * destroy event request
		 */
		destroy_event_request(pevent);
		PRINT_INFO("dpci_core: dpci_wait_events(): event destroyed\n");
	}
	else
	{
		// TBD
		/*
		 * We do not want to remove the event request from the queue
		 * We only want to pass this event/timeout signal to the user 
		 * process/thread and again continue waiting for a new event.
		 */
		
	}
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_wait_allevents()
 *
 * Parameters:  ddp - the device instance.
 *		timeout - event-wait timeout
 *
 * Returns:     int - 0 on success; error code otherwise
 *
 * Description: Function to handler ioctl request for wait on all events. 
 *
 ******************************************************************************/
int dpci_wait_allevents(struct dpci_device *ddp, 
			struct dpci_event_timeout *op_event_timeout)
{
	unsigned int event_type = 0;
	unsigned long int_mask = 0xffffffff;
	unsigned long edge_state = 0xffffffff;
	unsigned long autoconfig_edge = 0xffffffff;
	unsigned long timeout = op_event_timeout->timeout;
	
	/*
	 * No IO board means no events.
	 */
	if(ddp->reg_ip != NULL)
	{
		event_type |= EVENT_DIG_IP;
	}

	if(!disable_idlp)
	{
		event_type |= EVENT_IDLP;
	}

	if (ddp->features & HAVE_POWERFAILDETECT)
		event_type |= EVENT_PFD;
	if (ddp->ioboard &&
		ddp->ioboard->db_board_id == DPCI_IOBOARD_ID_80_0062A)
		event_type |= EVENT_TS;

	return dpci_wait_events(ddp, 
			event_type, 
			timeout, 
			1, 
			&op_event_timeout->event, 
			int_mask, 
			edge_state,
			autoconfig_edge);
}


/*******************************************************************************
 *
 * Function:    dump_event_request()
 *
 * Parameters:  ddp - the device instance.
 *		erp - event request structure
 *
 * Returns:     int - 0 on success; error code otherwise
 *
 * Description: dump an event request structure, following the chain.
 *
 ******************************************************************************/
static void dump_event_request(const char *tag, struct event_request *erp)
{
	PRINT_INFO("EVENT REQUEST @%p (\"%s\")\n", erp, tag);
	if (!erp)
	{
		return;
	}
	PRINT_INFO(" process_id=%d\n", erp->process_id);
	PRINT_INFO(" thread-leader=%d\n", erp->thread_group);
	PRINT_INFO(" er_state=%d\n", erp->er_state);
	PRINT_INFO(" er_type_mask=%d\n", erp->er_type_mask);
	PRINT_INFO(" er_signal=%d\n", erp->er_signal);
	PRINT_INFO(" last_ts=%lld\n", erp->last_ts);
	PRINT_INFO(" num_events=%d\n", erp->num_events);
	PRINT_INFO(" cont_wait=%d\n", erp->cont_wait);
	PRINT_INFO(" digip.int_mask=%08lx\n", erp->er_param.int_mask);
	PRINT_INFO(" digip.edge_state=%08lx\n", erp->er_param.edge_state);
	PRINT_INFO(" digip.autoconfig_edge=%08lx\n", erp->er_param.autoconfig_edge);
	PRINT_INFO(" next_er=%p\n", erp->next_er);
	PRINT_INFO("END OF REQUEST @%p (\"%s\")\n", erp, tag);
	if (erp->next_er)
		dump_event_request("next in chain", erp->next_er);
}


/*******************************************************************************
 *
 * Function:    dpci_event_queue_dump()
 *
 * Parameters:  ddp - the device instance.
 *
 * Returns:     int - 0 on success; error code otherwise
 *
 * Description: Dump event queue information to system log.
 *
 ******************************************************************************/
void dpci_event_queue_dump(struct dpci_device *ddp)
{
	PRINT_INFO("IOCTL_DPCI_DEBUG_DUMP_EVENTWAIT:\n");
	PRINT_INFO("er_thread_signal=%d\n", ddp->er_thread_signal);
	PRINT_INFO("er_thread=%p\n", ddp->er_thread);
	PRINT_INFO("er_queue_len=%d\n", ddp->er_queue_len);
	PRINT_INFO("event_flag=0x%x\n", ddp->event_flag);
	PRINT_INFO("er_stopped=%d\n", ddp->er_stopped);
	dump_event_request("global thread_event", &ddp->thread_event);
	dump_event_request("global queue head", ddp->er_queue_head);
}


/*******************************************************************************
 *
 * Function:    dpci_event_release_requests()
 *
 * Parameters:  ddp - the device instance.
 *
 * Returns:     nothing
 *
 * Description: remove event requests for the current thread group.  This is
 *              called when the current thread closes its file descriptor for
 *		/dev/dpci0.
 *
 ******************************************************************************/
void dpci_event_release_requests(struct dpci_device *ddp)
{
	unsigned long flags;
	struct event_request *erp, *erp_next = NULL;

	spin_lock_irqsave(&ddp->event_queue_lock, flags);
	for (erp = ddp->er_queue_head; erp; erp = erp_next)
	{
		erp_next = erp->next_er;
		if (erp->thread_group == current->group_leader->pid)
		{
			queue_remove_er(ddp, erp);
			destroy_event_request(erp);
		}
	}
	spin_unlock_irqrestore(&ddp->event_queue_lock, flags);
}


/*******************************************************************************
 *
 * Function:    dpci_event_release_requests()
 *
 * Parameters:  ddp - the device instance.
 *
 * Returns:     nothing
 *
 * Description: remove event requests for the current thread only.  This is
 *              called when the current thread calls dpci_ev_close_stream().
 *
 ******************************************************************************/
void dpci_event_release_current_request(struct dpci_device *ddp, pid_t thr_id)
{
	unsigned long flags;
	struct event_request *erp, *erp_next = NULL;

	spin_lock_irqsave(&ddp->event_queue_lock, flags);
	for (erp = ddp->er_queue_head; erp; erp = erp_next)
	{
		erp_next = erp->next_er;
		if ((erp->thread_group == current->group_leader->pid)
			&& (thr_id == current->pid))
		{
			erp->er_state = SIGNALLED;
			erp->er_signal = SIGNAL_KILL;
			if (waitqueue_active(&erp->er_wqh))
			{
				wake_up_interruptible(&erp->er_wqh);
			}
			PRINT_DBG("Released resources for thread %d in group %d\n",
				thr_id,
				current->group_leader->pid);
			break;
		}
	}
	if (!erp)
	{
		PRINT_DBG("Couldn't release resources for thread %d in group %d\n",
			thr_id,
			current->group_leader->pid);
	}
	spin_unlock_irqrestore(&ddp->event_queue_lock, flags);
}

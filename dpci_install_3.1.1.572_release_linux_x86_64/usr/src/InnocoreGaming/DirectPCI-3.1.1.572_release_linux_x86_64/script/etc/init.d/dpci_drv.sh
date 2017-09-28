#!/bin/bash
###############################################################################
#
# $Id: dpci_drv.sh 10468 2014-01-24 14:06:56Z aidan $
#
# Copyright 2005-2014 Advantech Co Limited.
# All rights reserved.
#
# Description:
# Boot-time loader for DirectPCI facilities.
# DirectPCI v@VERSION@
#
###############################################################################

### BEGIN INIT INFO
# Provides: directpci
# Required-Start: 
# Required-Stop: 
# Default-Start:  2 3 4 5
# Default-Stop: 0 1 6
# Short-Description: start and stop services for Innocore DirectPCI
# Description: DirectPCI is Innocore Gaming's I/O facility.  Enabling the
#	DirectPCI service ensures that the SRAM, ROM and core drivers are loaded
#	and that dependent services are not loaded earlier.
### END INIT INFO

[ -f /etc/default/directpci ] && . /etc/default/directpci

# See if we're on an LSB-compliant distribution.
#
if [ -f /lib/lsb/init-functions ]; then
	 . /lib/lsb/init-functions
else
	killproc()
	{
		if [ "$2" ]; then
			pkill -$2 $1 
		else
			pkill $1
		fi
	}
	pidofproc()
	{
		/sbin/pidof $1
	}
	log_warning_msg()
	{
		echo "$*"
	}
	log_failure_msg()
	{
		echo "$*" >&2
	}
	log_success_msg()
	{
		echo "$*"
	}
fi

# Function to wait for a device to appear - we do this because
# udev may not respond always as fast as we'd like during system
# boot.
#
# Wait for $3 seconds for path $2 to appear to test $1
# Default test is -f, default time-out is 5sec.
#
wait_path()
{
	left=${3-5}
	tst=${1--f}
	while [ $left -gt 0 ]; do
		if [ $tst $2 ]; then
			return 0;
		fi
		left=`expr ${left} - 1`
		sleep 1
	done
	log_failure_msg "$2 did not appear in $3 seconds."
	return 1
}


# See if udev or some work-alike (e.g. busybox mdev) is running.
#
CAN_HOTPLUG=`cat /proc/sys/kernel/hotplug`

load_dpci_core()
{
	_RET=0

	# Load the driver.  This will not fail or crash even if the driver is
	# loaded already.
	#
	/sbin/modprobe dpci_core
	if [ $? -ne 0 ]; then
		log_failure_msg "  core I/O driver: did installation succeed?"
		return 1;
	fi

	# If udev or mdev are not running then make the device node manually.
	#
	if [ -z "$CAN_HOTPLUG" ]; then	
		core=`grep dpci_core /proc/devices|cut -f1 -d' '`
		rm -f /dev/dpci0
		mknod /dev/dpci0 c $core 0
	else
		wait_path -c /dev/dpci0 10
	fi
	if [ ! -c /dev/dpci0 ]; then
		log_failure_msg "  core I/O driver"
		return 1
	fi
	log_success_msg "  core I/O driver"


	if [ "$SYNC_IDLP_DATE" = yes ]; then
		if idutil -d sync; then
			log_success_msg "  IDLP date-sync"
		else
			log_failure_msg "  IDLP date-sync"
			_RET=1
		fi
	fi

	if [ "$DEBUG_IDLP_WATCHDOG" = yes ]; then
		if idutil -W $DEBUG_IDLP_WATCHDOG_TMOUT_SECS; then
			log_success_msg "  IDLP watch-dog"
		else
			log_failure_msg "  IDLP watch-dog"
			_RET=1
		fi
	fi

	if [ "$DEBUG_IO_WATCHDOG" = yes ]; then
		if ioboard -W; then
			log_success_msg "  I/O watch-dog"
		else
			log_failure_msg "  I/O watch-dog"
			_RET=1
		fi
	fi
	if [ "$DPCI_DEBUG_LEVEL" ]; then
		if dpci -D $DPCI_DEBUG_LEVEL; then
			log_success_msg "  debug level $DPCI_DEBUG_LEVEL"
		else
			log_failure_msg "  debug level $DPCI_DEBUG_LEVEL"
			_RET=1
		fi
	fi
	return $_RET
}

load_dpci_memory()
{
	# Only load if PCI hardware found.
	#
	(lspci -n | grep "16cd:01[08][01234]" >/dev/null) || return 0

	# Load the driver.  This will not fail or crash even if the driver is
	# loaded already.
	#
	/sbin/modprobe dpci_mem
	if [ $? -ne 0 ]; then
		log_failure_msg "  SRAM & ROM: did installation succeed?"
		return 1;
	fi

	# If udev or mdev are not running then make the device nodes manually.
	#
	if [ $? -eq 0 -a -z "$CAN_HOTPLUG" ]; then	
		mem=`grep dpci_mem /proc/devices|head -1|cut -f1 -d' '`
		rm -f /dev/sram0 /dev/rom0
		mknod /dev/sram0 c $mem 0
		mknod /dev/rom0 c $mem 1
	else
		# Dependening upon the hardware configuration either, neither
		# or both of SRAM and RAM might be configured.  We need to
		# be able to cope with this.
		#
		# We only check for the ROM device if PCI device IDs 0103/0104
		# are used.  If it's 0180 (the PCIe version) then we don't do
		# the check because with this device we can't tell at this
		# stage if the ROM-enable jumper is in place.
		#
		(lspci -n |grep "16cd:01[08][012]" >/dev/null) && wait_path -c /dev/sram0 10
		(lspci -n |grep "16cd:010[34]" >/dev/null) && wait_path -c /dev/rom0 10
	fi
	if [ -c /dev/sram0 -o -c /dev/rom0 ]; then
		log_success_msg "  SRAM & ROM"
	else
		log_failure_msg "  SRAM & ROM"
		return 1;
	fi
	return 0;
}

load_evdev()
{
	# See if we need to load evdev.
	#
	if [ -f /proc/config.gz ]; then
		case "`zcat /proc/config.gz | grep EVDEV | cut -f2 -d=`" in
		y)	# It's loaded do nothing.
			;;

		m)	# It's not loaded so load it.
			if modprobe evdev; then
				log_success_msg "  evdev"
			else
				log_failure_msg "  evdev"
				return 1;
			fi
			;;

		*)	# It's not configured.
			;;
		esac
	elif ! grep evdev /proc/bus/input/handlers >/dev/null; then
		# Pray this works.  If it doesn't, we ignore the errors.
		#
		if modprobe evdev; then
 			log_success_msg "  evdev"
		else
			log_failure_msg "  evdev"
			return 1;
		fi
	fi
	return 0;
}

load_dpci_serial()
{
	# See if we need to load serial devices.
	#
	if ioboard >/dev/null 2>&1; then
		# For kernels built with support for more than 4 legacy serial
		# ports (e.g. many default kernels on distributions) we have to
		# clear out the configuration data from /dev/ttyS4 - /dev/ttyS7
		# so that the serial serial port devices are consistent across
		# all platforms.  We may also have to do this for ports S14,
		# S15, S44 and S45 for the same reason.
		#
		# Also, to ensure we get a consistent experience, we also set up
		# ttyS2 (COM3) and ttyS3 (COM4) to have their traditional port
		# addresses even though they're not there and won't be used.
		#
		tty=${DPCI_SERIAL_FIRST_PORT-4}
		while [ $tty -lt 64 ]; do
			minor=`expr $tty + 64`
			[ ! -c /dev/ttyS$tty ] && mknod /dev/ttyS$tty c 4 $minor
			setserial /dev/ttyS$tty uart none irq 0 port 0 >/dev/null 2>&1
			tty=`expr $tty + 1`
		done

		# You can enable these lines if you have problems getting the
		# ports to add at the ttyS4 point.
		#
		#setserial /dev/ttyS2 uart 16550A port 0x3e8
		#setserial /dev/ttyS3 uart 16550A port 0x2e8

		# Load the driver.
		#
		if modprobe dpci_serial; then
			log_success_msg "  serial"
		else
			log_failure_msg "  serial"
			return 1
		fi
	fi
	return  0
}

stop_dpci_core()
{
	if grep dpci_core /proc/modules >/dev/null; then
		pkill ioboard
		pkill idutil
		sleep 1
		if /sbin/rmmod dpci_core; then
			log_success_msg "Stopped DirectPCI core I/O services."
		else
			log_failure_msg "Failed stopping dpci_core services."
			return 1
		fi
	fi
	return 0
}

stop_dpci_memory()
{
	if grep dpci_mem /proc/modules >/dev/null; then
		if /sbin/rmmod dpci_mem; then
			log_success_msg "Stopped DirectPCI memory services."
		else
			log_failure_msg "Failed Stopping dpci_mem services."
			return 1
		fi
	fi
	return 0
}

stop_dpci_serial()
{
	if grep dpci_serial /proc/modules >/dev/null; then
		if /sbin/rmmod dpci_serial; then
			log_success_msg "Stopped DirectPCI serial services."
		else
			log_failure_msg "Failed Stopping dpci_serial services."
			return 1
		fi
	fi
	return 0
}


if [ $# != 1 ]; then
	echo "usage: $0 [start|stop|restart|reload]" >&2
	exit 1;
fi

case "$1" in
start)
	echo "Starting DirectPCI services v@VERSION_NUM@:"
	RET=0
	load_dpci_core || RET=1
	([ $RET -eq 0 ] && load_dpci_serial) || RET=1
	load_evdev || RET=1
	load_dpci_memory || RET=1

	# That's it.
	#
	exit $RET
	;;

start_core)
	echo "Starting DirectPCI services v@VERSION_NUM@:"
	RET=0
	load_dpci_core || RET=1
	load_evdev || RET=1
	exit $RET
	;;

start_memory|start_mem)
	RET=0
	echo "Starting DirectPCI services v@VERSION_NUM@:"
	load_dpci_memory || RET=1
	exit $RET
	;;

start_serial)
	RET=0
	echo "Starting DirectPCI services v@VERSION_NUM@:"
	load_dpci_serial || RET=1
	exit $RET
	;;

stop)
	# Simply unload the drivers. Obviously this all depends upon no
	# processes existing and using the devices.
	#
	RET=0
	stop_dpci_serial || RET=1
	stop_dpci_memory || RET=1
	stop_dpci_core || RET=1
	exit $RET
	;;

stop_core)
	RET=0
	stop_dpci_serial || RET=1
	stop_dpci_memory || RET=1
	stop_dpci_core || RET=1
	exit $RET
	;;

stop_memory|stop_mem)
	RET=0
	stop_dpci_memory || RET=1
	exit $RET
	;;

stop_serial)
	RET=0
	stop_dpci_serial || RET=1
	exit $RET
	;;

restart|force-reload|reload)
	$0 stop && $0 start
	;;

probe)
	if lsmod |grep dpci >/dev/null; then
		echo $"restart";
	else
		echo $"start"
	fi
	exit 0
	;;

status)
	if [ "`$0 probe`" == restart ]; then
		echo "DPCI loaded"
		exit 0
	else
		echo "DPCI not loaded."
		exit 3
	fi
	;;

try-restart)
	if [ "`$0 probe`" == restart ]; then
		$0 restart
	fi
	;;

condrestart)
	if [ "`$0 probe`" == start ]; then
		$0 start
	else
		$0 restart
	fi
	;;

*)
	echo "Unknown command $1".
	exit 2
	;;
esac
exit 0;

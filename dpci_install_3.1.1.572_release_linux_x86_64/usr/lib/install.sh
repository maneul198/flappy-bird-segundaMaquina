#!/bin/sh
###############################################################################
#
# $Id: install-libs.sh 11750 2015-07-01 15:15:35Z aidan $
#
# Copyright 2008-2015 Advantech Corp Limited.
# All rights reserved.
#
# Description:
# Work out where to put the libraries.
#
# Some Linux distribs that support BOTH 64- and 32-bit versions of /usr/lib
# and /lib have nonetheless not got a standard way of choosing whether to have
# /lib and /lib32, /lib64 and /lib or /lib32 and /lib64 so we have work it out
# by avoiding placing libraries where they shouldn't go.
#
# Note:
# Ubuntu has /lib32 and /lib (and /usr/... too) and /lib64 for compatibility.
# Fedora has /lib64 and /lib (and /usr/... too)
#
###############################################################################

set -x
SHD=`dirname $0`
cd $SHD
echo test
if [ linux_x86_64 = linux_x86_64 ]; then
	# This is a 64-bit version of dpci being installed.
	#
	if [ -d /usr/lib64 ]; then
		cp -a *.so* /usr/lib64
	else
		cp -a *.so* /usr/lib
	fi
elif [ linux_x86_64 = linux_i686 ]; then
	# This is a 32-bit version of dpci being installed.
	#
	if [ -d /usr/lib32 ]; then
		cp -a *.so* /usr/lib32
	elif [ -d /usr/lib64 ]; then
		cp -a *.so* /usr/lib
	else
		cp -a *.so* /usr/lib
	fi
else
	echo "$0: ERROR: Unsupported Linux kernel architecture"
	exit 1;
fi
ldconfig

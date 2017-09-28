###############################################################################
#
# $Id: Makefile.src 11957 2015-09-23 11:19:20Z aidan $
#
# Copyright 2006-2015 Advantech Co Ltd.
# All rights reserved.
#
# Description:
# Makefile for DirectPCI kernel modules and demo source code
#
###############################################################################

SRCROOT		= $(shell pwd)
SUBDIRS		= driver demo
PROJ_CPPFLAGS	= -I$(PROJ_ROOT)/include -I/usr/include -D_VERSION_ARCH="\\\"linux_x86_64\\\""
DEFAULT_ARCH	= linux_x86_64
KERN_ARCH	= x86_64
LOCALKERNEL	= $(shell uname -r)
export DEFAULT_ARCH LOCALKERNEL

export DPCI_VERSION DPCI_VERSION_CODE PROJ_CPPFLAGS SRCROOT KERN_ARCH

include $(SRCROOT)/build/build.mk

.PHONY: dpci_% install_dpci_%
dpci_%:
	$(SILENT)$(MAKE) -C driver/$@ all

install-dpci_%:
	$(SILENT)$(MAKE) -C driver/`echo $@|sed s/install-//g` all modules_install
	$(SILENT)depmod -a

install-drivers:
	$(SILENT)$(MAKE) -C driver TARGET="all modules_install"
	$(SILENT)depmod -a


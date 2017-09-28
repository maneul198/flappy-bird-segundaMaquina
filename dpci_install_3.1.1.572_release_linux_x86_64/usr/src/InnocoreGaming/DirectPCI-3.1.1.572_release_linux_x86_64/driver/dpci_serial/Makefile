###############################################################################
#
# $Id: Makefile 11958 2015-09-23 11:38:03Z aidan $
#
# Copyright 2003-2008 Innocore Gaming Limited.
# All rights reserved.
#
# Description:
# Makefile for DPCI serial kernel 2.6 module
#
###############################################################################

DRIVERS = dpci_serial.ko

dpci_serial.ko_MODSYMDIRS = ../dpci_core
dpci_serial.ko_CPPFLAGS	= -I$(PROJ_ROOT)/driver/include

include $(SRCROOT)/build/rules.mk


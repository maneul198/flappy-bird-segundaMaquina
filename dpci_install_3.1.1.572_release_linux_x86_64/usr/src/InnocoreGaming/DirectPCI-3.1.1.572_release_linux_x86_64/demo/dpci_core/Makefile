###############################################################################
#
# $Id: Makefile 9996 2013-06-25 09:27:20Z richard $
#
# Copyright 2003-2011 Advantech Corporation Ltd.
# All rights reserved.
#
# Description:
# Makefile for DirectPCI core demos
#
###############################################################################

# These settings are designed so this makefile can be used (a) on installed
# systems, (b) stand-alone in the build environment or (c) invoked as part of
# a system build.  Although some of the paths here don't likely exist on a
# customer installed system, the make should still work.
#

# To link with the debug version, end users should change -ldpci to -ldpcidebug
#
PROJ_LDLIBS = -ldpci
VPATH = $(COMMON)/demo
BUILDS = DEBUG
EXES = batt_demo bios_demo idlp_demo io_demo i2c_demo eeprom_demo ts_demo \
	callback_demo idprom_demo

batt_demo_OBJS		= batt_demo.o
bios_demo_OBJS		= bios_demo.o
idlp_demo_OBJS		= idlp_demo.o
io_demo_OBJS		= io_demo.o
i2c_demo_OBJS		= i2c_demo.o
eeprom_demo_OBJS	= eeprom_demo.o
ts_demo_OBJS		= ts_demo.o
callback_demo_OBJS	= callback_demo.o
idprom_demo_OBJS	= idprom_demo.o

include $(SRCROOT)/build/rules.mk

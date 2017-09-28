###############################################################################
#
# $Id: Makefile 6571 2010-08-06 10:01:20Z aidan $
#
# Copyright 2003-2008 Innocore Gaming Limited.
# All rights reserved.
#
# Description:
# Makefile for DirectPCI ROM demo.
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
EXES = rom_demo
rom_demo_OBJS = rom_demo.o

include $(SRCROOT)/build/rules.mk

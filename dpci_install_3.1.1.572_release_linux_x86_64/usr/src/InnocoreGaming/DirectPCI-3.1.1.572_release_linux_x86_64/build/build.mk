###############################################################################
#
# $Id: build.mk.distrib 7738 2011-07-13 13:24:07Z aidan $
#
# Copyright 2008-2011 Advantech Corporation Limited.
# All rights reserved.
#
# Description:
# Customer-facing makefile generic rules and settings.
#
###############################################################################


SRCROOT := $(shell pwd)
PROJ_ROOT = $(shell pwd)
COMMON = $(shell pwd)

export SRCROOT COMMON PROJ_ROOT

AUTOBUILD=D

export AUTOBUILD

# Work out some kind of 'default' architecture for this build
#
ifeq ($(DEFAULT_ARCH),)
 ifeq ($(shell uname),Linux)
  ifeq ($(shell uname -m),i686)
ARCH    := linux_i686
  else
ARCH    := linux_x86_64
  endif
 else
ARCH    := win32
 endif
else
ARCH    := $(DEFAULT_ARCH)
endif
export ARCH

# Include OS-specific stuff
#
include $(SRCROOT)/build/build-$(ARCH).mk

export RULESET
export BUILD_SUBAREA

BUILDCONFIG_FILE=$(PROJ_ROOT)/.last-buildconfig
export BUILDCONFIG_FILE

BUILD_ROOT      = $(PROJ_ROOT)
RELEASE_BINDIR  = $(BUILD_ROOT)/bin
DEBUG_BINDIR    = $(BUILD_ROOT)/bin.DEBUG
export BUILD_ROOT RELEASE_BINDIR DEBUG_BINDIR

.PHONY: bin bindirs mkbindirs
bin: all

FIRST = mkbindirs
# mkbindirs merely creates the binary directories themselves.
#
mkbindirs:
	$(VSILENT)mkdir -p  $(RELEASE_BINDIR) $(DEBUG_BINDIR)


PROJ_CPPFLAGS += -$(AUTOBUILD)AUTO_BUILD_NO -I$(SRCROOT)/build

export PROJ_CPPFLAGS PROJ_CFLAGS PROJ_CXXFLAGS PROJ_LDFLAGS PROJ_LDLIBS

include $(SRCROOT)/build/rules.mk

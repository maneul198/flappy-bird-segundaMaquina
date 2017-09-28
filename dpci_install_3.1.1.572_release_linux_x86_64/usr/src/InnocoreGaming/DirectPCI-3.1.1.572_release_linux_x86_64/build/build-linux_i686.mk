###############################################################################
#
# $Id: build-linux_i686.mk 6049 2010-03-01 12:09:41Z aidan $
#
# Copyright 2008 Innocore Gaming Limited.
# All rights reserved.
#
# Description:
# build.mk supplements for Linux builds.
#
###############################################################################


# Flags for this architecture. 
#
RULESET		:= linux
ARCH		:= linux_i686
KERN_ARCH	:= i386
VERSION_OS	:= linux
VERSION_ARCH	:= i686
ARCH_CPPFLAGS	:=
ARCH_CFLAGS	:= -m32
ARCH_LDFLAGS	:= -m32 -Wl,-melf_i386
ARCH_LDNFLAGS	:= -melf_i386

export KERN_ARCH

BSYS_CPPFLAGS	= $(ARCH_CPPFLAGS) \
			-D_IGL_SYSTEM_BUILD \
			-D_VERSION_ARCH=$(VERSION_ARCH) \
			-D_VERSION_BUILD=0 \
			-$(AUTOBUILD)AUTO_BUILD_NO \
			-I$(SRCROOT)/build
BSYS_CFLAGS	= $(ARCH_CFLAGS)
BSYS_LDFLAGS	= $(ARCH_LDFLAGS)
BSYS_LDNFLAGS	= $(ARCH_LDNFLAGS)

SRCROOT_X	:= $(SRCROOT)
export SRCROOT_X

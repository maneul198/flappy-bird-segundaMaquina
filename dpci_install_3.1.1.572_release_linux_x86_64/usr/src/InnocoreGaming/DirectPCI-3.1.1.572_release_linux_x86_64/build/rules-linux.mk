###############################################################################
#
# $Id: rules-linux.mk 12909 2016-11-24 12:29:27Z aidan $
#
# Copyright 2006-2013 Advantech Co Limited.
# All rights reserved.
#
# Description:
# Partial make file with rules for most types of thing.
#
###############################################################################

# These variables may be overridden in local Makefiles.
#
SONAME		?= $(_NAME)
DEBUG_SONAME	?= $(SONAME)
KDIR		?= /lib/modules/$(shell uname -r)/build
PWD		:= $(shell pwd)
OBJ_SUFFIX	:= .o
SHL_SUFFIX	:= .so
EXE_SUFFIX	:= 
LIB_SUFFIX	:= .a

KSILENT		= @

# These are paths to useful programs.
#
AR		= ar
ARFLAGS		= rus
CC		= gcc
CPP             = cpp
CXX             = g++
LD              = ld
LINKER		?= gcc

MAKE_JOB_FLAGS	= -j 3

# Flags for programs
#
DEPS_LDFLAGS	:= $(patsubst %, -L%, $(_DEPS_LIBPATHS)) $(patsubst %, -Wl$(comma)-rpath-link$(comma)%, $(_DEPS_LIBPATHS))
DEPS_CPPFLAGS	:= $(patsubst %, -I%, $(_DEPS_INCPATHS))

OPTIMIZE	?= -O
CPPFLAGS	= $(BSYS_CPPFLAGS) $(DEPS_CPPFLAGS) $(PROJ_CPPFLAGS) -D_GNU_SOURCE -Dlinux -I$(SRCROOT)/build
CPPFLAGS_RELEASE= $(CPPFLAGS) -DNDEBUG -DRELEASE
CPPFLAGS_DEBUG  = $(CPPFLAGS) -DDEBUG
CFLAGS		= $(BSYS_CFLAGS) $(PROJ_CFLAGS) -MMD
CFLAGS_RELEASE  = $(CFLAGS) $(OPTIMIZE) -Wall -Werror
CFLAGS_DEBUG    = $(CFLAGS) -g -Wall
CXXFLAGS	= $(PROJ_CXXFLAGS)
ifneq ($(VERSION_CODE),)
LDFLAGS		= $(DEPS_LDFLAGS) -Wl,--defsym,_IGL_VERSION=0x$(VERSION_CODE) $(BSYS_LDFLAGS) $(PROJ_LDFLAGS)
else
LDFLAGS		= $(DEPS_LDFLAGS) $(BSYS_LDFLAGS) $(PROJ_LDFLAGS)
endif
RPATH_FLAGS	= -Wl,-rpath,.
LDFLAGS_DEBUG	= $(RPATH_FLAGS)
LDFLAGS_RELEASE	=
LDLIBS		= $(PROJ_LDLIBS)

B__DATE__	:= $(shell date "+%a %_d %b %Y ")
B__TIME__	:= $(shell date "+%T %Z")

export SYSKERNELS

export CFLAGS

.PHONY: clean_%
clean: $(CLEAN)
	@echo "Cleaning."
	-$(SILENT)$(RM) -rf $(RELEASE_DIR)/obj_* $(DEBUG_DIR)/obj_*
	$(VSILENT)$(MAKE) $(MAKE_OPT) subdirs TARGET=clean

# Rules for normal object files.
#
$(RELEASE_OBJDIR)/%.o: %.i
	@echo "Compiling $<"
	$(SILENT)$(CC) -c $(CFLAGS_RELEASE) -o $@ $<

$(DEBUG_OBJDIR)/%.o: %.i
	@echo "Compiling $< (debug)"
	$(SILENT)$(CC) -c $(CFLAGS_DEBUG) -o $@ $<

$(RELEASE_OBJDIR)/%.o: %.c
	@echo "Compiling $<"
	$(SILENT)$(CC) -c $(CPPFLAGS_RELEASE) $(CFLAGS_RELEASE) -o $@ $<

$(DEBUG_OBJDIR)/%.o: %.c
	@echo "Compiling $< (debug)"
	$(SILENT)$(CC) -c $(CPPFLAGS_DEBUG) $(CFLAGS_DEBUG) -o $@ $<

$(RELEASE_OBJDIR)/%.o: %.cpp $(BUILDCONFIG_FILE)
	@echo "Compiling $<"
	$(SILENT)$(CXX) -c $(CXXFLAGS) $(CPPFLAGS_RELEASE) $(CFLAGS_RELEASE) -o $@ $<

$(DEBUG_OBJDIR)/%.o: %.cpp $(BUILDCONFIG_FILE)
	@echo "Compiling $< (debug)"
	$(SILENT)$(CXX) -c $(CXXFLAGS) $(CPPFLAGS_DEBUG) $(CFLAGS_DEBUG) -o $@ $<

$(RELEASE_OBJDIR)/%.i: %.c
	@echo "Pre-processing $<"
	$(SILENT)$(CPP) $(CPPFLAGS_RELEASE) >$@ $<

$(DEBUG_OBJDIR)/%.i: %.c
	@echo "Pre-processing $< (debug)"
	$(SILENT)$(CPP) $(CPPFLAGS_DEBUG) >$@ $<

$(RELEASE_OBJDIR)/%.ipp: %.cpp
	@echo "Pre-processing $<"
	$(SILENT)$(CPP) $(CPPFLAGS_RELEASE) >$@ $<

$(DEBUG_OBJDIR)/%.ipp: %.cpp
	@echo "Pre-processing $< (debug)"
	$(SILENT)$(CPP) $(CPPFLAGS_DEBUG) >$@ $<

$(RELEASE_OBJDIR) $(DEBUG_OBJDIR) $(RELEASE_DIR) $(DEBUG_DIR) $(RELEASE_BINDIR) $(DEBUG_BINDIR)::
	$(SILENT)mkdir -p $@


# Rules to build libraries
#
# This is how we build .a and .so files.
#
$(DEBUG_DIR)/%.o: $(DEBUG_OBJS) $(DEPS) $(BUILDCONFIG_FILE)
	@echo "Linking object $@"
	$(SILENT)$(LINKER) $(LDFLAGS) -o $@ -r $(DEBUG_OBJS) $(LDLIBS)
	$(SILENT)mkdir -p $(DEBUG_BINDIR)/$(PROJ_BINSUBDIR)
	$(SILENT)cp $@ $(DEBUG_BINDIR)/$(PROJ_BINSUBDIR)

$(RELEASE_DIR)/%.o: $(RELEASE_OBJS) $(DEPS) $(BUILDCONFIG_FILE)
	@echo "Linking object $@"
	$(SILENT)$(LINKER) $(LDFLAGS) -o $@ -r $(RELEASE_OBJS) $(LDLIBS)
	$(SILENT)mkdir -p $(RELEASE_BINDIR)/$(PROJ_BINSUBDIR)
	$(SILENT)cp $@ $(RELEASE_BINDIR)/$(PROJ_BINSUBDIR)

$(DEBUG_DIR)/%.so: $(DEBUG_OBJS) $(DEPS) $(BUILDCONFIG_FILE)
	@echo "Linking shared library $@"
	$(SILENT)$(LINKER) $(LDFLAGS) $(LDFLAGS_DEBUG) -o $@ -shared -Wl,-soname=$(DEBUG_SONAME) $(DEBUG_OBJS) $(LDLIBS)
	$(SILENT)mkdir -p $(DEBUG_BINDIR)/$(PROJ_BINSUBDIR)
	$(SILENT)cp $@ $(DEBUG_BINDIR)/$(PROJ_BINSUBDIR)

$(RELEASE_DIR)/%.so: $(RELEASE_OBJS) $(DEPS) $(BUILDCONFIG_FILE)
	@echo "Linking shared library $@"
	$(SILENT)$(LINKER) $(LDFLAGS) $(LDFLAGS_RELEASE) -o $@ -shared -Wl,-soname=$(SONAME) $(RELEASE_OBJS) $(LDLIBS)
	$(SILENT)mkdir -p $(RELEASE_BINDIR)/$(PROJ_BINSUBDIR)
	$(SILENT)cp $@ $(RELEASE_BINDIR)/$(PROJ_BINSUBDIR)

$(DEBUG_DIR)/%.a: $(DEBUG_OBJS) $(DEPS) $(PROJ_DEPS) $(BUILDCONFIG_FILE)
	@echo "Creating archive $@"
	$(SILENT)-rm -f $@
	$(SILENT)$(AR) $(ARFLAGS) $@ $(DEBUG_OBJS)
	$(SILENT)mkdir -p $(DEBUG_BINDIR)/$(PROJ_BINSUBDIR)
	$(SILENT)cp $@ $(DEBUG_BINDIR)/$(PROJ_BINSUBDIR)

$(RELEASE_DIR)/%.a: $(RELEASE_OBJS) $(DEPS) $(PROJ_DEPS) $(BUILDCONFIG_FILE)
	@echo "Creating archive $@"
	$(SILENT)-rm -f $@
	$(SILENT)$(AR) $(ARFLAGS) $@ $(RELEASE_OBJS)
	$(SILENT)mkdir -p $(RELEASE_BINDIR)/$(PROJ_BINSUBDIR)
	$(SILENT)cp $@ $(RELEASE_BINDIR)/$(PROJ_BINSUBDIR)


ifneq ($(_NAME),)
_LIBS = $(foreach build,$(BUILDS),$(build)/$(_NAME))
endif

ifneq ($(MODSYMDIRS),)
_DBGMODSYMPATHS=$(foreach dir,$(MODSYMDIRS),$(dir)/$(DEBUG_DIR)/obj_$(notdir $(dir))$(_KTAG)/Module.symvers)
_RELMODSYMPATHS=$(foreach dir,$(MODSYMDIRS),$(dir)/$(RELEASE_DIR)/obj_$(notdir $(dir))$(_KTAG)/Module.symvers)
endif

ifneq ($(_NAME),)
_DRIVERS = $(foreach build,$(BUILDS),$(build)/$(_NAME).ko)
endif

modules: drivers

KTGT=modules

modules_install:
	$(VSILENT)$(MAKE) drivers KTGT=modules_install

# Rules to build Linux kernel drivers
#
$(DEBUG_DIR)/%.ko: $(DEPS)
	@echo "Compiling and linking kernel driver $@"
	$(SILENT)echo "obj-m =$(_NAME).o" >$(DEBUG_OBJDIR)/Makefile
ifneq ($(OBJS),)
	$(SILENT)echo "$(_NAME)-objs  = $(OBJS)" >>$(DEBUG_OBJDIR)/Makefile
endif
ifneq ($(MODSYMDIRS),)
	$(SILENT)rm -f $(DEBUG_OBJDIR)/Module.symvers
	$(SILENT)cat $(_DBGMODSYMPATHS) >$(DEBUG_OBJDIR)/Module.symvers
endif
ifeq ($(SRCS),)
	-$(SILENT)cp *.c *.h $(DEBUG_OBJDIR)
else
	$(SILENT)cp $(SRCS) $(DEBUG_OBJDIR)
endif
	$(KSILENT)$(MAKE) ARCH=$(KERN_ARCH) -C $(KDIR) Q=$(SILENT) M="$(PWD)/$(DEBUG_OBJDIR)" EXTRA_CFLAGS="$(EXTRA_CFLAGS) $(DEPS_CPPFLAGS) $(BSYS_CPPFLAGS) -DDEBUG -D_B__DATE__=\"$(B__DATE__)\" -D_B__TIME__=\"$(B__TIME__)\"" $(KTGT)
	$(SILENT)mkdir -p $(DEBUG_BINDIR)/linux$(_KTAG)
	$(SILENT)cp $(DEBUG_OBJDIR)/$(_NAME).ko $(DEBUG_BINDIR)/linux$(_KTAG)/$(_NAME).ko

$(RELEASE_DIR)/%.ko: $(DEPS)
	@echo "Compiling and linking kernel driver $@"
	$(SILENT)echo "obj-m =$(_NAME).o" >$(RELEASE_OBJDIR)/Makefile
ifneq ($(OBJS),)
	$(SILENT)echo "$(_NAME)-objs  = $(OBJS)" >>$(RELEASE_OBJDIR)/Makefile
endif
ifneq ($(MODSYMDIRS),)
	$(SILENT)rm -f $(RELEASE_OBJDIR)/Module.symvers
	$(SILENT)cat $(_RELMODSYMPATHS) >$(RELEASE_OBJDIR)/Module.symvers
endif
ifeq ($(SRCS),)
	-$(SILENT)cp *.c *.h $(RELEASE_OBJDIR)
else
	$(SILENT)cp $(SRCS) $(RELEASE_OBJDIR)
endif
	$(KSILENT)$(MAKE) ARCH=$(KERN_ARCH) -C $(KDIR) Q=$(SILENT) M="$(PWD)/$(RELEASE_OBJDIR)" EXTRA_CFLAGS="$(EXTRA_CFLAGS) $(DEPS_CPPFLAGS) $(BSYS_CPPFLAGS) -D_B__DATE__=\"$(B__DATE__)\" -D_B__TIME__=\"$(B__TIME__)\"" $(KTGT)
	$(SILENT)mkdir -p $(RELEASE_BINDIR)/linux$(_KTAG)
	$(SILENT)cp $(RELEASE_OBJDIR)/$(_NAME).ko $(RELEASE_BINDIR)/linux$(_KTAG)/$(_NAME).ko

ifeq ($(AUTOBUILD),D)
 ifeq ($(LOCALKERNEL),)
SYSKERNELS=1
 endif
endif

ifeq ($(SYSKERNELS),1)
KERNELS ?= $(shell svn proplist $(SRCROOT)|grep igl:build:$(ARCH):kernel)
endif
ifeq ($(KERNELS),)
LOCALKERNEL:=$(shell uname -r)
KERNELS=$(LOCALKERNEL)
endif
ifeq ($(KDIR),)
KDIR=/lib/modules/$(shell uname -r)/build
endif
export KDIR

.PHONY: $(DRIVERS)
$(DRIVERS):
	@echo
	@echo "***********************************"
	@echo "Building kernel driver $@"
	@echo "***********************************"
	$(VSILENT)for kv in $(KERNELS); do \
		if [ -z "$(LOCALKERNEL)" ]; then \
			ktag=`echo $$kv|sed "s/^ *igl:build:$(ARCH):kernel://"`; \
			KDIR=`svn propget $$kv $(SRCROOT)`; \
		else \
			ktag=$$kv; \
		fi; \
		if [ ! -d "$$KDIR" ]; then \
			echo "\n*** Kernel build directory $$KDIR does not exist. ***\n" >&2; \
			exit 1; \
		fi; \
		if ! $(MAKE) $(MAKE_OPT) \
			objdirs \
			$(foreach build,$(BUILDS),$($(build)_DIR)/$@) \
			_NAME=$(basename $@) \
			OBJS="$($@_OBJS)" \
			SRCS="$($@_SRCS)" \
			DEPS="$($@_DEPS)" \
			KDIR=$$KDIR \
			_KTAG=_$$ktag \
			KNAME=$$ktag \
			MODSYMDIRS="$($@_MODSYMDIRS)" \
			EXTRA_CFLAGS="$($@_CPPFLAGS) $($@_CFLAGS) $(PROJ_CPPFLAGS)"; \
		then \
			exit $$?; \
		fi; \
	done


# Rules to build executables
#
$(DEBUG_DIR)/%: $(DEBUG_OBJS) $(DEPS) $(PROJ_DEPS) $(BUILDCONFIG_FILE)
	@echo "Linking executable $@"
	$(SILENT)$(LINKER) $(LDFLAGS) $(LDFLAGS_DEBUG) -o $@ $(DEBUG_OBJS) $(EXTRA_OBJS) $(LDLIBS)
	$(SILENT)mkdir -p $(DEBUG_BINDIR)/$(PROJ_BINSUBDIR)
	$(SILENT)cp $@ $(DEBUG_BINDIR)/$(PROJ_BINSUBDIR)

$(RELEASE_DIR)/%: $(RELEASE_OBJS) $(DEPS) $(PROJ_DEPS) $(BUILDCONFIG_FILE)
	@echo "Linking executable $@"
	$(SILENT)$(LINKER) $(LDFLAGS) $(LDFLAGS_RELEASE) -o $@ $(RELEASE_OBJS) $(EXTRA_OBJS) $(LDLIBS)
	$(SILENT)mkdir -p $(RELEASE_BINDIR)/$(PROJ_BINSUBDIR)
	$(SILENT)cp $@ $(RELEASE_BINDIR)/$(PROJ_BINSUBDIR)

ifneq ($(_NAME),)
_EXES = $(foreach build,$(BUILDS),$(build)/$(_NAME))
endif

.PHONY: $(EXES) $(LIBS)
$(LIBS) $(EXES):
	@echo
	@echo "***********************************"
	@echo "Building $@"
	@echo "***********************************"
	$(VSILENT)$(MAKE) $(MAKE_OPT) $(MAKE_JOB_FLAGS) \
		objdirs $(foreach build,$(BUILDS),$($(build)_DIR)/$@) \
		_NAME=$@ \
		OBJS="$($@_OBJS)" \
		EXTRA_OBJS="$($@_EXTRA_OBJS)" \
		PROJ_CPPFLAGS="$($@_CPPFLAGS) $(PROJ_CPPFLAGS)" \
		PROJ_CXXFLAGS="$($@_CXXFLAGS) $(PROJ_CXXFLAGS)" \
		PROJ_CFLAGS="$($@_CFLAGS) $(PROJ_CFLAGS)" \
		OBJ_DEPS="$($@_OBJS_DEPS)" \
		DEPS="$($@_DEPS)" \
		PROJ_LDLIBS="$($@_LDLIBS) $(PROJ_LDLIBS)" \
		PROJ_LDFLAGS="$($@_LDFLAGS) $(PROJ_LDFLAGS)" \
		PROJ_BINSUBDIR="$($@_BINSUBDIR)"

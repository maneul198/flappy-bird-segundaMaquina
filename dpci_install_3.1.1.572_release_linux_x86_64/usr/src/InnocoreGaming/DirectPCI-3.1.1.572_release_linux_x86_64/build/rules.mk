###############################################################################
#
# $Id: rules.mk 9876 2013-05-07 14:27:47Z aidan $
#
# Copyright 2006-2013 Advantech Co Limited.
# All rights reserved.
#
# Description:
# Automatically include rules for appropriate OS/Compiler combination.
#
###############################################################################

BUILDS		?= DEBUG RELEASE
TARGET		?= all
ALL		?= $(AUTOBUILD_H) first subdirs libs exes drivers _target_specials final
LDLIBS		?= 

# These are options that may be overridden for debugging the build process.
#
SILENT		= @
VSILENT		= @
MAKE_OPT	+= --no-print-directory
export SILENT VSILENT

RELEASE_DIR	= RELEASE_$(ARCH)
DEBUG_DIR	= DEBUG_$(ARCH)
export RELEASE_DIR DEBUG_DIR

ifneq ($(_NAME),)
RELEASE_OBJDIR	= $(RELEASE_DIR)/obj_$(_NAME)$(_KTAG)
DEBUG_OBJDIR	= $(DEBUG_DIR)/obj_$(_NAME)$(_KTAG)
RELEASE_OBJS	= $(patsubst %$(OBJ_SUFFIX),$(RELEASE_OBJDIR)/%$(OBJ_SUFFIX),$(OBJS))
DEBUG_OBJS	= $(patsubst %$(OBJ_SUFFIX),$(DEBUG_OBJDIR)/%$(OBJ_SUFFIX),$(OBJS))

.PRECIOUS:	$(RELEASE_OBJS) $(DEBUG_OBJS)

_DEPS_LIBPATHS	:= $(shell $(SRCROOT_X)/build/getdepends.pl -v -s $(SRCROOT_X) -d $(PROJ_ROOT) -p $(ARCH) -L)
_DEPS_INCPATHS	:= $(shell $(SRCROOT_X)/build/getdepends.pl -v -s $(SRCROOT_X) -d $(PROJ_ROOT) -p $(ARCH) -I)
export _DEPS_LIBPATHS _DEPS_INCPATHS
endif

FIXAT		= "$(SRCROOT)/build/fixatat.sh"
export FIXAT

# Rule to do EVERYTHING automagically.
#
.PHONY: all
all: $(ALL)

# Include OS-specific stuff
#
ifneq ($(TARGET_OS),)
 include $(SRCROOT)/build/build-$(TARGET_OS).mk
 include $(SRCROOT)/build/rules-$(TARGET_OS).mk
else
 include $(SRCROOT)/build/rules-$(RULESET).mk
endif

MAKEFILE_NAME	?= Makefile

# Allow makefiles to specify object-level dependencies to be met.
#
ifneq ($(OBJS),)
$(RELEASE_OBJS) $(DEBUG_OBJS): $(OBJ_DEPS) $(PROJ_OBJ_DEPS) $(PROJ_DEPS) $(MAKEFILE_NAME)

.PRECIOUS:      $(RELEASE_OBJS) $(DEBUG_OBJS)

endif

.PHONY: objdirs
ifneq ($(_NAME),)
objdirs: $(RELEASE_OBJDIR) $(DEBUG_OBJDIR)
else
objdirs: $(RELEASE_DIR) $(DEBUG_DIR)
endif

# Basic rules for 'stuff'
#
.PHONY: libs exes drivers _target_specials first final
first: check-build-config $(FIRST)
libs: check-build-config $(LIBS)
exes: check-build-config $(EXES)
drivers: check-build-config $(DRIVERS)
_target_specials: $(_TARGET_SPECIALS)
final: check-build-config objdirs $(OTHER) $(FINAL)


# Miscellaneous rules.
#
$(BUILDCONFIG_FILE):
	$(SILENT)echo $(BUILDCONFIG) >$@

.PHONY: check-build-config
check-build-config: $(BUILDCONFIG_FILE)
	$(SILENT)if [ "$(BUILDCONFIG)" != "`cat $(BUILDCONFIG_FILE)`" ]; then \
		echo; \
		echo "** Build configuration has changed: full recompile required. ** "; \
		echo; \
		echo $(BUILDCONFIG) >$(BUILDCONFIG_FILE); \
	fi

%.sha1sum: %
	$(SILENT)sha1sum "$^" >"$@"

%.md5sum: %
	$(SILENT)md5sum "$^" >"$@"

%.tar.bz2: %
	$(SILENT)tar cf - "$<" | bzip2 -9 >"$@"

%.tar.gz: %
	$(SILENT)tar cf - "$<" | gzip -9 >"$@"

%.tar: %
	$(SILENT)tar cf "$@" "$<"

%.zip: %
	$(SILENT)zip -q -9 -r "$@" "$<"

.PHONY: showvars
showvars:
	@echo "BUILDS = " $(BUILDS)
	@echo "ARCH_CPPFLAGS = " $(ARCH_CPPFLAGS)
	@echo "ARCH_CFLAGS = " $(ARCH_CFLAGS)
	@echo "ARCH_LDFLAGS = " $(ARCH_LDFLAGS)
	@echo "ARCH_LDNFLAGS = " $(ARCH_LDNFLAGS)
	@echo "BSYS_CPPFLAGS = " $(BSYS_CPPFLAGS)
	@echo "BSYS_CFLAGS = " $(BSYS_CFLAGS)
	@echo "BSYS_LDFLAGS = " $(BSYS_LDFLAGS)
	@echo "BSYS_LDNFLAGS = " $(BSYS_LDNFLAGS)
	@echo "PROJ_CPPFLAGS = " $(PROJ_CPPFLAGS)
	@echo "PROJ_CFLAGS = " $(PROJ_CFLAGS)
	@echo "PROJ_LDFLAGS = " $(PROJ_LDFLAGS)
	@echo "PROJ_LDLIBS = " $(PROJ_LDLIBS)
	@echo "LIBS = " $(LIBS)
	@echo "EXES = " $(EXES)
	@echo "_TARGET_SPECIALS = " $(_TARGET_SPECIALS)
ifneq ($(_NAME),)
	@echo "_NAME = \"$(_NAME)\""
	@echo "_LIBS = " $(_LIBS)
	@echo "_EXES = " $(_EXES)
	@echo "DEBUG_OBJS = " $(DEBUG_OBJS)
	@echo "RELEASE_OBJS = " $(RELEASE_OBJS)
endif


# Cleaning and clobbering...
#
.PHONY: clean_% new_%
clean_%:
	$(VSILENT)$(MAKE) $(MAKE_OPT) -C `echo $@|sed s/^clean_//` \
			SILENT=$(SILENT) \
			clean

new_%:
	$(VSILENT)$(MAKE) $(MAKE_OPT) -C `echo $@|sed s/^new_//` \
			SILENT=$(SILENT) \
			clean all

.PHONY: clobber clobber_%
clobber: $(CLOBBER)
	@echo "Clobbering."
	-$(SILENT)$(RM) -r $(RELEASE_DIR) $(DEBUG_DIR)
	$(VSILENT)$(MAKE) $(MAKE_OPT) subdirs TARGET=clobber
	
clobber_%:
	$(VSILENT)$(MAKE) $(MAKE_OPT) -C `echo $@|sed s/^clobber_//` \
			SILENT=$(SILENT) \
			clobber

.PHONY: nuke nuke_%
nuke: $(CLOBBER)
	@echo "Nuking everything."
	-$(SILENT)$(RM) -r RELEASE* DEBUG*
	$(VSILENT)$(MAKE) $(MAKE_OPT) subdirs TARGET=nuke
	
nuke_%:
	$(VSILENT)$(MAKE) $(MAKE_OPT) -C `echo $@|sed s/^nuke_//` \
			SILENT=$(SILENT) \
			nuke

.PHONY: showvars_%
showvars_%:
	$(VSILENT)$(MAKE) $(MAKE_OPT) -C `echo $@|sed s/^showvars_//` \
			SILENT=$(SILENT) \
			showvars


# Handle subdirectories.
#
.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
	@:

$(SUBDIRS):
	@echo "+$@"
	$(VSILENT)$(MAKE) $(MAKE_OPT) -C $@ SILENT=$(SILENT) $(TARGET) _SUBDIR=$@
	@echo "-$@"

# If the makefile changes then this makes sure things get rebuilt automagically.
#
ifneq ($(_NAME),)
$(_EXES) $(_LIBS) $(_DRIVERS) $(OTHER) $(RELEASE_OBJS) $(DEBUG_OBJS): $(MAKEFILE_NAME) $(SRCROOT_X)/build/rules.mk $(SRCROOT_X)/build/build.mk $(BUILD_H) $(VERSION_H)
endif

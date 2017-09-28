###############################################################################
#
# $Id: Makefile 11958 2015-09-23 11:38:03Z aidan $
#
# Copyright 2003-2012 Advantech Corporation Limited.
# All rights reserved.
#
# Description:
# Makefile for DPCI core kernel 2.6 module
#
###############################################################################


DRIVERS = dpci_core.ko
IDLPPSWFLAGS= $(shell if test -n "$(IDLP_PSW)"; then echo -DIDLP_PSW;fi)

dpci_core.ko_OBJS =	dpci_core_module.o \
			dpci_core_fileops.o \
			dpci_core_regintr.o \
			dpci_core_digio.o \
			dpci_core_idlp.o \
			dpci_core_ow.o \
			dpci_core_i2c.o \
			dpci_core_events.o \
			dpci_boards.o

dpci_core.ko_SRCS =	dpci_core_module.c \
			dpci_core_fileops.c \
			dpci_core_regintr.c \
			dpci_core_digio.c \
			dpci_core_idlp.c \
			dpci_core_ow.c \
			dpci_core_i2c.c \
			dpci_core_events.c \
			$(COMMON)/api/dpci_boards.c \
			dpci_core_priv.h
dpci_core.ko_DEPS =	$(IDLPPWD_H)
dpci_core.ko_CFLAGS =	$(IDLPPSWFLAGS)
dpci_core.ko_CPPFLAGS	= -I$(PROJ_ROOT)/../common/driver/include \
		-I$(PROJ_ROOT)/../common/driver/dpci_core \
		-I$(PROJ_ROOT)/driver/include

ifneq ($(IDLP_PSW),)
IDLPPWD_H = idlppwd.h
IDLP_DATE_PWD = "igldate"
IDLP_EVENT_PWD = "iglevent"
endif

.PHONY: idlppwd.h
idlppwd.h:
	rm -f $@
	echo "/* SHA1 hash of idlp password for setdate */ " >$@
	echo -n "static const unsigned char idlp_date_psw[] __attribute__((section(\".text\"))) =  {" >>$@
	echo -n $(IDLP_DATE_PWD)| sha1sum | sed -e 's/ .*//g' -e 's/\(..\)/0x\1, /g' >>$@
	echo -n "};" >>$@
	echo '\n' >>$@
	echo "/* SHA1 hash of idlp password for reading events */ " >>$@
	echo -n "static const unsigned char idlp_event_psw[] __attribute__((section(\".text\"))) =  {" >>$@
	echo -n $(IDLP_EVENT_PWD)| sha1sum | sed -e 's/ .*//g' -e 's/\(..\)/0x\1, /g' >>$@
	echo -n "};" >>$@
	echo '\n' >>$@


include $(SRCROOT)/build/rules.mk

################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of LogCfg.
# Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

config-in  := Config.in
config-h   := $(PACKAGE)/config.h
config-obj := config.o

HEADERDIR := $(CURDIR)/include
headers   := $(PACKAGE)/priv/cdefs.h

subdirs   := logcfgd

ifeq ($(CONFIG_LOGCFG_UTEST),y)
subdirs   += test
test-deps := logcfgd
endif # ($(CONFIG_LOGCFG_UTEST),y)

define liblogcfg_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: liblogcfg
Description: logcfg library
Version: %%PKG_VERSION%%
Requires.private: libdpack
Cflags: -I$${includedir}
Libs: -L$${libdir} -llogcfg
endef

pkgconfigs        := liblogcfg.pc
liblogcfg.pc-tmpl := liblogcfg_pkgconf_tmpl

################################################################################
# Source code tags generation
################################################################################

tagfiles := $(shell find $(addprefix $(CURDIR)/,$(subdirs)) \
                    $(HEADERDIR) \
                    -type f)

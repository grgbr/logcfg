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
headers   += $(PACKAGE)/common.h
headers   += $(PACKAGE)/selector.h

subdirs      += common

subdirs      += dbase
dbase-deps   := common

subdirs      += bin
bin-deps     := dbase

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

################################################################################
# Documentation generation
################################################################################

#doxyconf  := $(CURDIR)/sphinx/Doxyfile
#doxyenv   := SRCDIR="$(HEADERDIR) $(addprefix $(CURDIR)/,$(subdirs))"
#
#sphinxsrc := $(CURDIR)/sphinx
#sphinxenv := \
#	VERSION="$(VERSION)" \
#	$(if $(strip $(EBUILDDOC_TARGET_PATH)), \
#	     EBUILDDOC_TARGET_PATH="$(strip $(EBUILDDOC_TARGET_PATH))") \
#	$(if $(strip $(EBUILDDOC_INVENTORY_PATH)), \
#	     EBUILDDOC_INVENTORY_PATH="$(strip $(EBUILDDOC_INVENTORY_PATH))")

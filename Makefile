################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of LogCfg
# Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

override PACKAGE := logcfg
override VERSION := 1.0
EXTRA_CFLAGS     := -O2 -DNDEBUG
EXTRA_LDFLAGS    := $(EXTRA_CFLAGS)

export VERSION EXTRA_CFLAGS EXTRA_LDFLAGS

ifeq ($(strip $(EBUILDDIR)),)
ifneq ($(realpath ebuild/main.mk),)
EBUILDDIR := $(realpath ebuild)
else  # ($(realpath ebuild/main.mk),)
EBUILDDIR := $(realpath /usr/share/ebuild)
endif # !($(realpath ebuild/main.mk),)
endif # ($(strip $(EBUILDDIR)),)

ifeq ($(realpath $(EBUILDDIR)/main.mk),)
$(error '$(EBUILDDIR)': no valid eBuild install found !)
endif # ($(realpath $(EBUILDDIR)/main.mk),)

include $(EBUILDDIR)/main.mk

ifeq ($(CONFIG_LOGCFG_UTEST),y)

ifeq ($(strip $(CROSS_COMPILE)),)

check_lib_search_path := \
	$(BUILDDIR)/src$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))

CHECK_FORCE ?= y
ifneq ($(filter y 1,$(CHECK_FORCE)),)
.PHONY: $(BUILDDIR)/test/logcfg-utest.xml
endif

CHECK_HALT_ON_FAIL ?= n
ifeq ($(filter y 1,$(CHECK_HALT_ON_FAIL)),)
K := --keep-going
else
K := --no-keep-going
endif

CHECK_VERBOSE ?= --silent

.PHONY: check
check: $(BUILDDIR)/test/logcfg-utest.xml

$(BUILDDIR)/test/logcfg-utest.xml: | build-check
	@echo "  CHECK   $(@)"
	$(Q)env LD_LIBRARY_PATH="$(check_lib_search_path)" \
	        $(BUILDDIR)/test/logcfg-utest \
	        $(CHECK_VERBOSE) \
	        --xml='$(@)' \
	        run

clean-check: _clean-check

.PHONY: _clean-check
_clean-check:
	$(call rm_recipe,$(BUILDDIR)/test/logcfg-utest.xml)

else  # ifneq ($(strip $(CROSS_COMPILE)),)

.PHONY: check
check:
	$(error Cannot check while cross building !)

endif # ifeq ($(strip $(CROSS_COMPILE)),)

else  # ifneq ($(CONFIG_LOGCFG_UTEST),y)

.PHONY: check
check:

endif # ifeq ($(CONFIG_LOGCFG_UTEST),y)

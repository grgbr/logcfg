################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of LogCfg.
# Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

common-cflags  := -Wall \
                  -Wextra \
                  -Wformat=2 \
                  -Wconversion \
                  -Wundef \
                  -Wshadow \
                  -Wcast-qual \
                  -Wcast-align \
                  -Wmissing-declarations \
                  -D_GNU_SOURCE \
                  -I $(TOPDIR)/include \
                  $(EXTRA_CFLAGS) \
                  -fvisibility=hidden
common-ldflags := $(common-cflags) \
                  $(EXTRA_LDFLAGS) \
                  -Wl,-z,start-stop-visibility=hidden

# When assertions are enabled, ensure NDEBUG macro is not set to enable glibc
# assertions.
ifneq ($(filter y,$(CONFIG_LOGCFG_ASSERT_API) $(CONFIG_LOGCFG_ASSERT_INTERN)),)
common-cflags  := $(filter-out -DNDEBUG,$(common-cflags))
common-ldflags := $(filter-out -DNDEBUG,$(common-ldflags))
endif # ($(filter y,$(CONFIG_LOGCFG_ASSERT_API) $(CONFIG_LOGCFG_ASSERT_INTERN)),)

bins := logcfgd

logcfgd-objs    := main.o conf.o rule.o
logcfgd-cflags  := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic
logcfgd-ldflags := $(common-ldflags)
logcfgd-pkgconf := libelog libutils libstroll libconfig

# vim: filetype=make :

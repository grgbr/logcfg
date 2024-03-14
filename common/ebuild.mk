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
                  -I$(TOPDIR) \
                  -I$(TOPDIR)/include \
                  $(EXTRA_CFLAGS) \
                  -fvisibility=hidden
common-ldflags := $(common-cflags) \
                  $(EXTRA_LDFLAGS) \
                  -Wl,--as-needed \
                  -Wl,-z,start-stop-visibility=hidden

# When assertions are enabled, ensure NDEBUG macro is not set to enable glibc
# assertions.
ifneq ($(filter y,$(CONFIG_LOGCFG_ASSERT_API) $(CONFIG_LOGCFG_ASSERT_INTERN)),)
common-cflags  := $(filter-out -DNDEBUG,$(common-cflags))
common-ldflags := $(filter-out -DNDEBUG,$(common-ldflags))
endif # ($(filter y,$(CONFIG_LOGCFG_ASSERT_API) $(CONFIG_LOGCFG_ASSERT_INTERN)),)

solibs                      := liblogcfg_common.so
liblogcfg_common.so-objs    += shared/init.o \
                               shared/session.o \
                               shared/mapper.o \
                               shared/selector.o \
                               shared/rule.o \
                               shared/conf.o
liblogcfg_common.so-cflags  := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic
liblogcfg_common.so-ldflags := $(filter-out -fpie -fPIE,$(common-ldflags)) \
                               -shared -fpic -Bsymbolic \
                               -Wl,-soname,liblogcfg_common.so
liblogcfg_common.so-pkgconf := libdmod \
                               libdpack \
                               libelog \
                               libutils \
                               libstroll \
                               libconfig

arlibs                      := liblogcfg_common.a
liblogcfg_common.a-objs     := static/init.o \
                               static/session.o \
                               static/mapper.o \
                               static/selector.o \
                               static/rule.o \
                               static/conf.o
liblogcfg_common.a-cflags   := $(common-cflags)
liblogcfg_common.a-pkgconf  := libdmod \
                               libdpack \
                               libelog \
                               libutils \
                               libstroll \
                               libconfig

# vim: filetype=make :

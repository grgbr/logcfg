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
                  -I $(TOPDIR) \
                  $(EXTRA_CFLAGS) \
                  -fvisibility=hidden
common-ldflags := $(common-cflags) \
                  -L $(BUILDDIR)/../common \
                  $(EXTRA_LDFLAGS) \
                  -Wl,--as-needed \
                  -Wl,-z,start-stop-visibility=hidden

# When assertions are enabled, ensure NDEBUG macro is not set to enable glibc
# assertions.
ifneq ($(filter y,$(CONFIG_LOGCFG_ASSERT_API) $(CONFIG_LOGCFG_ASSERT_INTERN)),)
common-cflags  := $(filter-out -DNDEBUG,$(common-cflags))
common-ldflags := $(filter-out -DNDEBUG,$(common-ldflags))
endif # ($(filter y,$(CONFIG_LOGCFG_ASSERT_API) $(CONFIG_LOGCFG_ASSERT_INTERN)),)

builtins := builtin.a

builtin.a-objs    := session.o kvstore.o selector.o rule.o
builtin.a-cflags  := $(common-cflags)
builtin.a-pkgconf := libdmod libkvstore libdpack libstroll libconfig

# vim: filetype=make :

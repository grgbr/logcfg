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
                  -D LOGCFG_SYSCONFIGDIR='"$(SYSCONFDIR)/logcfg"' \
                  -D LOGCFG_LOCALSTATEDIR='"$(LOCALSTATEDIR)/logcfg"' \
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

bins := logcfgd

logcfgd-objs    := daemon.o gen.o
logcfgd-cflags  := $(common-cflags)
logcfgd-ldflags := $(common-ldflags) \
                   $(BUILDDIR)/../dbase/builtin.a \
                   -llogcfg_common
logcfgd-pkgconf := libkvstore libdpack libelog libutils libstroll libconfig

$(BUILDDIR)/logcfgd: $(BUILDDIR)/../common/liblogcfg_common.so \
                     $(BUILDDIR)/../dbase/builtin.a

bins := logcfg-clui
logcfg-clui-objs    := clui.o rule.o
logcfg-clui-cflags  := $(common-cflags)
logcfg-clui-ldflags := $(common-ldflags) \
                       $(BUILDDIR)/../dbase/builtin.a \
                       -llogcfg_common
logcfg-clui-pkgconf := libclui \
                       libkvstore \
                       libdpack \
                       libelog \
                       libutils \
                       libstroll \
                       libconfig

$(BUILDDIR)/logcfg-clui: $(BUILDDIR)/../common/liblogcfg_common.so \
                         $(BUILDDIR)/../dbase/builtin.a

install: $(SYSCONFDIR)/logcfg/logcfgd.conf

.PHONY: $(SYSCONFDIR)/logcfg/logcfgd.conf
$(SYSCONFDIR)/logcfg/logcfgd.conf: $(TOPDIR)/logcfgd.conf
	$(call install_recipe,--mode=644,$(<),$(@))

uninstall: _uninstall_data

.PHONY: _uninstall_data
_uninstall_data:
	$(call rm_recipe,$(SYSCONFDIR)/logcfg/logcfgd.conf)

# vim: filetype=make :

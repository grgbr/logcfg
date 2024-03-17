/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "common.h"
#include <kvstore/store.h>

struct elog * logcfg_logger;

const char *
logcfg_strerror(int err)
{
#warning FIXME: make me independent of kvstore (also remove kvstore dependency \
                of liblogcfg_common from ebuild.mk)
	/* kvs_strerror() also handles negative system errno codes. */
	return kvs_strerror(err);
}

void
logcfg_init(struct elog * logger)
{
	logcfg_assert_api(logger);

	logcfg_logger = logger;
}

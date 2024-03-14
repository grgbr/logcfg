/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "common.h"

struct elog * logcfg_logger;

void
logcfg_init(struct elog * logger)
{
	logcfg_assert_api(logger);

	logcfg_logger = logger;
}

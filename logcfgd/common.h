/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_INTERN_COMMON_H
#define _LOGCFG_INTERN_COMMON_H

#include "logcfg/priv/cdefs.h"
#include <elog/elog.h>

#if defined(CONFIG_LOGCFG_ASSERT_API)

#include <stroll/assert.h>

#define logcfg_assert_api(_cond) \
	stroll_assert("logcfg", _cond)

#else  /* !defined(CONFIG_LOGCFG_ASSERT_API) */

#define logcfg_assert_api(_cond)

#endif /* defined(CONFIG_LOGCFG_ASSERT_API) */

#if defined(CONFIG_LOGCFG_ASSERT_INTERN)

#define logcfg_assert_intern(_cond) \
	stroll_assert("logcfg", _cond)

#else  /* !defined(CONFIG_LOGCFG_ASSERT_INTERN) */

#define logcfg_assert_intern(_cond) \
	do {} while (0)

#endif /* defined(CONFIG_LOGCFG_ASSERT_INTERN) */

extern struct elog * logcfg_logger;

#define logcfg_err(_format, ...) \
	elog_err(logcfg_logger, _format ".", ## __VA_ARGS__)

#if defined(CONFIG_LOGCFG_DEBUG)

#define logcfg_debug(_format, ...) \
	elog_debug(logcfg_logger, _format ".", ## __VA_ARGS__)

#else  /* !defined(CONFIG_LOGCFG_DEBUG) */

#define logcfg_debug(_format, ...)

#endif /* defined(CONFIG_LOGCFG_DEBUG) */

#endif /* _LOGCFG_INTERN_COMMON_H */

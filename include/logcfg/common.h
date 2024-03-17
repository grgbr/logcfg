/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_COMMON_H
#define _LOGCFG_COMMON_H

#include <logcfg/priv/cdefs.h>

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

extern const char *
logcfg_strerror(int err) __logcfg_export;

struct elog;

extern void
logcfg_init(struct elog * logger)
	__logcfg_nonull(1) __logcfg_export;

#endif /* _LOGCFG_COMMON_H */

/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_COMMON_SESSION_H
#define _LOGCFG_COMMON_SESSION_H

#include <logcfg/session.h>

#define logcfg_session_ops_assert_intern(_ops) \
	logcfg_assert_intern(_ops); \
	logcfg_assert_intern((_ops)->fini)

#define logcfg_session_assert_intern(_session) \
	logcfg_assert_intern(_session); \
	logcfg_session_ops_assert_intern((_session)->ops)

extern void
logcfg_session_init(struct logcfg_session *               session,
                    const struct logcfg_session_ops *     sess_ops,
                    const struct logcfg_mapper_repo_ops * maps_ops)
	__logcfg_nonull(1, 2, 3) __logcfg_export;

#endif /* _LOGCFG_COMMON_SESSION_H */

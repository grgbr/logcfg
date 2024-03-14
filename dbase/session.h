/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_DBASE_SESSION_H
#define _LOGCFG_DBASE_SESSION_H

#include "common/session.h"

struct kvs_repo;

struct logcfg_dbase_session {
	struct logcfg_session base;
	struct kvs_repo *     kvdb;
};

static inline struct kvs_repo *
logcfg_dbase_session_get_kvdb(const struct logcfg_session * session)
{
	logcfg_session_assert_intern(session);
	logcfg_assert_intern(((const struct logcfg_dbase_session *)
	                      session)->kvdb);

	return ((const struct logcfg_dbase_session *)session)->kvdb;
}

extern struct logcfg_session *
logcfg_dbase_session_create(struct kvs_repo * kvdb);

#endif /* _LOGCFG_DBASE_SESSION_H */

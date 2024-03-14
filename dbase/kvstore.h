/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_DBASE_KVSTORE_H
#define _LOGCFG_DBASE_KVSTORE_H

#include "common/common.h"
#include <kvstore/repo.h>

enum logcfg_dbase_table_id {
	LOGCFG_DBASE_SELECTOR_TID = 0,
	LOGCFG_DBASE_TID_NR
};

static inline const char *
logcfg_dbase_strerror(int err)
{
	logcfg_assert_api(!err);

	return kvs_strerror(err);
}

static inline int
logcfg_dbase_begin_xact(const struct kvs_repo * repo,
                        const struct kvs_xact * parent,
                        struct kvs_xact *       xact,
                        unsigned int            flags)
{
	logcfg_assert_api(repo);
	logcfg_assert_api(xact);

	return kvs_begin_xact(&repo->depot, parent, xact, flags);
}

static inline const struct kvs_table *
logcfg_dbase_get_table(const struct kvs_repo *    repo,
                       enum logcfg_dbase_table_id tid)
{
	logcfg_assert_api(repo);
	logcfg_assert_api(tid >= 0);
	logcfg_assert_api(tid < LOGCFG_DBASE_TID_NR);

	return kvs_repo_get_table(repo, (unsigned int)tid);
}

extern int
logcfg_dbase_open(struct kvs_repo * repo, const char * path);

extern int
logcfg_dbase_close(const struct kvs_repo * repo);

extern struct kvs_repo *
logcfg_dbase_create(void);

extern void
logcfg_dbase_destroy(struct kvs_repo *repo);

#endif /* _LOGCFG_DBASE_KVSTORE_H */

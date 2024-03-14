/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_DBASE_SELECTOR_H
#define _LOGCFG_DBASE_SELECTOR_H

#include "common/selector.h"
#include <kvstore/table.h>

struct logcfg_session;

extern const struct kvs_table_desc logcfg_dbase_selector_table;

extern struct logcfg_selector_mapper *
logcfg_selector_dbase_mapper_create(const struct logcfg_session * session);

extern void
logcfg_selector_dbase_mapper_destroy(struct logcfg_selector_mapper * mapper);

#endif /* _LOGCFG_DBASE_SELECTOR_H */

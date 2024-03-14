/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_DBASE_RULE_H
#define _LOGCFG_DBASE_RULE_H

#include "common/rule.h"
#include <libconfig.h>

extern int
logcfg_rule_load_conf(const config_setting_t * __restrict setting);

#if defined(CONFIG_LOGCFG_ASSERT_INTERN)

extern void
logcfg_rule_repo_init(void);

#else  /* !defined(CONFIG_LOGCFG_ASSERT_INTERN) */

static inline void
logcfg_rule_repo_init(void)
{
}

#endif /* defined(CONFIG_LOGCFG_ASSERT_INTERN) */

extern void
logcfg_rule_repo_fini(void);

struct logcfg_session;

extern struct logcfg_rule_mapper *
logcfg_rule_dbase_mapper_create(const struct logcfg_session * session __unused);

extern void
logcfg_rule_dbase_mapper_destroy(struct logcfg_rule_mapper * mapper);

#endif /* _LOGCFG_DBASE_RULE_H */

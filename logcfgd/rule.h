/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_INTERN_RULE_H
#define _LOGCFG_INTERN_RULE_H

#include "conf.h"

#define LOGCFG_RULE_NAME_MAX  (16U)
#define LOGCFG_RULE_MATCH_MAX (256U)

struct logcfg_rule {
	char   name[LOGCFG_RULE_NAME_MAX];
	char * match;
};

struct logcfg_rule_repo {
	unsigned int         nr;
	struct logcfg_rule * rules;
};

extern struct logcfg_rule_repo logcfg_the_rule_repo;

#define logcfg_rule_foreach(_rule) \
	for ((_rule) = logcfg_the_rule_repo.rules; \
	     (_rule) < &logcfg_the_rule_repo.rules[logcfg_the_rule_repo.nr]; \
	     (_rule)++)

extern const struct logcfg_rule *
logcfg_rule_get_byname(const char * name);

#if defined(CONFIG_LOGCFG_ASSERT_INTERN)

extern const struct logcfg_rule *
logcfg_rule_get_byid(unsigned int id);

extern void
logcfg_rule_init_repo(void);

#else  /* !defined(CONFIG_LOGCFG_ASSERT_INTERN) */

static inline const struct logcfg_rule *
logcfg_rule_get_byid(unsigned int id)
{
	if (id < logcfg_the_rule_repo.nr)
		return &logcfg_the_rule_repo.rules[id];

	return NULL;
}

static inline void
logcfg_rule_init_repo(void)
{
}

#endif /* defined(CONFIG_LOGCFG_ASSERT_INTERN) */

extern void
logcfg_rule_fini_repo(void);

#if defined(CONFIG_LOGCFG_ASSERT_INTERN)

extern unsigned int
logcfg_rule_get_id(const struct logcfg_rule * __restrict rule);

extern const char *
logcfg_rule_get_name(const struct logcfg_rule * __restrict rule);

extern const char *
logcfg_rule_get_match(const struct logcfg_rule * __restrict rule);

extern void
logcfg_rule_fini(struct logcfg_rule * __restrict rule);

#else  /* !defined(CONFIG_LOGCFG_ASSERT_INTERN) */

static inline unsigned int
logcfg_rule_get_id(const struct logcfg_rule * __restrict rule)
{
	return (unsigned int)(rule - logcfg_the_rule_repo.rules);
}

static inline const char *
logcfg_rule_get_name(const struct logcfg_rule * __restrict rule)
{
	return rule->name;
}

static inline const char *
logcfg_rule_get_match(const struct logcfg_rule * __restrict rule)
{
	return rule->match;
}

#endif /* defined(CONFIG_LOGCFG_ASSERT_INTERN) */

extern int __logcfg_nonull(1)
logcfg_rule_load_conf(const config_setting_t * __restrict setting);

#endif /* _LOGCFG_INTERN_RULE_H */

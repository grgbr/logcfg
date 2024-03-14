/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_RULE_H
#define _LOGCFG_RULE_H

#include <logcfg/common.h>
#include <dmod/dmod.h>

/******************************************************************************
 * Rule object
 ******************************************************************************/

#define LOGCFG_RULE_NAMESZ_MAX  (16U)
#define LOGCFG_RULE_MATCHSZ_MAX (256U)

struct logcfg_rule {
	struct dmod_object dmod;
	unsigned int       id;
	char               name[LOGCFG_RULE_NAMESZ_MAX];
	char *             match;
};

extern int
logcfg_rule_check_name(const char * __restrict name)
	__logcfg_nonull(1) __logcfg_export;

#if defined(CONFIG_LOGCFG_ASSERT_API)

extern unsigned int
logcfg_rule_get_id(const struct logcfg_rule * __restrict rule)
	__logcfg_nonull(1) __logcfg_export;

extern const char *
logcfg_rule_get_name(const struct logcfg_rule * __restrict rule)
	__logcfg_nonull(1) __logcfg_export;

extern const char *
logcfg_rule_get_match(const struct logcfg_rule * __restrict rule)
	__logcfg_nonull(1) __logcfg_export;

#else  /* !defined(CONFIG_LOGCFG_ASSERT_API) */

static inline unsigned int
logcfg_rule_get_id(const struct logcfg_rule * __restrict rule)
{
	return rule->id;
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

#endif /* defined(CONFIG_LOGCFG_ASSERT_API) */

/******************************************************************************
 * Rule object mapper interface
 ******************************************************************************/

struct logcfg_rule_mapper;
struct dmod_const_iter;

extern const struct logcfg_rule *
logcfg_rule_get_byid(struct logcfg_rule_mapper * mapper, unsigned int id)
	__logcfg_nonull(1) __logcfg_export;

extern const struct logcfg_rule *
logcfg_rule_get_byname(struct logcfg_rule_mapper * mapper, const char * name)
	__logcfg_nonull(1) __logcfg_export;

#define logcfg_rule_iter_foreach(_iter, _rule) \
	for ((_rule) = (const struct logcfg_rule *) \
	               dmod_const_iter_step(_iter); \
	     (_rule) != NULL; \
	     (_rule) = (const struct logcfg_rule *)dmod_const_iter_step(_iter))

extern struct dmod_const_iter *
logcfg_rule_iter(struct logcfg_rule_mapper * mapper)
	__logcfg_nonull(1) __logcfg_export;

#endif /* _LOGCFG_RULE_H */

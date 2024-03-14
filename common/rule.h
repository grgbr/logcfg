/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_COMMON_RULE_H
#define _LOGCFG_COMMON_RULE_H

#include <logcfg/rule.h>

/******************************************************************************
 * Rule object
 ******************************************************************************/

#define logcfg_rule_assert_api(_rule) \
	logcfg_assert_api(_rule); \
	logcfg_assert_api(dmod_object_is_clean(&(_rule)->dmod)); \
	logcfg_assert_api(!logcfg_rule_check_name((_rule)->name)); \
	logcfg_assert_api(!logcfg_rule_check_match((_rule)->match)); \

#define logcfg_rule_assert_intern(_rule) \
	logcfg_assert_intern(_rule); \
	logcfg_assert_intern(dmod_object_is_clean(&(_rule)->dmod)); \
	logcfg_assert_intern(!logcfg_rule_check_name((_rule)->name)); \
	logcfg_assert_intern(!logcfg_rule_check_match((_rule)->match)); \

extern int
logcfg_rule_check_match(const char * match)
	__logcfg_nonull(1) __logcfg_export;

/******************************************************************************
 * Rule object mapper interface
 ******************************************************************************/

typedef const struct logcfg_rule *
        (logcfg_rule_get_byid_fn)(struct logcfg_rule_mapper *, unsigned int);

typedef const struct logcfg_rule *
        (logcfg_rule_get_byname_fn)(struct logcfg_rule_mapper *, const char *);

typedef struct dmod_const_iter *
        (logcfg_rule_iter_fn)(void);

struct logcfg_rule_mapper_ops {
	struct dmod_mapper_ops      dmod;
	logcfg_rule_get_byid_fn *   get_byid;
	logcfg_rule_get_byname_fn * get_byname;
	logcfg_rule_iter_fn *       iter;
};

#define logcfg_rule_mapper_ops_assert_api(_ops) \
	logcfg_assert_api(_ops); \
	dmod_mapper_ops_assert_api(&(_ops)->dmod); \
	logcfg_assert_api((_ops)->get_byid); \
	logcfg_assert_api((_ops)->get_byname); \
	logcfg_assert_api((_ops)->iter)

#define logcfg_rule_mapper_ops_assert_intern(_ops) \
	logcfg_assert_intern(_ops); \
	dmod_mapper_ops_assert_intern(&(_ops)->dmod); \
	logcfg_assert_intern((_ops)->get_byid); \
	logcfg_assert_intern((_ops)->get_byname); \
	logcfg_assert_intern((_ops)->iter)

struct logcfg_rule_mapper {
	struct dmod_mapper dmod;
};

#define logcfg_rule_mapper_assert_api(_mapper) \
	logcfg_assert_api(_mapper); \
	logcfg_rule_mapper_ops_assert_api( \
		logcfg_rule_mapper_to_ops(_mapper))

#define logcfg_rule_mapper_assert_intern(_mapper) \
	logcfg_assert_intern(_mapper); \
	logcfg_rule_mapper_ops_assert_intern( \
		logcfg_rule_mapper_to_ops(_mapper))

static const struct logcfg_rule_mapper_ops *
logcfg_rule_mapper_to_ops(const struct logcfg_rule_mapper * mapper)
{
	logcfg_assert_intern(mapper);

	return (const struct logcfg_rule_mapper_ops *)mapper->dmod.ops;
}
static inline void
logcfg_rule_mapper_init(struct logcfg_rule_mapper *           mapper,
                        const struct logcfg_rule_mapper_ops * ops)
{
	logcfg_assert_intern(mapper);
	logcfg_rule_mapper_ops_assert_intern(ops);

	dmod_mapper_init(&mapper->dmod, &ops->dmod);
}

static inline void
logcfg_rule_mapper_fini(struct logcfg_rule_mapper * mapper)
{
	logcfg_rule_mapper_assert_intern(mapper);

	dmod_mapper_fini(&mapper->dmod);
}

#endif /* _LOGCFG_COMMON_RULE_H */

/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_MAPPER_H
#define _LOGCFG_MAPPER_H

#include <logcfg/common.h>

struct logcfg_session;
struct logcfg_selector_mapper;
struct logcfg_rule_mapper;

typedef struct logcfg_selector_mapper *
               (logcfg_selector_create_mapper_fn)
               (const struct logcfg_session * session);

typedef void (logcfg_selector_destroy_mapper_fn)
             (struct logcfg_selector_mapper *);

typedef struct logcfg_rule_mapper *
               (logcfg_rule_create_mapper_fn)
               (const struct logcfg_session * session);

typedef void (logcfg_rule_destroy_mapper_fn)(struct logcfg_rule_mapper *);

struct logcfg_mapper_repo_ops {
	logcfg_selector_create_mapper_fn *  create_selector;
	logcfg_rule_create_mapper_fn *      create_rule;

	logcfg_selector_destroy_mapper_fn * destroy_selector;
	logcfg_rule_destroy_mapper_fn *     destroy_rule;
};

#define logcfg_mapper_repo_ops_assert_api(_ops) \
	logcfg_assert_api(_ops); \
	logcfg_assert_api((_ops)->create_selector); \
	logcfg_assert_api((_ops)->create_rule); \
	logcfg_assert_api((_ops)->destroy_selector); \
	logcfg_assert_api((_ops)->destroy_rule)

struct logcfg_mapper_repo {
	const struct logcfg_mapper_repo_ops * ops;
	struct logcfg_selector_mapper *       selector;
	struct logcfg_rule_mapper *           rule;
};

#define logcfg_mapper_repo_assert_api(_mappers) \
	logcfg_assert_api(_mappers); \
	logcfg_mapper_repo_ops_assert_api((_mappers)->ops)

static inline struct logcfg_selector_mapper *
logcfg_mapper_get_selector(struct logcfg_mapper_repo *   mappers,
                           const struct logcfg_session * session)
{
	logcfg_mapper_repo_assert_api(mappers);
	logcfg_assert_api(session);

	if (!mappers->selector)
		mappers->selector = mappers->ops->create_selector(session);

	return mappers->selector;
}

static inline struct logcfg_rule_mapper *
logcfg_mapper_get_rule(struct logcfg_mapper_repo *   mappers,
                       const struct logcfg_session * session)
{
	logcfg_mapper_repo_assert_api(mappers);
	logcfg_assert_api(session);

	if (!mappers->rule)
		mappers->rule = mappers->ops->create_rule(session);

	return mappers->rule;
}

#endif /* _LOGCFG_MAPPER_H */

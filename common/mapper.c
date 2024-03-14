/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "mapper.h"
#include <stdlib.h>

#define logcfg_mapper_repo_ops_assert_intern(_ops) \
	logcfg_assert_intern(_ops); \
	logcfg_assert_intern((_ops)->create_selector); \
	logcfg_assert_intern((_ops)->create_rule); \
	logcfg_assert_intern((_ops)->destroy_selector); \
	logcfg_assert_intern((_ops)->destroy_rule)

#define logcfg_mapper_repo_assert_intern(_mappers) \
	logcfg_assert_intern(_mappers); \
	logcfg_mapper_repo_ops_assert_intern((_mappers)->ops)

void
logcfg_mapper_repo_init(struct logcfg_mapper_repo *           mappers,
                        const struct logcfg_mapper_repo_ops * ops)
{
	logcfg_assert_intern(mappers);
	logcfg_mapper_repo_ops_assert_intern(ops);

	mappers->ops = ops;

	mappers->selector = NULL;
	mappers->rule = NULL;
}

void
logcfg_mapper_repo_fini(struct logcfg_mapper_repo * mappers)
{
	logcfg_assert_intern(mappers);
	logcfg_mapper_repo_ops_assert_intern(mappers->ops);

	if (mappers->selector)
		mappers->ops->destroy_selector(mappers->selector);
	if (mappers->rule)
		mappers->ops->destroy_rule(mappers->rule);
}

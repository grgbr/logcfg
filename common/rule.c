/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "rule.h"
#include <string.h>

/******************************************************************************
 * Rule object
 ******************************************************************************/

int
logcfg_rule_check_name(const char * name)
{
	logcfg_assert_api(name);

	size_t len;

	len = strnlen(name, LOGCFG_RULE_NAMESZ_MAX);
	if (!len || len >= LOGCFG_RULE_NAMESZ_MAX)
		return -EINVAL;

	return 0;
}

int
logcfg_rule_check_match(const char * match)
{
	logcfg_assert_api(match);

	size_t len;

	len = strnlen(match, LOGCFG_RULE_MATCHSZ_MAX);
	if (!len || len >= LOGCFG_RULE_MATCHSZ_MAX)
		return -EINVAL;

	return 0;
}

unsigned int
logcfg_rule_get_id(const struct logcfg_rule * __restrict rule)
{
	logcfg_rule_assert_api(rule);

	return rule->id;
}

const char *
logcfg_rule_get_name(const struct logcfg_rule * __restrict rule)
{
	logcfg_rule_assert_api(rule);

	return rule->name;
}

const char *
logcfg_rule_get_match(const struct logcfg_rule * __restrict rule)
{
	logcfg_rule_assert_api(rule);

	return rule->match;
}

/******************************************************************************
 * Rule object mapper interface
 ******************************************************************************/

const struct logcfg_rule *
logcfg_rule_get_byid(struct logcfg_rule_mapper * mapper, unsigned int id)
{
	logcfg_rule_mapper_assert_api(mapper);

	const struct logcfg_rule * rule;

	rule = logcfg_rule_mapper_to_ops(mapper)->get_byid(mapper, id);
	logcfg_rule_assert_intern(rule);

	return rule;
}

const struct logcfg_rule *
logcfg_rule_get_byname(struct logcfg_rule_mapper * mapper, const char * name)
{
	logcfg_rule_mapper_assert_api(mapper);
	logcfg_assert_api(!logcfg_rule_check_name(name));

	const struct logcfg_rule * rule;

	rule = logcfg_rule_mapper_to_ops(mapper)->get_byname(mapper, name);
	logcfg_rule_assert_intern(rule);

	return rule;
}

struct dmod_const_iter *
logcfg_rule_iter(struct logcfg_rule_mapper * mapper)
{
	logcfg_rule_mapper_assert_api(mapper);

	return logcfg_rule_mapper_to_ops(mapper)->iter();
}

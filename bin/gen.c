#include "common/rule.h"
#include <stdio.h>

int
logcfg_gen_rule_decl(FILE * file, const struct logcfg_rule * rule)
{
	logcfg_assert_intern(file);
	logcfg_rule_assert_intern(rule);

	int ret;

	ret = fprintf(file,
	              "filter logcfg_%s_filt { %s; };\n",
	              logcfg_rule_get_name(rule),
	              logcfg_rule_get_match(rule));
	if (ret < 0)
		return -errno;

	return 0;
}

int
logcfg_gen_rule_call(FILE * file, const struct logcfg_rule * rule)
{
	logcfg_assert_intern(file);
	logcfg_rule_assert_intern(rule);

	int ret;

	ret = fprintf(file,
	              "filter(logcfg_%s_filt);\n",
	              logcfg_rule_get_name(rule));
	if (ret < 0)
		return -errno;

	return 0;
}

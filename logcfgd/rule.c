#include "rule.h"
#include <stdlib.h>
#include <string.h>

struct logcfg_rule_repo logcfg_the_rule_repo;

#if defined(CONFIG_LOGCFG_ASSERT_INTERN)

#define logcfg_rule_repo_assert() \
	logcfg_assert_intern(logcfg_the_rule_repo.nr); \
	logcfg_assert_intern(logcfg_the_rule_repo.rules)

#define logcfg_rule_assert_uninit(_rule) \
	logcfg_rule_repo_assert(); \
	logcfg_assert_intern(_rule); \
	logcfg_assert_intern((_rule) >= logcfg_the_rule_repo.rules); \
	logcfg_assert_intern( \
		(_rule) < \
		&logcfg_the_rule_repo.rules[logcfg_the_rule_repo.nr])

#define logcfg_rule_assert(_rule) \
	logcfg_rule_assert_uninit(_rule); \
	logcfg_assert_intern(!(_rule)->match || (_rule)->name[0]); \
	logcfg_assert_intern(!(_rule)->match || \
	                     strnlen((_rule)->match, LOGCFG_RULE_MATCH_MAX) < \
	                     LOGCFG_RULE_MATCH_MAX)

unsigned int
logcfg_rule_get_id(const struct logcfg_rule * __restrict rule)
{
	logcfg_rule_assert(rule);

	return (unsigned int)(rule - logcfg_the_rule_repo.rules);
}

const char *
logcfg_rule_get_name(const struct logcfg_rule * __restrict rule)
{
	logcfg_rule_assert(rule);

	return rule->name;
}

const char *
logcfg_rule_get_match(const struct logcfg_rule * __restrict rule)
{
	logcfg_rule_assert(rule);

	return rule->match;
}

#else  /* !defined(CONFIG_LOGCFG_ASSERT_INTERN) */

#define logcfg_rule_repo_assert()
#define logcfg_rule_assert_uninit(_rule)
#define logcfg_rule_assert(_rule)

#endif /* defined(CONFIG_LOGCFG_ASSERT_INTERN) */

static int
logcfg_rule_init(struct logcfg_rule * __restrict rule,
                 const char * __restrict         name,
                 size_t                          length,
                 const char * __restrict         match)
{
	logcfg_rule_assert_uninit(rule);
	logcfg_assert_intern(!rule->match);
	logcfg_assert_intern(name);
	logcfg_assert_intern(length);
	logcfg_assert_intern(length < sizeof(rule->name));
	logcfg_assert_intern(strlen(name) == length);
	logcfg_assert_intern(match);
	logcfg_assert_intern(strnlen(match, LOGCFG_RULE_MATCH_MAX) <
	                     LOGCFG_RULE_MATCH_MAX);

	rule->match = strdup(match);
	if (!rule->match)
		return -errno;

	memcpy(rule->name, name, length);
	rule->name[length] = '\0';

	return 0;
}

void
logcfg_rule_fini(struct logcfg_rule * __restrict rule)
{
	logcfg_rule_assert(rule);

	free(rule->match);
}

#if defined(CONFIG_LOGCFG_ASSERT_INTERN)

const struct logcfg_rule *
logcfg_rule_get_byid(unsigned int id)
{
	logcfg_rule_repo_assert();

	if (id < logcfg_the_rule_repo.nr) {
		logcfg_rule_assert(&logcfg_the_rule_repo.rules[id]);
		
		return &logcfg_the_rule_repo.rules[id];
	}

	return NULL;
}

#endif /* defined(CONFIG_LOGCFG_ASSERT_INTERN) */

const struct logcfg_rule *
logcfg_rule_get_byname(const char * name)
{
	logcfg_rule_repo_assert();

	const struct logcfg_rule * rule;

#warning Replace me with a proper data structure such as radix tree or \
         string hash table. Maybe a simple sorted array with dichotomic search \
         is enough.

	logcfg_rule_foreach(rule) {
		logcfg_rule_assert(rule);

		if (!strcmp(rule->name, name)) {
			return rule;
		}
	}

	return NULL;
}

static int
logcfg_rule_setup_repo(unsigned int nr)
{
	logcfg_assert_intern(nr);

	logcfg_the_rule_repo.rules = calloc(nr, sizeof(struct logcfg_rule));
	if (!logcfg_the_rule_repo.rules)
		return -errno;

	logcfg_the_rule_repo.nr = nr;

	return 0;
}

#if defined(CONFIG_LOGCFG_ASSERT_INTERN)

void
logcfg_rule_init_repo(void)
{
	logcfg_assert_intern(!logcfg_the_rule_repo.nr);
	logcfg_assert_intern(!logcfg_the_rule_repo.rules);
}

#endif /* defined(CONFIG_LOGCFG_ASSERT_INTERN) */

void
logcfg_rule_fini_repo(void)
{
	unsigned int r;

	for (r = 0; r < logcfg_the_rule_repo.nr; r++)
		logcfg_rule_fini(&logcfg_the_rule_repo.rules[r]);

	free(logcfg_the_rule_repo.rules);
}

static int
logcfg_rule_load_conf_elem(const config_setting_t * __restrict setting)
{
	logcfg_rule_repo_assert();
	logcfg_assert_intern(setting);

	const config_setting_t * attr;
	int                      nr;
	int                      id;
	const char *             name;
	size_t                   nlen;
	const char *             match;
	size_t                   mlen;
	struct logcfg_rule *     rule;

	if (!config_setting_is_group(setting)) {
		logcfg_conf_err(setting, "dictionary required");
		return -EINVAL;
	}

	nr = config_setting_length(setting);
	logcfg_assert_intern(nr >= 0);
	if (nr != 3) {
		/* All rule field definitions are mandatory. */
		logcfg_conf_err(setting, "invalid number of settings");
		return -EINVAL;
	}

	/* Parse rule identifier */
	attr = config_setting_get_member(setting, "id");
	if (!attr) {
		logcfg_conf_err(setting, "missing 'id' setting");
		return -EINVAL;
	}
	if (config_setting_type(attr) != CONFIG_TYPE_INT) {
		logcfg_conf_err(attr, "positive integer required");
		return -EINVAL;
	}
	id = config_setting_get_int(attr);
	if ((id < 0) || ((unsigned int)id >= logcfg_the_rule_repo.nr)) {
		logcfg_conf_err(attr, "out of range integer");
		return -EINVAL;
	}
	if (logcfg_rule_get_byid((unsigned int)id)->match) {
		logcfg_conf_err(attr, "unique identifier required");
		return -EINVAL;
	}

	/* Parse rule name */
	attr = config_setting_get_member(setting, "name");
	if (!attr) {
		logcfg_conf_err(setting, "missing 'name' setting");
		return -EINVAL;
	}
	name = config_setting_get_string(attr);
	if (!name) {
		logcfg_conf_err(attr, "string required");
		return -EINVAL;
	}
	nlen = strnlen(name, sizeof(rule->name));
	if (!nlen || (nlen >= sizeof(rule->name))) {
		logcfg_conf_err(attr, "too long or empty");
		return -EINVAL;
	}
	if (logcfg_rule_get_byname(name)) {
		logcfg_conf_err(attr, "unique name required");
		return -EINVAL;
	}

	/* Parse rule matching expression */
	attr = config_setting_get_member(setting, "match");
	if (!attr) {
		logcfg_conf_err(setting, "missing 'match' setting");
		return -EINVAL;
	}
	match = config_setting_get_string(attr);
	if (!match) {
		logcfg_conf_err(attr, "string required");
		return -EINVAL;
	}
	mlen = strnlen(match, LOGCFG_RULE_MATCH_MAX);
	if (!mlen || (mlen >= LOGCFG_RULE_MATCH_MAX)) {
		logcfg_conf_err(attr, "too long or empty");
		return -EINVAL;
	}

	return logcfg_rule_init(&logcfg_the_rule_repo.rules[id],
	                        name,
	                        nlen,
	                        match);
}

int
logcfg_rule_load_conf(const config_setting_t * __restrict setting)
{
	logcfg_assert_intern(setting);

	int          nr;
	int          err;
	unsigned int r;

	if (!config_setting_is_list(setting)) {
		logcfg_conf_err(setting, "list required");
		return -EINVAL;
	}

	nr = config_setting_length(setting);
	logcfg_assert_intern(nr >= 0);
	if (!nr) {
		/* No entry definition found. */
		logcfg_conf_err(setting, "empty list not allowed");
		return -ENODATA;
	}

	err = logcfg_rule_setup_repo((unsigned int)nr);
	if (err)
		return err;

	for (r = 0; r < (unsigned int)nr; r++) {
		const config_setting_t * set;

		set = config_setting_get_elem(setting, r);
		logcfg_assert_intern(set);

		err = logcfg_rule_load_conf_elem(set);
		if (err)
			return err;
	}

	return 0;
}

#include "rule.h"
#include "common/conf.h"
#include <dmod/iter.h>
#include <stdlib.h>
#include <string.h>

static void
logcfg_rule_fini(struct logcfg_rule * __restrict rule);

/******************************************************************************
 * Internal database rule repository
 ******************************************************************************/

struct logcfg_rule_repo {
	unsigned int         nr;
	struct logcfg_rule * rules;
};

static struct logcfg_rule_repo logcfg_the_rule_repo;

#define logcfg_rule_repo_assert() \
	logcfg_assert_intern(logcfg_the_rule_repo.nr); \
	logcfg_assert_intern(logcfg_the_rule_repo.rules)

static const struct logcfg_rule *
logcfg_rule_repo_get_byname(const char * name, unsigned int count)
{
	logcfg_rule_repo_assert();
	logcfg_assert_intern(!logcfg_rule_check_name(name));

	const struct logcfg_rule * rule;

#warning Replace me with a proper data structure such as radix tree or \
         string hash table. Maybe a simple sorted array with dichotomic search \
         is enough.

	for (rule = logcfg_the_rule_repo.rules;
	     rule < &logcfg_the_rule_repo.rules[count];
	     rule++) {
		logcfg_rule_assert_intern(rule);

		if (!strcmp(rule->name, name))
			return rule;
	}

	return NULL;
}

static int
logcfg_rule_repo_setup(unsigned int nr)
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
logcfg_rule_repo_init(void)
{
	logcfg_assert_intern(!logcfg_the_rule_repo.nr);
	logcfg_assert_intern(!logcfg_the_rule_repo.rules);
}

#endif /* defined(CONFIG_LOGCFG_ASSERT_INTERN) */

void
logcfg_rule_repo_fini(void)
{
	unsigned int r;

	for (r = 0; r < logcfg_the_rule_repo.nr; r++)
		logcfg_rule_fini(&logcfg_the_rule_repo.rules[r]);

	free(logcfg_the_rule_repo.rules);
}

/******************************************************************************
 * Internal database rule instantiation
 ******************************************************************************/

#define logcfg_rule_assert_uninit(_rule) \
	logcfg_rule_repo_assert(); \
	logcfg_assert_intern(_rule); \
	logcfg_assert_intern((_rule) >= logcfg_the_rule_repo.rules); \
	logcfg_assert_intern( \
		(_rule) < \
		&logcfg_the_rule_repo.rules[logcfg_the_rule_repo.nr])

static int
logcfg_rule_init(struct logcfg_rule * __restrict rule,
                 const char * __restrict         name,
                 const char *                    match)
{
	logcfg_rule_assert_uninit(rule);
	logcfg_assert_intern(!rule->match);
	logcfg_assert_intern(!logcfg_rule_check_name(name));
	logcfg_assert_intern(!logcfg_rule_check_match(match));

	rule->match = strdup(match);
	if (!rule->match)
		return -errno;

	rule->dmod.state = DMOD_CLEAN_STATE;
	rule->id = (unsigned int)(rule - logcfg_the_rule_repo.rules);
	strcpy(rule->name, name);

	return 0;
}

static void
logcfg_rule_fini(struct logcfg_rule * __restrict rule)
{
	logcfg_rule_assert_uninit(rule);

	free(rule->match);
}

static int
logcfg_rule_load_conf_elem(const config_setting_t * __restrict setting,
                           unsigned int                        id)
{
	logcfg_rule_repo_assert();
	logcfg_assert_intern(setting);
	logcfg_assert_intern(id < logcfg_the_rule_repo.nr);

	const config_setting_t * attr;
	int                      nr;
	const char *             name;
	const char *             match;

	if (!config_setting_is_group(setting)) {
		logcfg_conf_err(setting, "dictionary required");
		return -EINVAL;
	}

	nr = config_setting_length(setting);
	logcfg_assert_intern(nr >= 0);
	if (nr != 2) {
		/* All rule field definitions are mandatory. */
		logcfg_conf_err(setting, "invalid number of settings");
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
	if (logcfg_rule_check_name(name)) {
		logcfg_conf_err(attr, "too long or empty");
		return -EINVAL;
	}
	if (logcfg_rule_repo_get_byname(name, id)) {
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
	if (logcfg_rule_check_match(match)) {
		logcfg_conf_err(attr, "too long or empty");
		return -EINVAL;
	}

	return logcfg_rule_init(&logcfg_the_rule_repo.rules[id],
	                        name,
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

	err = logcfg_rule_repo_setup((unsigned int)nr);
	if (err)
		return err;

	for (r = 0; r < (unsigned int)nr; r++) {
		const config_setting_t * set;

		set = config_setting_get_elem(setting, r);
		logcfg_assert_intern(set);

		err = logcfg_rule_load_conf_elem(set, r);
		if (err)
			return err;
	}

	return 0;
}

/******************************************************************************
 * Rule database iterator
 ******************************************************************************/

struct logcfg_rule_dbase_iter {
	struct dmod_const_iter dmod;
	unsigned int           curr;
};

static const struct dmod_object *
logcfg_rule_dbase_iter_step(struct dmod_const_iter * iter)
{
	logcfg_assert_intern(iter);

	struct logcfg_rule_dbase_iter * it = (struct logcfg_rule_dbase_iter *)
	                                     iter;

	if (it->curr < logcfg_the_rule_repo.nr)
		return (const struct dmod_object *)
		       &logcfg_the_rule_repo.rules[it->curr++];

	return NULL;
}

static int
logcfg_rule_dbase_iter_rewind(struct dmod_const_iter * iter)
{
	logcfg_assert_intern(iter);

	struct logcfg_rule_dbase_iter * it = (struct logcfg_rule_dbase_iter *)
	                                     iter;

	it->curr = 0;

	return 0;
}

static const struct dmod_const_iter_ops logcfg_rule_dbase_iter_ops = {
	.step   = logcfg_rule_dbase_iter_step,
	.rewind = logcfg_rule_dbase_iter_rewind,
	.fini   = dmod_const_iter_null_fini
};

static struct logcfg_rule_dbase_iter *
logcfg_rule_dbase_iter_create(void)
{
	struct logcfg_rule_dbase_iter * iter;

	iter = malloc(sizeof(*iter));
	if (!iter)
		return NULL;

	dmod_const_iter_init(&iter->dmod, &logcfg_rule_dbase_iter_ops);
	iter->curr = 0;

	return iter;
}

/******************************************************************************
 * Rule database mapper
 ******************************************************************************/

struct logcfg_rule_dbase_mapper {
	struct logcfg_rule_mapper base;
};

static const struct logcfg_rule *
logcfg_rule_dbase_get_byid(struct logcfg_rule_mapper * mapper __unused,
                           unsigned int                id)
{
	logcfg_rule_repo_assert();
	logcfg_assert_intern(mapper);

	if (id < logcfg_the_rule_repo.nr) {
		logcfg_rule_assert_intern(&logcfg_the_rule_repo.rules[id]);
		
		return &logcfg_the_rule_repo.rules[id];
	}

	errno = ENOENT;

	return NULL;
}

static const struct logcfg_rule *
logcfg_rule_dbase_get_byname(struct logcfg_rule_mapper * mapper __unused,
                             const char *                name)
{
	const struct logcfg_rule * rule;

	rule = logcfg_rule_repo_get_byname(name, logcfg_the_rule_repo.nr);
	if (rule)
		return rule;

	errno = ENOENT;

	return NULL;
}

static struct dmod_const_iter *
logcfg_rule_dbase_iter(void)
{
	struct logcfg_rule_dbase_iter * it;

	it = logcfg_rule_dbase_iter_create();
	if (!it)
		return NULL;

	return &it->dmod;
}

static const char *
logcfg_rule_dbase_errstr(int error)
{
	logcfg_assert_intern(error <= 0);

	return strerror(error);
}

const struct logcfg_rule_mapper_ops logcfg_rule_dbase_mapper_ops = {
	.dmod = {
		.save   = dmod_mapper_rdonly_save,
		.errstr = logcfg_rule_dbase_errstr
	},
	.get_byid       = logcfg_rule_dbase_get_byid,
	.get_byname     = logcfg_rule_dbase_get_byname,
	.iter           = logcfg_rule_dbase_iter
};

struct logcfg_rule_mapper *
logcfg_rule_dbase_mapper_create(const struct logcfg_session * session __unused)
{
	logcfg_assert_intern(session);

	struct logcfg_rule_dbase_mapper * map;

	map = malloc(sizeof(*map));
	if (!map)
		return NULL;

	logcfg_rule_mapper_init(&map->base, &logcfg_rule_dbase_mapper_ops);

	return &map->base;
}

void
logcfg_rule_dbase_mapper_destroy(struct logcfg_rule_mapper * mapper)
{
	logcfg_rule_mapper_fini(mapper);

	free(mapper);
}

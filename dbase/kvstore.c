#include "kvstore.h"
#include "selector.h"
#include "rule.h"
#include <utils/path.h>

/* Restrict maximum transaction log file size to 512kB. */
#define LOGCFG_DBASE_MAX_LOG_SIZE \
	((CONFIG_LOGCFG_DBASE_MAX_LOG_SIZE) << 10)

static const struct kvs_repo_desc logcfg_dbase = {
	.tbl_nr = LOGCFG_DBASE_TID_NR,
	.tables  = {
		[LOGCFG_DBASE_SELECTOR_TID] = &logcfg_dbase_selector_table
	}
};

int
logcfg_dbase_open(struct kvs_repo * repo, const char * path)
{
	logcfg_assert_api(repo);
	logcfg_assert_api(upath_validate_path_name(path) > 0);

	int err;

	err = kvs_repo_open(repo,
	                    path,
	                    LOGCFG_DBASE_MAX_LOG_SIZE,
	                    KVS_DEPOT_PRIV,
	                    S_IRWXU);
	if (err) {
		logcfg_err("%s: failed to open database: %s",
		           path,
		           kvs_strerror(err));
		return err;
	}

	logcfg_debug("%s: database opened", path);

	return 0;
}

int
logcfg_dbase_close(const struct kvs_repo * repo)
{
	logcfg_assert_api(repo);

	return kvs_repo_close(repo);
}

struct kvs_repo *
logcfg_dbase_create(void)
{
	struct kvs_repo * repo;

	repo = kvs_repo_create(&logcfg_dbase);
	if (!repo)
		return NULL;

	logcfg_rule_repo_init();

	return repo;
}

void
logcfg_dbase_destroy(struct kvs_repo * repo)
{
	logcfg_assert_api(repo);

	logcfg_rule_repo_fini();

	kvs_repo_destroy(repo);
}

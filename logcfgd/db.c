#include "selector.h"

/* Restrict maximum transaction log file size to 512kB. */
#define LOGCFG_DB_MAX_LOG_SIZE (CONFIG_LOGCFG_DB_MAX_LOG_SIZE)

enum logcfg_table_id {
	LOGCFG_SELECTOR_TID,
	LOGCFG_TID_NR
};

static const struct kvs_repo_desc logcfg_db = {
	.tbl_nr = LOGCFG_TID_NR,
	tables  = {
		[LOGCFG_SELECTOR_TID] = &logcfg_selector_table
	}
};

const char *
logcfg_db_strerror(int err)
{
	return kvs_strerror(err);
}

int
logcfg_db_begin_xact(const struct kvs_repo *repo,
                     const struct kvs_xact *parent,
                     struct kvs_xact       *xact,
                     unsigned int           flags)
{
	nwif_assert(repo);

	return kvs_begin_xact(&repo->depot, parent, xact, flags);
}

struct kvs_repo *
logcfg_db_create(const char * path)
{
	logcfg_assert_api(upath_validate_path_name(path) > 0);

	struct kvs_repo * repo;
	int               err;

	repo = kvs_repo_create(&logcfg_db);
	if (!repo)
		return NULL;

	err = kvs_repo_open(repo,
	                    path,
	                    LOGCFG_DB_MAX_LOG_SIZE,
	                    flags | KVS_DEPOT_PRIV,
	                    mode);
	if (err)
		goto err;

	return 0;

err:
	kvs_repo_destroy(repo);

	return err;
}

int
logcfg_db_destroy(struct kvs_repo *repo)
{
	logcfg_assert_api(repo);

	int ret;

	ret = kvs_repo_close(repo);

	kvs_repo_destroy(repo);

	return ret;
}

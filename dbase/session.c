#include "session.h"
#include "selector.h"
#include "rule.h"
#include "common/common.h"
#include <dmod/xact.h>

static struct dmod_xact *
logcfg_dbase_session_xact(const struct logcfg_session * session)
{
	return dmod_xact_create_kvs(
		logcfg_dbase_session_get_kvdb(session));
}

static void
logcfg_dbase_session_fini(struct logcfg_session * session __unused)
{
}

static const struct logcfg_session_ops logcfg_dbase_session_ops = {
	.xact = logcfg_dbase_session_xact,
	.fini = logcfg_dbase_session_fini
};

static const struct logcfg_mapper_repo_ops logcfg_dbase_mappers_ops = {
	.create_selector  = logcfg_selector_dbase_mapper_create,
	.create_rule      = logcfg_rule_dbase_mapper_create,

	.destroy_selector = logcfg_selector_dbase_mapper_destroy,
	.destroy_rule     = logcfg_rule_dbase_mapper_destroy
};

struct logcfg_session *
logcfg_dbase_session_create(struct kvs_repo * kvdb)
{
	struct logcfg_dbase_session * sess;

	sess = malloc(sizeof(*sess));
	if (!sess)
		return NULL;

	logcfg_session_init(&sess->base,
	                    &logcfg_dbase_session_ops,
	                    &logcfg_dbase_mappers_ops);

	sess->kvdb = kvdb;

	logcfg_debug("database session created");

	return &sess->base;
}

#include "dbase/session.h"
#include "dbase/kvstore.h"
#include "dbase/selector.h"
#include "dbase/rule.h"
#include "common/conf.h"
#include <dmod/iter.h>
#include <stdlib.h>

static int
logcfg_clui_show_rules(struct logcfg_session * session)
{
	struct logcfg_rule_mapper * map;
	struct dmod_const_iter *    iter;
	const struct logcfg_rule *  rule;
	int                         ret;

	map = logcfg_session_get_mapper(session, rule);
	logcfg_assert_intern(map);

	iter = logcfg_rule_iter(map);
	if (!iter)
		return -errno;

	logcfg_rule_iter_foreach(iter, rule)
		printf("%u: %s (%s)\n",
		       logcfg_rule_get_id(rule),
		       logcfg_rule_get_name(rule),
		       logcfg_rule_get_match(rule));

	ret = dmod_const_iter_error(iter);
	if (ret)
		logcfg_info("failed to iterate over rules: %s",
		            dmod_const_iter_strerror(iter, ret));

	dmod_const_iter_destroy(iter);

	return ret;
}

int
main(int argc, char * const argv[])
{
	static struct elog_stdio_conf          stdlog = {
		.super.severity = CONFIG_LOGCFG_DAEMON_LOG_SEVERITY,
		.format         = CONFIG_LOGCFG_DAEMON_STDLOG_FORMAT
	};
	static struct elog_stdio               logger;
	static const struct logcfg_conf_loader loaders[] = {
		{ .name = "rules", .load = logcfg_rule_load_conf }
	};
	struct kvs_repo *                      db;
	struct logcfg_session *                sess;
	int                                    ret;

	elog_init_stdio(&logger, &stdlog);

	logcfg_init(elog_base(&logger));

	db = logcfg_dbase_create();
	if (!db) {
		ret = -errno;
		goto fini_elog;
	}

	ret = logcfg_conf_load_file(LOGCFG_SYSCONFIGDIR "/logcfgd.conf",
	                            loaders,
	                            stroll_array_nr(loaders));
	if (ret)
		goto destroy_dbase;

	ret = logcfg_dbase_open(db, LOGCFG_LOCALSTATEDIR);
	if (ret)
		goto destroy_dbase;

	sess = logcfg_dbase_session_create(db);
	if (!sess)
		goto close_dbase;

	logcfg_info("database session ready");





	struct kvs_xact xact;

	ret = logcfg_dbase_begin_xact(db, NULL, &xact, 0);
	if (ret) {
		logcfg_err("failed to start database transaction: %s",
		           kvs_strerror(ret));
		goto destroy_session;
	}

	logcfg_clui_show_rules(sess);

	kvs_rollback_xact(&xact);




#if 0
	struct logcfg_selector_mapper * selmap;
	selmap = logcfg_session_get_mapper(session, selector);

	struct logcfg_selector sel;
	logcfg_selector_init(&sel); /* cannot fail */
	logcfg_selector_lend_name(&sel, "untest");

	err = logcfg_selector_save(selmap, &sel, &xact);

	err = kvs_commit_xact(&xact);
	/* kvs_rollback_xact(&xact); */
#endif

destroy_session:
	logcfg_session_destroy(sess);
close_dbase:
	if (!ret)
		ret = logcfg_dbase_close(db);
	else
		logcfg_dbase_close(db);
destroy_dbase:
	logcfg_dbase_destroy(db);
fini_elog:
	elog_fini(logcfg_logger);

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

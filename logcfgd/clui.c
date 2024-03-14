#include "dbase/session.h"
#include "dbase/kvstore.h"
#include "dbase/selector.h"
#include "dbase/rule.h"
#include "common/conf.h"
#include <dmod/iter.h>
#include <stdlib.h>

struct clui_column_desc {
	const char * label;
	double       whint;
	int          flags;
};

typedef int (clui_table_view_update_fn)(struct clui_table_view *, void *);

struct clui_table_view {
	struct libscols_table *         table;
	clui_table_view_update_fn *     update;
	unsigned int                    count;
	const struct clui_column_desc * columns;
};

#define CLUI_TABLE_VIEW_INIT(_columns, _update) \
	{ \
		.table   = NULL, \
		.update  = _update, \
		.count   = array_nr(_columns), \
		.columns = _columns \
	}

static inline int
clui_table_view_display(const struct clui_table_view * view)
{
	scols_print_table(&view->table);
}

static inline int
clui_table_view_update(struct clui_table_view * view, void * data)
{
	return view->update(view, data);
}

struct clui_table_view *
clui_table_view_create(const struct clui_column_desc * columns,
                       unsigned int                    count)
{

	struct libscols_table * tbl;
	unsigned int            c;

	tbl = scols_new_table();
	if (!tbl)
		return NULL;

	scols_table_enable_header_repeat(tbl, 1);

	for (c = 0; c < count; c++) {
		const struct clui_column_desc * col = &columns[c];

		if (!scols_table_new_column(tbl,
		                            col->label,
		                            col->whint,
		                            col->flags))
			goto unref;
	}

	return tbl;

unref:
	scols_unref_table(tbl);

	return NULL;
}

static inline void
clui_table_view_fini(struct clui_table_view * view)
{
	scols_unref_table(view->table);
}




















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
		            dmod_const_iter_strerror(iter));

	dmod_const_iter_destroy(iter);

	return ret;
}

static struct logcfg_session * logcfg_clui_sess;
static struct kvs_repo *       logcfg_clui_dbase;
static struct elog_stdio       logcfg_clui_logger;

static int
logcfg_clui_init(void)
{
	struct elog_stdio_conf                logger_conf = {
		.super.severity = CONFIG_LOGCFG_DAEMON_LOG_SEVERITY,
		.format         = CONFIG_LOGCFG_DAEMON_STDLOG_FORMAT
	};
	int                                   err;
	const struct logcfg_conf_loader const loaders[] = {
		{ .name = "rules", .load = logcfg_rule_load_conf }
	};

	elog_init_stdio(&logcfg_clui_logger, &logger_conf);

	logcfg_init(elog_base(&logcfg_clui_logger));

	logcfg_clui_dbase = logcfg_dbase_create();
	if (!logcfg_clui_dbase) {
		err = -errno;
		goto fini_elog;
	}

	err = logcfg_conf_load_file(LOGCFG_SYSCONFIGDIR "/logcfgd.conf",
	                            loaders,
	                            array_nr(loaders));
	if (err)
		goto destroy_dbase;

	err = logcfg_dbase_open(logcfg_clui_dbase, LOGCFG_LOCALSTATEDIR);
	if (err)
		goto destroy_dbase;

	logcfg_clui_sess = logcfg_dbase_session_create(logcfg_clui_dbase);
	if (!logcfg_clui_sess)
		goto close_dbase;

	logcfg_info("database session ready");

	return 0;

close_dbase:
	logcfg_dbase_close(logcfg_clui_dbase);
destroy_dbase:
	logcfg_dbase_destroy(logcfg_clui_dbase);
fini_elog:
	elog_fini(&logcfg_clui_logger);

	return err;
}

static int
logcfg_clui_fini(void)
{
	int ret;

	logcfg_session_destroy(logcfg_clui_sess);
	ret = logcfg_dbase_close(logcfg_clui_dbase);
	logcfg_dbase_destroy(logcfg_clui_dbase);
	elog_fini(&logcfg_clui_logger);
}

int
main(int argc, char * const argv[])
{
	int ret;

	ret = logcfg_clui_init();
	if (ret)
		return EXIT_FAILURE;

	ret = logcfg_clui_fini();

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

#include "dbase/session.h"
#include "dbase/kvstore.h"
#include "dbase/selector.h"
#include "dbase/rule.h"
#include "common/conf.h"
#include <dmod/iter.h>
#include <clui/table.h>
#include <stdlib.h>
#include <locale.h>

/******************************************************************************
 * Logcfg command line user interface rules view
 ******************************************************************************/

enum {
	LOGCFG_CLUI_RULE_ID_COL,
	LOGCFG_CLUI_RULE_NAME_COL,
	LOGCFG_CLUI_RULE_MATCH_COL,
	LOGCFG_CLUI_RULE_COL_NR
};

struct logcfg_clui_rule_table {
	struct clui_table           clui;
	struct logcfg_rule_mapper * map;
};

static int
logcfg_clui_rule_table_update(struct clui_table * table, void * data __unused)
{
	struct dmod_const_iter *   iter;
	const struct logcfg_rule * rule;
	int                        err;

	iter = logcfg_rule_iter(((const struct logcfg_clui_rule_table *)
	                         table)->map);
	if (!iter)
		return -errno;

	logcfg_rule_iter_foreach(iter, rule) {
		struct libscols_line * line;

		line = clui_table_new_line(table, NULL);
		if (!line) {
			err = -errno;
			goto err;
		}

		err = clui_table_line_set_uint(line,
		                               LOGCFG_CLUI_RULE_ID_COL,
		                               logcfg_rule_get_id(rule));
		if (err)
			goto err;

		err = clui_table_line_set_str(line,
		                              LOGCFG_CLUI_RULE_NAME_COL,
		                              logcfg_rule_get_name(rule));
		if (err)
			goto err;

		err = clui_table_line_set_str(line,
		                              LOGCFG_CLUI_RULE_MATCH_COL,
		                              logcfg_rule_get_match(rule));
		if (err)
			goto err;
	}

	err = dmod_const_iter_error(iter);
	if (err) {
		//clui_err("failed to iterate over rules: %s",
		//         dmod_const_iter_strerror(iter));
		goto err;
	}

	clui_table_sort(table, LOGCFG_CLUI_RULE_NAME_COL);
	dmod_const_iter_destroy(iter);

	return 0;

err:
	clui_table_clear(table);
	dmod_const_iter_destroy(iter);

	return err;
}

static int
logcfg_clui_rule_table_init(struct logcfg_clui_rule_table * table,
                            struct logcfg_session *         session)

{
	logcfg_assert_intern(table);
	logcfg_assert_intern(session);

#define LOGCFG_CLUI_RULE_TOTAL_WHINT \
	(10.0 + \
	 (double)LOGCFG_RULE_NAMESZ_MAX + \
	 (double)LOGCFG_RULE_MATCHSZ_MAX)

	static const struct clui_column_desc cols[] = {
		[LOGCFG_CLUI_RULE_ID_COL] = {
#define LOGCFG_CLUI_RULE_MATCH_WHINT \
	((double)LOGCFG_RULE_MATCHSZ_MAX / LOGCFG_CLUI_RULE_TOTAL_WHINT)
			.label = "ID",
			.whint = LOGCFG_CLUI_RULE_MATCH_WHINT,
			.flags = SCOLS_FL_RIGHT
		},
		[LOGCFG_CLUI_RULE_NAME_COL] = {
#define LOGCFG_CLUI_RULE_NAME_WHINT \
	((double)LOGCFG_RULE_NAMESZ_MAX / LOGCFG_CLUI_RULE_TOTAL_WHINT)
			.label = "NAME",
			.whint = LOGCFG_CLUI_RULE_NAME_WHINT,
			.flags = 0
		},
		[LOGCFG_CLUI_RULE_MATCH_COL] = {
#define LOGCFG_CLUI_RULE_ID_WHINT \
	(10.0 / LOGCFG_CLUI_RULE_TOTAL_WHINT)
			.label = "MATCH",
			.whint = LOGCFG_CLUI_RULE_ID_WHINT,
			.flags = SCOLS_FL_WRAP
		}
	};
	static const struct clui_table_desc  tbl = {
		.update     = logcfg_clui_rule_table_update,
		.noheadings = 0,
		.col_cnt    = array_nr(cols),
		.columns    = cols
	};
	int                                  err;

	err = clui_table_init(&table->clui, &tbl);
	if (err)
		return err;

	table->map = logcfg_session_get_mapper(session, rule);
	logcfg_assert_intern(table->map);

	return 0;
}

static void
logcfg_clui_rule_table_fini(struct logcfg_clui_rule_table * table)
{
	logcfg_assert_intern(table);
	logcfg_assert_intern(table->map);

	clui_table_fini(&table->clui);
}

static struct elog_stdio             logcfg_clui_logger;
static struct logcfg_session *       logcfg_clui_sess;
static struct kvs_repo *             logcfg_clui_dbase;
static struct logcfg_clui_rule_table logcfg_clui_rule_view;

static int
logcfg_clui_init(struct clui_parser * parser, int argc, char * const argv[])
{
	logcfg_assert_intern(parser);
	logcfg_assert_intern(argv);

	struct elog_stdio_conf          logger_conf = {
		.super.severity = CONFIG_LOGCFG_DAEMON_LOG_SEVERITY,
		.format         = CONFIG_LOGCFG_DAEMON_STDLOG_FORMAT
	};
	int                             err;
	const struct logcfg_conf_loader loaders[] = {
		{ .name = "rules", .load = logcfg_rule_load_conf }
	};

	setlocale(LC_ALL, "");

	err = clui_init(parser, argc, argv);
	if (err)
		return err;

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

	err = logcfg_clui_rule_table_init(&logcfg_clui_rule_view,
	                                  logcfg_clui_sess);
	if (err)
		goto destroy_session;

	logcfg_info("command line user interface ready");

	return 0;

destroy_session:
	logcfg_session_destroy(logcfg_clui_sess);
close_dbase:
	logcfg_dbase_close(logcfg_clui_dbase);
destroy_dbase:
	logcfg_dbase_destroy(logcfg_clui_dbase);
fini_elog:
	elog_fini(elog_base(&logcfg_clui_logger));

	return err;
}

static int
logcfg_clui_fini(void)
{
	int ret;

	logcfg_clui_rule_table_fini(&logcfg_clui_rule_view);

	logcfg_session_destroy(logcfg_clui_sess);
	ret = logcfg_dbase_close(logcfg_clui_dbase);
	logcfg_dbase_destroy(logcfg_clui_dbase);
	elog_fini(elog_base(&logcfg_clui_logger));

	return ret;
}

int
main(int argc, char * const argv[])
{
	struct clui_parser parser;
	int                ret;

	ret = logcfg_clui_init(&parser, argc, argv);
	if (ret)
		return EXIT_FAILURE;

	ret = clui_table_update(&logcfg_clui_rule_view.clui, NULL);
	if (ret)
		goto fini;
	ret = clui_table_display(&logcfg_clui_rule_view.clui);
	if (ret)
		goto fini;

fini:
	ret = logcfg_clui_fini();

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

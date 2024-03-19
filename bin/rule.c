#include "clui.h"
#include "logcfg/rule.h"
#include "logcfg/session.h"
#include <clui/shell.h>
#include <clui/table.h>
#include <dmod/iter.h>
#include <string.h>

/******************************************************************************
 * Logcfg command line user interface rules tabular view
 ******************************************************************************/

enum {
	LOGCFG_CLUI_RULE_ID_COL,
	LOGCFG_CLUI_RULE_NAME_COL,
	LOGCFG_CLUI_RULE_MATCH_COL,
	LOGCFG_CLUI_RULE_COL_NR
};

struct logcfg_clui_rule_table {
	struct clui_table           clui;
	bool                        loaded;
	struct logcfg_rule_mapper * map;
};

static struct logcfg_clui_rule_table logcfg_clui_rule_table_view;

#if 0
static void
logcfg_clui_rule_update(struct clui_table * table)
{
	((struct logcfg_clui_rule_table *)table)->loaded = false;
}
#endif

static int
logcfg_clui_rule_table_load(struct clui_table * table, void * data __unused)
{
	struct logcfg_clui_rule_table * tbl = (struct logcfg_clui_rule_table *)
	                                      table;

	if (!tbl->loaded) {
		struct dmod_const_iter *   iter;
		const struct logcfg_rule * rule;
		int                        err;

		clui_table_clear(table);

		iter = logcfg_rule_iter(tbl->map);
		if (!iter)
			return -errno;

		logcfg_rule_iter_foreach(iter, rule) {
			struct libscols_line * line;

			line = clui_table_new_line(table, NULL);
			if (!line) {
				err = -errno;
				goto err;
			}

			err = clui_table_line_set_uint(
				line,
				LOGCFG_CLUI_RULE_ID_COL,
				logcfg_rule_get_id(rule));
			if (err)
				goto err;

			err = clui_table_line_set_str(
				line,
				LOGCFG_CLUI_RULE_NAME_COL,
				logcfg_rule_get_name(rule));
			if (err)
				goto err;

			err = clui_table_line_set_str(
				line,
				LOGCFG_CLUI_RULE_MATCH_COL,
				logcfg_rule_get_match(rule));
			if (err)
				goto err;
		}

		err = dmod_const_iter_error(iter);
		if (err)
			goto err;

		clui_table_sort(table, LOGCFG_CLUI_RULE_NAME_COL);
		dmod_const_iter_destroy(iter);

		tbl->loaded = true;

		return 0;

err:
		dmod_const_iter_destroy(iter);

		return err;
	}

	return 0;
}

static int
logcfg_clui_rule_table_init(struct logcfg_clui_rule_table * table,
                            struct logcfg_session *         session)

{
	logcfg_assert_intern(table);
	logcfg_assert_intern(session);

#define LOGCFG_CLUI_RULE_TOTAL_WHINT \
	(10.0 + \
	 (double)(LOGCFG_RULE_NAMESZ_MAX - 1) + \
	 (double)(LOGCFG_RULE_MATCHSZ_MAX- 1))

	static const struct clui_column_desc cols[] = {
		[LOGCFG_CLUI_RULE_ID_COL] = {
#define LOGCFG_CLUI_RULE_ID_WHINT \
	(10.0 / LOGCFG_CLUI_RULE_TOTAL_WHINT)
			.label = "ID",
			.whint = LOGCFG_CLUI_RULE_ID_WHINT,
			.flags = SCOLS_FL_RIGHT
		},
		[LOGCFG_CLUI_RULE_NAME_COL] = {
#define LOGCFG_CLUI_RULE_NAME_WHINT \
	((double)(LOGCFG_RULE_NAMESZ_MAX - 1) / LOGCFG_CLUI_RULE_TOTAL_WHINT)
			.label = "NAME",
			.whint = LOGCFG_CLUI_RULE_NAME_WHINT,
			.flags = 0
		},
		[LOGCFG_CLUI_RULE_MATCH_COL] = {
#define LOGCFG_CLUI_RULE_MATCH_WHINT \
	((double)(LOGCFG_RULE_MATCHSZ_MAX - 1) / LOGCFG_CLUI_RULE_TOTAL_WHINT)
			.label = "MATCH",
			.whint = LOGCFG_CLUI_RULE_MATCH_WHINT,
			.flags = SCOLS_FL_WRAP
		}
	};
	static const struct clui_table_desc  desc = {
		.load       = logcfg_clui_rule_table_load,
		.noheadings = 0,
		.col_cnt    = array_nr(cols),
		.columns    = cols
	};
	int                                  err;

	err = clui_table_init(&table->clui, &desc);
	if (err)
		return err;

	table->loaded = false;
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

/******************************************************************************
 * Rule command handling
 ******************************************************************************/

static int
logcfg_clui_rule_display(const struct logcfg_clui_ctx * ctx __unused,
                         const struct clui_parser *     parser)
{
	int ret;

	ret = clui_table_load(&logcfg_clui_rule_table_view.clui, NULL);
	if (ret) {
		logcfg_clui_err(parser,
		                ret,
		                "failed to load matching rule list");
		return ret;
	}

	ret = clui_table_display(&logcfg_clui_rule_table_view.clui);
	if (ret)
		logcfg_clui_err(parser,
		                ret,
		                "failed to display matching rule list");

	return ret;
}

static int
logcfg_clui_parse_rule(const struct clui_cmd * cmd,
                       struct clui_parser    * parser,
                       int                     argc,
                       char * const            argv[],
                       void                  * ctx)
{
	logcfg_assert_intern(ctx);

	if (argc != 1) {
		clui_err(parser, "invalid number of arguments.\n");
		goto help;
	}

	if (!strcmp(argv[0], "show")) {
		logcfg_clui_sched_exec(ctx, logcfg_clui_rule_display);
		return 0;
	}
	else if (!strcmp(argv[0], "help")) {
		logcfg_clui_sched_help(ctx, cmd);
		return 0;
	}
	else
		clui_err(parser, "unknown '%s' command.\n", argv[0]);

help:
	clui_help_cmd(cmd, parser, stderr);

	return -EINVAL;
}

static char **
logcfg_clui_rule_complete(const struct clui_cmd * cmd __unused,
                          struct clui_parser *    parser __unused,
                          int                     argc,
                          const char * const      argv[] __unused,
                          void *                  ctx __unused)
{
	static const char * const matches[] = {
		"show",
		"help"
	};

	if (argc)
		return NULL;

	return clui_shell_build_static_matches(matches, array_nr(matches));
}

LOGCFG_CLUI_DEFINE_HELP(
	logcfg_clui_rule_help,
	"rule <RULE_SPEC>",
	"Manage syslog daemon message matching rules",
	"Where <RULE_SPEC>:\n"
	"    show -- Show syslog daemon message matching rules.\n"
	"    help -- This help message.\n")

static const struct clui_cmd logcfg_clui_rule_cmd = {
	.parse    = logcfg_clui_parse_rule,
	.complete = logcfg_clui_rule_complete,
	.help     = logcfg_clui_rule_help
};

/******************************************************************************
 * Rule module definitions
 ******************************************************************************/

static int
logcfg_clui_rule_init(void)
{
	return logcfg_clui_rule_table_init(&logcfg_clui_rule_table_view,
	                                   logcfg_clui_sess);
}

static void
logcfg_clui_rule_fini(void)
{
	logcfg_clui_rule_table_fini(&logcfg_clui_rule_table_view);
}

const struct logcfg_clui_module logcfg_clui_rule_module = {
	.cmd  = logcfg_clui_rule_cmd,
	.init = logcfg_clui_rule_init,
	.fini = logcfg_clui_rule_fini
};

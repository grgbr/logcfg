#include "clui.h"
#include "logcfg/selector.h"
#include "logcfg/session.h"
#include <clui/shell.h>
#include <clui/table.h>
#include <dmod/iter.h>
#include <string.h>

/******************************************************************************
 * Logcfg command line user interface selector tabular view
 ******************************************************************************/

enum {
	LOGCFG_CLUI_SELECTOR_ID_COL,
	LOGCFG_CLUI_SELECTOR_NAME_COL,
	LOGCFG_CLUI_SELECTOR_COL_NR
};

struct logcfg_clui_selector_table {
	struct clui_table               clui;
	struct logcfg_selector_mapper * map;
};

static struct logcfg_clui_selector_table logcfg_clui_selector_table_view;

static int
logcfg_clui_selector_table_load(struct clui_table *        table,
                                const struct clui_parser * parser,
                                void *                     data)
{
	struct logcfg_clui_selector_table * tbl =
		(struct logcfg_clui_selector_table *)table;
	struct dmod_xact *                  xact;
	struct dmod_iter *                  iter;
	const struct logcfg_selector *      selector;
	int                                 err;

	clui_table_clear(table);

	xact = logcfg_clui_begin_xact(parser);
	if (!xact) {
		err = -errno;
		goto err;
	}

	iter = logcfg_selector_iter(tbl->map, xact);
	if (!iter) {
		err = -errno;
		clui_err(parser,
		         "failed to create message selectors iterator: "
		         "%s (%d).\n",
		         dmod_mapper_strerror((struct dmod_mapper *)tbl->map,
		                              err),
		         err);
		goto abort;
	}

	logcfg_selector_iter_foreach(iter, selector) {
		struct libscols_line *      line;
		const struct stroll_lvstr * name;

		err = logcfg_selector_get_name(selector, &name);
		logcfg_assert_intern(!err);

		line = clui_table_new_line(table, NULL);
		if (!line) {
			err = -errno;
			goto destroy;
		}

		err = clui_table_line_set_hex64(
			line,
			LOGCFG_CLUI_SELECTOR_ID_COL,
			logcfg_selector_get_id(selector));
		if (err)
			goto destroy;

		err = clui_table_line_set_str(
			line,
			LOGCFG_CLUI_SELECTOR_NAME_COL,
			stroll_lvstr_cstr(name));
		if (err)
			goto destroy;
	}

	err = dmod_iter_error(iter);
	if (err) {
		clui_err(parser,
		         "failed to iterate over message selectors: "
		         "%s (%d).\n",
		         dmod_mapper_strerror((struct dmod_mapper *)tbl->map,
		                              err),
		         err);
		goto abort;
	}

	dmod_iter_destroy(iter);

	err = logcfg_clui_abort_xact(xact, 0, parser);
	if (err)
		return err;

	clui_table_sort(table, LOGCFG_CLUI_SELECTOR_NAME_COL);

	return 0;

destroy:
	dmod_iter_destroy(iter);
abort:
	err = logcfg_clui_abort_xact(xact, err, parser);

	return err;
}

static int
logcfg_clui_selector_table_init(struct logcfg_clui_selector_table * table,
                                struct logcfg_session *             session)

{
	logcfg_assert_intern(table);
	logcfg_assert_intern(session);

#define LOGCFG_CLUI_SELECTOR_TOTAL_WHINT \
	(8.0 + \
	 (double)LOGCFG_SELECTOR_NAMELEN_MAX)

	static const struct clui_column_desc cols[] = {
		[LOGCFG_CLUI_SELECTOR_ID_COL] = {
#define LOGCFG_CLUI_SELECTOR_ID_WHINT \
	(8.0 / LOGCFG_CLUI_SELECTOR_TOTAL_WHINT)
			.label = "ID",
			.whint = LOGCFG_CLUI_SELECTOR_ID_WHINT,
			.flags = SCOLS_FL_RIGHT
		},
		[LOGCFG_CLUI_SELECTOR_NAME_COL] = {
#define LOGCFG_CLUI_SELECTOR_NAME_WHINT \
	((double)LOGCFG_SELECTOR_NAMELEN_MAX / LOGCFG_CLUI_SELECTOR_TOTAL_WHINT)
			.label = "NAME",
			.whint = LOGCFG_CLUI_SELECTOR_NAME_WHINT,
			.flags = 0
		}
	};
	static const struct clui_table_desc  desc = {
		.load       = logcfg_clui_selector_table_load,
		.noheadings = 0,
		.col_cnt    = stroll_array_nr(cols),
		.columns    = cols
	};
	int                                  err;

	err = clui_table_init(&table->clui, &desc);
	if (err)
		return err;

	table->map = logcfg_session_get_mapper(session, selector);
	logcfg_assert_intern(table->map);

	return 0;
}

static void
logcfg_clui_selector_table_fini(struct logcfg_clui_selector_table * table)
{
	logcfg_assert_intern(table);
	logcfg_assert_intern(table->map);

	clui_table_fini(&table->clui);
}

/******************************************************************************
 * Selector command handling
 ******************************************************************************/

static int
logcfg_clui_selector_show(const struct logcfg_clui_ctx * ctx __unused,
                          const struct clui_parser *     parser)
{
	err = clui_table_load(&logcfg_clui_selector_table_view.clui,
	                      parser,
	                      NULL);
	if (err)
		goto err;

	err = clui_table_display(&logcfg_clui_selector_table_view.clui, parser);
	if (err)
		goto err;

	return 0;

err:
	clui_err(parser, "failed to show message selectors (%d).\n", err);

	return -1;
}

static int
logcfg_clui_parse_selector(const struct clui_cmd * cmd,
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
		logcfg_clui_sched_exec(ctx, logcfg_clui_selector_show);
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

	return -1;
}

static char **
logcfg_clui_selector_complete(const struct clui_cmd * cmd __unused,
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

	return clui_shell_build_static_matches(matches,
	                                       stroll_array_nr(matches));
}

static void
logcfg_clui_selector_help(const struct clui_cmd    * cmd __unused,
                          const struct clui_parser * parser,
                          FILE                     * stdio)
{
	logcfg_clui_display_help(
		"selector <SELECTOR_SPEC>",
		"Manage syslog daemon message selectors",
		"Where <SELECTOR_SPEC>:\n"
		"    show -- Show syslog daemon message selectors.\n"
		"    help -- This help message.\n",
		parser,
		stdio);
}

static const struct clui_cmd logcfg_clui_selector_cmd = {
	.parse    = logcfg_clui_parse_selector,
	.complete = logcfg_clui_selector_complete,
	.help     = logcfg_clui_selector_help
};

/******************************************************************************
 * Selector module definitions
 ******************************************************************************/

static int
logcfg_clui_selector_init(struct logcfg_session * session)
{
	return logcfg_clui_selector_table_init(&logcfg_clui_selector_table_view,
	                                       logcfg_clui_sess);
}

static void
logcfg_clui_selector_fini(void)
{
	logcfg_clui_selector_table_fini(&logcfg_clui_selector_view);
}

const struct logcfg_clui_module logcfg_clui_selector_module = {
	.cmd  = logcfg_clui_selector_cmd,
	.init = logcfg_clui_selector_init,
	.fini = logcfg_clui_selector_fini
};

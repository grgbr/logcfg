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
	bool                            loaded;
	struct logcfg_selector_mapper * map;
};

static struct logcfg_clui_selector_table logcfg_clui_selector_table_view;

#if 0
static void
logcfg_clui_selector_update(struct clui_table * table)
{
	((struct logcfg_clui_selector_table *)table)->loaded = false;
}
#endif

static int
logcfg_clui_selector_table_load(struct clui_table * table, void * data)
{
	struct logcfg_clui_selector_table * tbl =
		(struct logcfg_clui_selector_table *)table;

	if (!tbl->loaded) {
		struct dmod_const_iter *       iter;
		const struct logcfg_selector * selector;
		int                            err;

		clui_table_clear(table);

		iter = logcfg_selector_iter(tbl->map,
		                            (struct logcfg_xact *)data);
		if (!iter)
			return -errno;

		logcfg_selector_iter_foreach(iter, selector) {
			struct libscols_line *      line;
			const struct stroll_lvstr * name;

			err = logcfg_selector_get_name(selector, &name);
			logcfg_assert_intern(!err);

			line = clui_table_new_line(table, NULL);
			if (!line) {
				err = -errno;
				goto err;
			}

			err = clui_table_line_set_hex64(
				line,
				LOGCFG_CLUI_SELECTOR_ID_COL,
				logcfg_selector_get_id(selector));
			if (err)
				goto err;

			err = clui_table_line_set_str(
				line,
				LOGCFG_CLUI_SELECTOR_NAME_COL,
				stroll_lvstr_cstr(name));
			if (err)
				goto err;
		}

		err = dmod_const_iter_error(iter);
		if (err)
			goto err;

		clui_table_sort(table, LOGCFG_CLUI_SELECTOR_NAME_COL);
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
logcfg_clui_selector_table_init(struct logcfg_clui_selector_table * table,
                                struct logcfg_session *         session)

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
		.col_cnt    = array_nr(cols),
		.columns    = cols
	};
	int                                  err;

	err = clui_table_init(&table->clui, &desc);
	if (err)
		return err;

	table->loaded = false;
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

#define LOGCFG_CLUI_SELECTOR_HELP \
	"Synopsis:\n" \
	"    %1$s%2$sselector show\n" \
	"    Show syslog message selectors.\n" \
	"\n" \
	"    %1$s%2$sselector help\n" \
	"    This help message.\n"

static int
logcfg_clui_selector_display(const struct logcfg_clui_ctx * ctx __unused,
                             const struct clui_parser *     parser)
{
	struct logcfg_xact * xact;
	int                  err;

	xact = logcfg_clui_begin_xact(parser);
	if (!xact)
		return -errno;

	err = clui_table_load(&logcfg_clui_selector_table_view.clui, xact);

	logcfg_clui_rollback_xact(xact);

	if (err) {
		logcfg_clui_err(parser,
		                err,
		                "failed to load message selector list");
		return err;
	}

	err = clui_table_display(&logcfg_clui_selector_table_view.clui);
	if (err) {
		logcfg_clui_err(parser,
		                ret,
		                "failed to display message selector list");
		return err;
	}

	return 0;
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
		logcfg_clui_sched_exec(ctx, logcfg_clui_selector_display);
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

	return clui_shell_build_static_matches(matches, array_nr(matches));
}

static void
logcfg_clui_selector_help(const struct clui_cmd    * cmd __unused,
                          const struct clui_parser * parser,
                          FILE                     * stdio)
{
	const char * pref = clui_prefix(parser);

	if (!pref)
		fprintf(stdio, LOGCFG_CLUI_SELECTOR_HELP, "", "");
	else
		fprintf(stdio, LOGCFG_CLUI_SELECTOR_HELP, pref, " ");
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

////////////////////////////////////////////////////////////////////////////////

struct dmod_xact *
logcfg_clui_begin_xact(const struct clui_parser * parser)
{
	struct dmod_xact * xact;

	xact = logcfg_session_begin_xact(logcfg_clui_sess, NULL);
	if (!xact) {
		int err = errno;

		logcfg_clui_err(parser, -err, "failed to begin transaction");

		errno = err;
	}

	return xact;
}

int
logcfg_clui_commit_xact(struct dmod_xact *         xact,
                        const struct clui_parser * parser)
{
	int err;

	err = dmod_xact_commit(xact);
	if (err)
		logcfg_clui_err(parser, err, "failed to commit transaction");

	return err;
}

int
logcfg_clui_rollback_xact(struct dmod_xact *         xact,
                          const struct clui_parser * parser)
{
	int err;

	err = dmod_xact_rollback(xact);
	if (err)
		logcfg_clui_err(parser, err, "failed to rollback transaction");

	return err;
}

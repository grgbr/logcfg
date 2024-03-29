#include "clui.h"
#include "dbase/session.h"
#include "dbase/kvstore.h"
#include "dbase/rule.h"
#include "common/conf.h"
#include <clui/shell.h>
#include <utils/signal.h>
#include <dmod/xact.h>
#include <stdlib.h>
#include <locale.h>

#define LOGCFG_CLUI_PROMPT "logcfg> "

/******************************************************************************
 * Command Line User Interface context (internal state) handling
 ******************************************************************************/

static void
logcfg_clui_reset_ctx(struct logcfg_clui_ctx * ctx)
{
	logcfg_assert_intern(ctx);

	ctx->cmd = NULL;
	ctx->exec = NULL;
}

static int
logcfg_clui_exec_help(const struct logcfg_clui_ctx * ctx,
                      const struct clui_parser     * parser)
{
	logcfg_assert_intern(ctx);
	logcfg_assert_intern(parser);

	clui_help_cmd(ctx->cmd, parser, stdout);

	return 0;
}

void
logcfg_clui_display_help(const char * __restrict               brief,
                         const char * __restrict               desc,
                         const char * __restrict               spec,
                         const struct clui_parser * __restrict parser,
                         FILE * __restrict                     stdio)
{
	logcfg_assert_intern(brief);
	logcfg_assert_intern(*brief);
	logcfg_assert_intern(desc);
	logcfg_assert_intern(*desc);
	logcfg_assert_intern(spec);
	logcfg_assert_intern(*spec);
	logcfg_assert_intern(parser);
	logcfg_assert_intern(stdio);

	const char * pref = clui_prefix(parser);

	if (!pref) {
		fprintf(stdio,
		        "Usage:\n"
		        "    %s\n"
		        "    %s.\n\n"
		        "%s",
		        brief,
		        desc,
		        spec);
		return;
	}

#define LOGCFG_CLUI_TOP_OPTS \
	"With OPTIONS:\n" \
	"    -c|--config <CONFIG_PATH> Use CONFIG_PATH as pathname to configuration\n" \
	"                              file\n" \
	"    -d|--dbdir  <DBDIR_PATH>  Use DBDIR_PATH as pathname to database\n" \
	"                              directory ; defaults to:\n" \
	"                              [" LOGCFG_LOCALSTATEDIR "]\n"
	fprintf(stdio,
	        "Usage:\n"
	        "    %s [OPTIONS] %s\n"
	        "    %s.\n\n"
	        "%s\n"
	        LOGCFG_CLUI_TOP_OPTS,
	        pref, brief,
	        desc,
	        spec);
}

void
logcfg_clui_sched_exec(void * ctx, logcfg_clui_exec_fn * exec)
{
	logcfg_assert_intern(ctx);
	logcfg_assert_intern(exec);
	logcfg_assert_intern(!((struct logcfg_clui_ctx *)ctx)->exec);

	((struct logcfg_clui_ctx *)ctx)->exec = exec;
}

void
logcfg_clui_sched_help(void * ctx, const struct clui_cmd * cmd)
{
	logcfg_assert_intern(ctx);

	((struct logcfg_clui_ctx *)ctx)->cmd = cmd;

	logcfg_clui_sched_exec(ctx, logcfg_clui_exec_help);
}

/******************************************************************************
 * Shell command
 ******************************************************************************/

#define LOGCFG_CLUI_TOP_BRIEF \
	"<COMMAND>"

#define LOGCFG_CLUI_TOP_DESC \
	"Manage syslog daemon configuration"

#define LOGCFG_CLUI_THIS_HELP \
	"    help                          -- This help message.\n"

#define LOGCFG_CLUI_SHELL_SPEC \
	"Where <COMMAND>:\n" \
	     LOGCFG_CLUI_TOP_RULE_HELP \
	     LOGCFG_CLUI_TOP_SELECTOR_HELP \
	"    quit                          -- Quit interactive shell.\n" \
	     LOGCFG_CLUI_THIS_HELP

LOGCFG_CLUI_DEFINE_HELP(logcfg_clui_shell_help,
                        LOGCFG_CLUI_TOP_BRIEF,
                        LOGCFG_CLUI_TOP_DESC,
                        LOGCFG_CLUI_SHELL_SPEC)

static int
logcfg_clui_parse_shell(const struct clui_cmd * cmd,
                        struct clui_parser    * parser,
                        int                     argc,
                        char * const            argv[],
                        void                  * ctx)
{
	logcfg_assert_intern(ctx);

	if (argc < 1) {
		clui_err(parser, "missing argument.\n");
		goto help;
	}

	if (!strcmp(argv[0], "rule"))
		return clui_parse_cmd(&logcfg_clui_rule_module.cmd,
		                      parser,
		                      argc - 1,
		                      &argv[1],
		                      ctx);

	if (argc != 1) {
		clui_err(parser, "invalid number of arguments.\n");
		goto help;
	}

	if (!strcmp(argv[0], "quit")) {
		return -ESHUTDOWN;
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

static const struct clui_cmd logcfg_clui_shell_cmd = {
	.parse = logcfg_clui_parse_shell,
	.help  = logcfg_clui_shell_help
};

/******************************************************************************
 * Top-level command
 ******************************************************************************/

#define LOGCFG_CLUI_TOP_SPEC \
	"Where <COMMAND>:\n" \
	     LOGCFG_CLUI_TOP_RULE_HELP \
	     LOGCFG_CLUI_TOP_SELECTOR_HELP \
	"    shell                         -- Run in interactive shell mode.\n" \
	LOGCFG_CLUI_THIS_HELP

LOGCFG_CLUI_DEFINE_HELP(logcfg_clui_top_help,
                        LOGCFG_CLUI_TOP_BRIEF,
                        LOGCFG_CLUI_TOP_DESC,
                        LOGCFG_CLUI_TOP_SPEC)

static int
logcfg_clui_parse_top(const struct clui_cmd * cmd,
                      struct clui_parser    * parser,
                      int                     argc,
                      char * const            argv[],
                      void                  * ctx)
{
	logcfg_assert_intern(ctx);

	if (argc < 1) {
		clui_err(parser, "missing argument.\n");
		goto help;
	}

	if (!strcmp(argv[0], "rule"))
		return clui_parse_cmd(&logcfg_clui_rule_module.cmd,
		                      parser,
		                      argc - 1,
		                      &argv[1],
		                      ctx);

	if (argc != 1) {
		clui_err(parser, "invalid number of arguments.\n");
		goto help;
	}

	if (!strcmp(argv[0], "shell")) {
		if (!clui_has_tty()) {
			logcfg_clui_err(parser,
			                -ENOTTY,
			                "cannot run in shell mode\n");
			return -ENOTTY;
		}

		((struct logcfg_clui_ctx *)ctx)->shell = true;
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
logcfg_clui_complete_top(const struct clui_cmd * cmd __unused,
                         struct clui_parser    * parser,
                         int                     argc,
                         const char * const    * argv,
                         void                  * ctx)
{
	static const char * const matches[] = {
		"rule",
		"quit",
		"help"
	};

	if (argc) {
		if (!strcmp(argv[0], "rule"))
			return clui_complete_cmd(&logcfg_clui_rule_module.cmd,
			                         parser,
			                         argc - 1,
			                         &argv[1],
			                         ctx);
		return NULL;
	}

	return clui_shell_build_static_matches(matches,
	                                       stroll_array_nr(matches));
}

static const struct clui_cmd logcfg_clui_top_cmd = {
	.parse    = logcfg_clui_parse_top,
	.complete = logcfg_clui_complete_top,
	.help     = logcfg_clui_top_help
};

/******************************************************************************
 * Command Line User Interface modules handling
 ******************************************************************************/

static const struct logcfg_clui_module * const logcfg_clui_modules[] = {
	&logcfg_clui_rule_module
};

static void
logcfg_clui_dofini_modules(unsigned int id)
{
	while (id--) {
		const struct logcfg_clui_module * mod = logcfg_clui_modules[id];

		if (mod->fini)
			mod->fini();
	}
}

static int
logcfg_clui_init_modules(void)
{
	unsigned int m;
	int          err = 0;

	for (m = 0; m < stroll_array_nr(logcfg_clui_modules); m++) {
		const struct logcfg_clui_module * mod = logcfg_clui_modules[m];

		if (mod->init) {
			err = mod->init();
			if (err)
				goto err;
		}
	}

	return 0;

err:
	logcfg_clui_dofini_modules(m);

	return err;
}

static void
logcfg_clui_fini_modules(void)
{
	logcfg_clui_dofini_modules(stroll_array_nr(logcfg_clui_modules));
}

/******************************************************************************
 * Main handling
 ******************************************************************************/

static struct elog_stdio  logcfg_clui_logger;
static struct kvs_repo *  logcfg_clui_dbase;
struct logcfg_session *   logcfg_clui_sess;
static struct dmod_xact * logcfg_clui_xact;

struct dmod_xact *
logcfg_clui_begin_xact(const struct clui_parser * __restrict parser)
{
	logcfg_assert_intern(parser);
	logcfg_assert_intern(logcfg_clui_xact);

	int err;

	err = dmod_xact_begin(logcfg_clui_xact, NULL);
	if (err) {
		clui_err(parser,
		         "failed to begin transaction: %s (%d).",
		         logcfg_session_strerror(logcfg_clui_sess, err),
		         err);

		errno = -err;

		return NULL;
	}

	return logcfg_clui_xact;
}

int
logcfg_clui_end_xact(struct dmod_xact *                    xact,
                     int                                   status,
                     const struct clui_parser * __restrict parser)
{
	logcfg_assert_intern(parser);
	logcfg_assert_intern(logcfg_clui_xact);

	int ret;

	ret = dmod_xact_end(xact, status);
	if (!ret)
		return 0;

	if (!status)
		clui_err(parser,
		         "failed to end transaction: %s (%d).",
		         logcfg_session_strerror(logcfg_clui_sess, ret),
		         ret);

	return ret;
}

int
logcfg_clui_abort_xact(struct dmod_xact *                    xact,
                       int                                   status,
                       const struct clui_parser * __restrict parser)
{
	logcfg_assert_intern(parser);
	logcfg_assert_intern(logcfg_clui_xact);

	int ret;

	ret = dmod_xact_abort(xact, status);
	if (!ret)
		return 0;

	if (!status)
		clui_err(parser,
		         "failed to abort transaction: %s (%d).",
		         logcfg_session_strerror(logcfg_clui_sess, ret),
		         ret);

	return ret;
}

static void
logcfg_clui_handle_sig(int signo __unused)
{
	clui_shell_shutdown();
}

static void
logcfg_clui_init_sigs(void)
{
	sigset_t         sigs;
	struct sigaction act = {
		.sa_handler = logcfg_clui_handle_sig,
		.sa_flags   = 0
	};

	usig_emptyset(&sigs);
	usig_addset(&sigs, SIGHUP);
	usig_addset(&sigs, SIGINT);
	usig_addset(&sigs, SIGQUIT);
	usig_addset(&sigs, SIGTERM);

	act.sa_mask = sigs;
	usig_action(SIGHUP, &act, NULL);
	usig_action(SIGINT, &act, NULL);
	usig_action(SIGQUIT, &act, NULL);
	usig_action(SIGTERM, &act, NULL);
}

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
	                            stroll_array_nr(loaders));
	if (err)
		goto destroy_dbase;

	err = logcfg_dbase_open(logcfg_clui_dbase, LOGCFG_LOCALSTATEDIR);
	if (err)
		goto destroy_dbase;

	logcfg_clui_sess = logcfg_dbase_session_create(logcfg_clui_dbase);
	if (!logcfg_clui_sess)
		goto close_dbase;

	logcfg_clui_xact = logcfg_session_create_xact(logcfg_clui_sess);
	if (!logcfg_clui_xact)
		goto destroy_session;

	logcfg_info("database session ready");

	err = logcfg_clui_init_modules();
	if (err)
		goto destroy_xact;

	logcfg_info("command line user interface ready");

	return 0;

destroy_xact:
	dmod_xact_destroy(logcfg_clui_xact);
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

	logcfg_clui_fini_modules();

	dmod_xact_destroy(logcfg_clui_xact);
	logcfg_session_destroy(logcfg_clui_sess);
	ret = logcfg_dbase_close(logcfg_clui_dbase);
	logcfg_dbase_destroy(logcfg_clui_dbase);
	elog_fini(elog_base(&logcfg_clui_logger));

	return ret;
}

int
main(int argc, char * const argv[])
{
	struct clui_parser     parser;
	struct logcfg_clui_ctx ctx = { 0, };
	int                    ret;

	ret = logcfg_clui_init(&parser, argc, argv);
	if (ret)
		return EXIT_FAILURE;

	/*
	 * Remove me once using clui_parse_opts() and use its return value
	 * instead.
	 */
	ret = 1;

	ret = clui_parse_cmd(&logcfg_clui_top_cmd,
	                     &parser,
	                     argc - ret,
	                     &argv[ret],
	                     &ctx);
	if (ret < 0)
		goto fini;

	if (ctx.shell) {
		logcfg_clui_init_sigs();
		clui_shell_init(&logcfg_clui_top_cmd,
		                &parser,
		                program_invocation_short_name,
		                LOGCFG_CLUI_PROMPT,
		                &ctx,
		                true);

		while (true) {
			struct clui_shell_expr expr;

			ret = clui_shell_read_expr(&expr);
			if (ret == -ESHUTDOWN) {
				/* Shell shutdown requested. */
				ret = 0;
				break;
			}
			else if (ret == -ENODATA) {
				/* Empty input. */
				continue;
			}
			else if (ret) {
				/* Input life fetching error. */
				break;
			}

			logcfg_clui_reset_ctx(&ctx);
			ret = clui_parse_cmd(&logcfg_clui_shell_cmd,
			                     &parser,
			                     (int)expr.nr,
			                     expr.words,
			                     &ctx);
			if (!ret)
				ctx.exec(&ctx, &parser);

			clui_shell_free_expr(&expr);

			if (ret == -ESHUTDOWN) {
				ret = 0;
				break;
			}
		}

		clui_shell_fini(&parser);
	}
	else
		ret = ctx.exec(&ctx, &parser);

fini:
	if (!ret)
		ret = logcfg_clui_fini();
	else
		logcfg_clui_fini();

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

#include "clui.h"
#include "dbase/session.h"
#include "dbase/kvstore.h"
#include "dbase/rule.h"
#include "common/conf.h"
#include <clui/shell.h>
#include <utils/signal.h>
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

#define LOGCFG_CLUI_SHELL_HELP \
	"Synopsis:\n" \
	LOGCFG_CLUI_TOP_RULE_HELP \
	"\n" \
	"    quit\n" \
	"    Quit interactive shell.\n" \
	"\n" \
	"    help\n" \
	"    This help message.\n"

static void
logcfg_clui_shell_cmd_help(const struct clui_cmd    * cmd __unused,
                           const struct clui_parser * parser __unused,
                           FILE                     * stdio)
{
	fprintf(stdio, LOGCFG_CLUI_SHELL_HELP, "", "");
}

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
	.help  = logcfg_clui_shell_cmd_help
};

/******************************************************************************
 * Top-level command
 ******************************************************************************/

#define LOGCFG_CLUI_TOP_HELP \
	"Usage:\n" \
	"    %1$s -- Manage syslog daemon configuration.\n" \
	"\n" \
	"Synopsis:\n" \
	"    %1$s%2$sshell\n" \
	"    Run in interactive shell mode.\n" \
	"\n" \
	LOGCFG_CLUI_TOP_RULE_HELP \
	"\n" \
	"    %1$s help\n" \
	"    This help message.\n" \
	"\n" \
	"With [OPTIONS]:\n" \
	"    -c | --config <CONFIG_PATH> use CONFIG_PATH as pathname to configuration\n" \
	"                                file\n" \
	"    -d | --dbdir <DBDIR_PATH>   use DBDIR_PATH as pathname to database\n" \
	"                                directory. Defaults to:\n" \
	"                                [" LOGCFG_LOCALSTATEDIR "]\n"

static void
logcfg_clui_top_cmd_help(const struct clui_cmd    * cmd __unused,
                         const struct clui_parser * parser,
                         FILE                     * stdio)
{
	fprintf(stdio,
	        LOGCFG_CLUI_TOP_HELP,
	        clui_prefix(parser),
	        " [OPTIONS] ");
}

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

	return clui_shell_build_static_matches(matches, array_nr(matches));
}

static const struct clui_cmd logcfg_clui_top_cmd = {
	.parse    = logcfg_clui_parse_top,
	.complete = logcfg_clui_complete_top,
	.help     = logcfg_clui_top_cmd_help
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

	for (m = 0; m < array_nr(logcfg_clui_modules); m++) {
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
	logcfg_clui_dofini_modules(array_nr(logcfg_clui_modules));
}

/******************************************************************************
 * Main handling
 ******************************************************************************/

static struct elog_stdio       logcfg_clui_logger;
struct logcfg_session *        logcfg_clui_sess;
static struct kvs_repo *       logcfg_clui_dbase;

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

	err = logcfg_clui_init_modules();
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

	logcfg_clui_fini_modules();

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

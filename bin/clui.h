#ifndef _LOGCFG_BIN_CLUI_H
#define _LOGCFG_BIN_CLUI_H

#include <clui/clui.h>
#include <stdbool.h>

#define logcfg_clui_err(_parser, _error, _format, ...) \
	clui_err(_parser, \
	         _format ": %s (%d).", \
	         ## __VA_ARGS__, \
	         logcfg_strerror(_error), \
	         _error)

extern bool logcfg_clui_shell_mode;

extern void
logcfg_clui_sched_help(void * ctx, const struct clui_cmd * cmd);

struct logcfg_clui_ctx;

typedef int (logcfg_clui_exec_fn)
            (const struct logcfg_clui_ctx * ctx,
             const struct clui_parser     * parser);

struct logcfg_clui_ctx {
	const struct clui_cmd * cmd;
	logcfg_clui_exec_fn *   exec;
	union {
		bool            shell;
	};
};

extern void
logcfg_clui_sched_exec(void * ctx, logcfg_clui_exec_fn * exec);

struct logcfg_session;

typedef int (logcfg_clui_module_init_fn)(struct logcfg_session *);
typedef void (logcfg_clui_module_fini_fn)(void);

struct logcfg_clui_module {
	struct clui_cmd              cmd;
	logcfg_clui_module_init_fn * init;
	logcfg_clui_module_fini_fn * fini;
};

/******************************************************************************
 * Rule support
 ******************************************************************************/

#define LOGCFG_CLUI_TOP_RULE_HELP \
	"    %1$s%2$srule show | help\n" \
	"    Show syslog message matching rules.\n"

extern const struct logcfg_clui_module logcfg_clui_rule_module;

#endif /* _LOGCFG_BIN_CLUI_H */

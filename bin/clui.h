#ifndef _LOGCFG_BIN_CLUI_H
#define _LOGCFG_BIN_CLUI_H

#include "logcfg/common.h"
#include <clui/clui.h>
#include <stdbool.h>

struct logcfg_session;

extern struct logcfg_session * logcfg_clui_sess;

typedef int (logcfg_clui_module_init_fn)(void);
typedef void (logcfg_clui_module_fini_fn)(void);

struct logcfg_clui_module {
	struct clui_cmd              cmd;
	logcfg_clui_module_init_fn * init;
	logcfg_clui_module_fini_fn * fini;
};

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

extern void
logcfg_clui_sched_help(void * ctx, const struct clui_cmd * cmd);

#define logcfg_clui_err(_parser, _error, _format, ...) \
	clui_err(_parser, \
	         _format ": %s (%d).", \
	         ## __VA_ARGS__, \
	         logcfg_strerror(_error), \
	         _error)

extern void
logcfg_clui_display_help(const char * __restrict               brief,
                         const char * __restrict               desc,
                         const char * __restrict               spec,
                         const struct clui_parser * __restrict parser,
                         FILE * __restrict                     stdio)
	__logcfg_nonull(1, 2, 3, 4, 5);

#define LOGCFG_CLUI_DEFINE_HELP(_func, _brief, _desc, _spec) \
	static void \
	_func(const struct clui_cmd * __restrict    cmd __unused, \
	      const struct clui_parser * __restrict parser, \
	      FILE * __restrict                     stdio) \
	{ \
		logcfg_clui_display_help(_brief, _desc, _spec, parser, stdio); \
	}

extern struct dmod_xact *
logcfg_clui_begin_xact(const struct clui_parser * __restrict parser)
	__logcfg_nonull(1);

extern int
logcfg_clui_end_xact(struct dmod_xact *                    xact,
                     int                                   status,
                     const struct clui_parser * __restrict parser)
	__logcfg_nonull(1, 3);

extern int
logcfg_clui_abort_xact(struct dmod_xact *                    xact,
                       int                                   status,
                       const struct clui_parser * __restrict parser)
	__logcfg_nonull(1, 3);

/******************************************************************************
 * Rule support
 ******************************************************************************/

#define LOGCFG_CLUI_TOP_RULE_HELP \
	"    rule     help|<RULE_SPEC>     -- Manage syslog daemon matching rules.\n"

extern const struct logcfg_clui_module logcfg_clui_rule_module;

/******************************************************************************
 * Selector support
 ******************************************************************************/

#define LOGCFG_CLUI_TOP_SELECTOR_HELP \
	"    selector help|<SELECTOR_SPEC> -- Manage syslog daemon message selectors.\n"

extern const struct logcfg_clui_module logcfg_clui_selector_module;

#endif /* _LOGCFG_BIN_CLUI_H */

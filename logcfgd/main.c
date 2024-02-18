#include "conf.h"
#include "rule.h"
#include <stdlib.h>
#include <stroll/cdefs.h>

static struct elog_stdio      stdcfg_logger;
struct elog *                 logcfg_logger = (struct elog *)&stdcfg_logger;
static struct elog_stdio_conf stdlog_conf = {
	.super.severity = CONFIG_LOGCFG_DAEMON_LOG_SEVERITY,
	.format         = CONFIG_LOGCFG_DAEMON_STDLOG_FORMAT
};

int
main(int argc, char * const argv[])
{
	int                                    ret;
	static const struct logcfg_conf_loader loaders[] = {
		{ .name = "rules", .load = logcfg_rule_load_conf }
	};

	elog_init_stdio(&stdcfg_logger, &stdlog_conf);

	logcfg_rule_init_repo();

	ret = logcfg_conf_load_file(argv[1], loaders, array_nr(loaders));
	if (ret)
		goto exit;

	const struct logcfg_rule * rule;
	logcfg_rule_foreach(rule)
		printf("%u: %s\n",
		       logcfg_rule_get_id(rule),
		       logcfg_rule_get_name(rule));

exit:
#if defined(CONFIG_LOGCFG_DEBUG)
	logcfg_rule_fini_repo();

	elog_fini(logcfg_logger);
#endif /* defined(CONFIG_LOGCFG_DEBUG) */

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

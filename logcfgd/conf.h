/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_INTERN_CONF_H
#define _LOGCFG_INTERN_CONF_H

#include "common.h"
#include <libconfig.h>

typedef int (logcfg_conf_load_setting_fn)
            (const config_setting_t * __restrict setting);

struct logcfg_conf_loader {
	const char *                  name;
	logcfg_conf_load_setting_fn * load;
};

#define logcfg_conf_log(_setting, _severity, _format, ...) \
	({ \
		const config_setting_t * __set = _setting; \
		enum elog_severity       __svrt = _severity; \
		const char *             __path; \
		unsigned int             __line; \
		const char *             __name; \
		\
		__path = config_setting_source_file(__set); \
		logcfg_assert_intern(__path); \
		__line = config_setting_source_line(__set); \
		logcfg_assert_intern(__line); \
		__name = config_setting_name(__set); \
		\
		if (__name) \
			elog_log(logcfg_logger, \
			         __svrt, \
			         "%s:%d: '%s': " _format ".", \
			         __path, \
			         __line, \
			         __name, \
			         ## __VA_ARGS__); \
		else \
			elog_log(logcfg_logger, \
			         __svrt, \
			         "%s:%d: " _format ".", \
			         __path, \
			         __line, \
			         ## __VA_ARGS__); \
	 })

#define logcfg_conf_err(_setting, _format, ...) \
	logcfg_conf_log(_setting, ELOG_ERR_SEVERITY, _format, ## __VA_ARGS__)

#define logcfg_conf_warn(_setting, _format, ...) \
	logcfg_conf_log(_setting, ELOG_ERR_SEVERITY, _format, ## __VA_ARGS__)

#define logcfg_conf_info(_setting, _format, ...) \
	logcfg_conf_log(_setting, ELOG_INFO_SEVERITY, _format, ## __VA_ARGS__)

extern int __logcfg_nonull(1, 2)
logcfg_conf_load_file(const char * __restrict         path,
                      const struct logcfg_conf_loader loaders[__restrict_arr],
                      unsigned int                    count);

#endif /* _LOGCFG_INTERN_CONF_H */

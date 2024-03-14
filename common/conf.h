/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_COMMON_CONF_H
#define _LOGCFG_COMMON_CONF_H

#include "common/common.h"
#include <libconfig.h>

extern void
logcfg_conf_log_attrs(const config_setting_t *  setting,
                      const char ** __restrict  path,
                      unsigned int * __restrict line,
                      const char ** __restrict  name)
	__logcfg_nonull(1, 2, 3, 4) __logcfg_export;

#define logcfg_conf_log(_setting, _severity, _format, ...) \
	({ \
	        struct elog *      __log = logcfg_logger; \
		enum elog_severity __svrt = _severity; \
		const char *       __path; \
		unsigned int       __line; \
		const char *       __name; \
		\
		logcfg_conf_log_attrs(_setting, &__path, &__line, &__name); \
		\
		if (__name) \
			elog_log(__log, \
			         __svrt, \
			         "%s:%u: '%s': " _format ".", \
			         __path, \
			         __line, \
			         __name, \
			         ## __VA_ARGS__); \
		else \
			elog_log(__log, \
			         __svrt, \
			         "%s:%u: " _format ".", \
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

typedef int (logcfg_conf_load_setting_fn)
            (const config_setting_t * __restrict setting);

struct logcfg_conf_loader {
	const char *                  name;
	logcfg_conf_load_setting_fn * load;
};

extern int
logcfg_conf_load_file(const char * __restrict         path,
                      const struct logcfg_conf_loader loaders[__restrict_arr],
                      unsigned int                    count)
	__logcfg_nonull(1, 2) __logcfg_export;

#endif /* _LOGCFG_COMMON_CONF_H */

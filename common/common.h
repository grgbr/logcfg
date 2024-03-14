/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_COMMON_COMMON_H
#define _LOGCFG_COMMON_COMMON_H

#include <logcfg/common.h>
#include <elog/elog.h>

struct elog;

extern struct elog * logcfg_logger __logcfg_export;

#define logcfg_err(_format, ...) \
	elog_err(logcfg_logger, _format ".", ## __VA_ARGS__)

#define logcfg_info(_format, ...) \
	elog_info(logcfg_logger, _format ".", ## __VA_ARGS__)

#if defined(CONFIG_LOGCFG_DEBUG)

#define logcfg_debug(_format, ...) \
	elog_debug(logcfg_logger, _format ".", ## __VA_ARGS__)

#else  /* !defined(CONFIG_LOGCFG_DEBUG) */

#define logcfg_debug(_format, ...)

#endif /* defined(CONFIG_LOGCFG_DEBUG) */

#endif /* _LOGCFG_COMMON_COMMON_H */

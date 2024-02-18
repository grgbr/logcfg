/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Common definitions
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      17 Feb 2024
 * @copyright Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _LOGCFG_CDEFS_H
#define _LOGCFG_CDEFS_H

#include <logcfg/config.h>
#include <stroll/cdefs.h>

#define __logcfg_export  __export_public

#if defined(CONFIG_LOGCFG_ASSERT_API) || defined(CONFIG_LOGCFG_ASSERT_INTERN)

#define __logcfg_nonull(_arg_index, ...)
#define __logcfg_const
#define __logcfg_pure
#define __logcfg_nothrow

#else   /* !(defined(CONFIG_LOGCFG_ASSERT_API) || \
             defined(CONFIG_LOGCFG_ASSERT_INTERN)) */

#define __logcfg_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define __logcfg_const   __const
#define __logcfg_pure    __pure
#define __logcfg_nothrow __nothrow

#endif /* defined(CONFIG_LOGCFG_ASSERT_API) || \
          defined(CONFIG_LOGCFG_ASSERT_INTERN) */

#endif /* _LOGCFG_CDEFS_H */

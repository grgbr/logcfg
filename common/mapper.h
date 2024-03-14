/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_COMMON_MAPPER_H
#define _LOGCFG_COMMON_MAPPER_H

#include <logcfg/mapper.h>

extern void
logcfg_mapper_repo_init(struct logcfg_mapper_repo *           mappers,
                        const struct logcfg_mapper_repo_ops * ops);

extern void
logcfg_mapper_repo_fini(struct logcfg_mapper_repo * mappers);

#endif /* _LOGCFG_COMMON_SELECTOR_H */

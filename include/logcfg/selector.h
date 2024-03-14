/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_SELECTOR_H
#define _LOGCFG_SELECTOR_H

#include <logcfg/common.h>
#include <dmod/dmod.h>
#include <dpack/lvstr.h>

/******************************************************************************
 * Selector object
 ******************************************************************************/

struct logcfg_selector {
	struct dmod_object  dmod;
	uint64_t            id;
	struct stroll_lvstr name;
};

#define LOGCFG_SELECTOR_NAMESZ_MAX  (64U)
#define LOGCFG_SELECTOR_NAMELEN_MAX (LOGCFG_SELECTOR_NAMESZ_MAX - 1)

#define LOGCFG_SELECTOR_PACKSZ(_name_len) \
	DPACK_STR_SIZE(_name_len)

#define LOGCFG_SELECTOR_PACKSZ_MIN \
	DPACK_STR_SIZE(1)

#define LOGCFG_SELECTOR_PACKSZ_MAX \
	DPACK_STR_SIZE(LOGCFG_SELECTOR_NAMELEN_MAX)

#define LOGCFG_SELECTOR_INIT \
	{ .dmod = DMOD_INIT, .id = 0, .name = STROLL_LVSTR_INIT }

extern int
logcfg_selector_check(const struct logcfg_selector * __restrict selector)
	__logcfg_nonull(1) __logcfg_export;

extern int
logcfg_selector_check_name(const char * __restrict name)
	__logcfg_nonull(1) __logcfg_export;

#if defined(CONFIG_LOGCFG_ASSERT_API)

extern int
logcfg_selector_get_name(struct logcfg_selector * __restrict     selector,
                         const struct stroll_lvstr ** __restrict name)
	__logcfg_nonull(1, 2) __logcfg_export;

#else  /* !defined(CONFIG_LOGCFG_ASSERT_API) */

static inline int
logcfg_selector_get_name(struct logcfg_selector * __restrict     selector,
                         const struct stroll_lvstr ** __restrict name)
{
	*name  = &selector->name;

	return 0;
}

#endif /* defined(CONFIG_LOGCFG_ASSERT_API) */

extern void
logcfg_selector_lend_name(struct logcfg_selector * __restrict selector,
                          const char *                        name)
	__logcfg_nonull(1, 2) __logcfg_export;

extern void
logcfg_selector_cede_name(struct logcfg_selector * __restrict selector,
                          char *                              name)
	__logcfg_nonull(1, 2) __logcfg_export;

static inline size_t
logcfg_selector_packsz(const struct logcfg_selector  * __restrict selector)
{
	logcfg_assert_api(selector);
	logcfg_assert_api(logcfg_selector_check(selector));

	return dpack_str_size(stroll_lvstr_len(&selector->name));
}

extern int
logcfg_selector_init(struct logcfg_selector * __restrict selector)
	__logcfg_nonull(1) __logcfg_export;

extern void
logcfg_selector_fini(struct logcfg_selector * __restrict selector)
	__logcfg_nonull(1) __logcfg_export;

extern struct logcfg_selector *
logcfg_selector_create(void)
	__logcfg_export;

extern void
logcfg_selector_destroy(struct logcfg_selector * selector)
	__logcfg_nonull(1) __logcfg_export;

/******************************************************************************
 * Selector object mapper interface
 ******************************************************************************/

struct logcfg_selector_mapper;

extern struct logcfg_selector *
logcfg_selector_get_byid(struct logcfg_selector_mapper * mapper,
                         uint64_t                        id,
                         void *                          data)
	__logcfg_nonull(1) __logcfg_export;

extern int
logcfg_selector_load_byid(struct logcfg_selector_mapper * mapper,
                          uint64_t                        id,
                          struct logcfg_selector *        selector,
                          void *                          data)
	__logcfg_nonull(1, 3) __logcfg_export;

extern int
logcfg_selector_save(struct logcfg_selector_mapper * mapper,
                     struct logcfg_selector *        selector,
                     void *                          data)
	__logcfg_nonull(1, 2) __logcfg_export;

#endif /* _LOGCFG_SELECTOR_H */

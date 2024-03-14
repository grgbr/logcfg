/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_COMMON_SELECTOR_H
#define _LOGCFG_COMMON_SELECTOR_H

#include <logcfg/selector.h>
#include <dpack/codec.h>

/******************************************************************************
 * Selector object
 ******************************************************************************/

extern int
logcfg_selector_pack(const struct logcfg_selector * __restrict selector,
                     struct dpack_encoder *                    encoder)
	__logcfg_nonull(1, 2) __logcfg_export;

extern int
logcfg_selector_unpack(struct logcfg_selector * __restrict selector,
                       struct dpack_decoder *              decoder)
	__logcfg_nonull(1, 2) __logcfg_export;

extern int
logcfg_selector_unpackn_check(struct logcfg_selector * __restrict selector,
                              struct dpack_decoder *              decoder)
	__logcfg_nonull(1, 2) __logcfg_export;

/******************************************************************************
 * Selector object mapper interface
 ******************************************************************************/

typedef int (logcfg_selector_load_byid_fn)(struct logcfg_selector_mapper *,
                                           uint64_t,
                                           struct logcfg_selector *,
                                           void *);

struct logcfg_selector_mapper_ops {
	struct dmod_mapper_ops         dmod;
	logcfg_selector_load_byid_fn * load_byid;
};

#define logcfg_selector_mapper_ops_assert_api(_ops) \
	logcfg_assert_api(_ops); \
	dmod_mapper_ops_assert_api(&(_ops)->dmod); \
	logcfg_assert_api((_ops)->load_byid)

#define logcfg_selector_mapper_ops_assert_intern(_ops) \
	logcfg_assert_intern(_ops); \
	dmod_mapper_ops_assert_intern(&(_ops)->dmod); \
	logcfg_assert_intern((_ops)->load_byid)

struct logcfg_selector_mapper {
	struct dmod_mapper            dmod;
	union {
		struct dpack_encoder  encoder;
		struct dpack_decoder  decoder;
	};
};

#define logcfg_selector_mapper_assert_api(_mapper) \
	logcfg_assert_api(_mapper); \
	logcfg_selector_mapper_ops_assert_api( \
		logcfg_selector_mapper_to_ops(_mapper))

#define logcfg_selector_mapper_assert_intern(_mapper) \
	logcfg_assert_intern(_mapper); \
	logcfg_selector_mapper_ops_assert_intern( \
		logcfg_selector_mapper_to_ops(_mapper))

static const struct logcfg_selector_mapper_ops *
logcfg_selector_mapper_to_ops(const struct logcfg_selector_mapper * mapper)
{
	logcfg_assert_intern(mapper);

	return (const struct logcfg_selector_mapper_ops *)mapper->dmod.ops;
}

static inline void
logcfg_selector_mapper_init(struct logcfg_selector_mapper *           mapper,
                            const struct logcfg_selector_mapper_ops * ops)
{
	logcfg_assert_intern(mapper);
	logcfg_selector_mapper_ops_assert_intern(ops);

	dmod_mapper_init(&mapper->dmod, &ops->dmod);
}

static inline void
logcfg_selector_mapper_fini(struct logcfg_selector_mapper * mapper)
{
	logcfg_selector_mapper_assert_intern(mapper);

	dmod_mapper_fini(&mapper->dmod);
}

#endif /* _LOGCFG_COMMON_SELECTOR_H */

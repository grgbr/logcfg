/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "selector.h"

/******************************************************************************
 * Selector object
 ******************************************************************************/

static int
logcfg_selector_ncheck_name(const char * __restrict name, size_t length)
{
	logcfg_assert_intern(name);

	if (!length || length >= LOGCFG_SELECTOR_NAMESZ_MAX)
		return -EINVAL;

	return 0;
}

static int
logcfg_selector_assess_name(const struct logcfg_selector * __restrict selector)
{
	logcfg_assert_intern(selector);

	const char * name;

	name = stroll_lvstr_cstr(&selector->name);
	if (!name)
		return -EINVAL;

	return logcfg_selector_ncheck_name(name,
	                                   stroll_lvstr_len(&selector->name));
}

int
logcfg_selector_check(const struct logcfg_selector * __restrict selector)
{

	return logcfg_selector_assess_name(selector);
}

int
logcfg_selector_check_name(const char * __restrict name)
{
	logcfg_assert_api(name);

	return logcfg_selector_ncheck_name(name,
	                                   strnlen(name,
	                                           LOGCFG_SELECTOR_NAMESZ_MAX));
}

#if defined(CONFIG_LOGCFG_ASSERT_API)

int
logcfg_selector_get_name(struct logcfg_selector * __restrict     selector,
                         const struct stroll_lvstr ** __restrict name)
{
	logcfg_assert_api(selector);
	logcfg_assert_api(name);
	logcfg_assert_api(!dmod_object_is_empty(&selector->dmod));
	logcfg_assert_api(!logcfg_selector_assess_name(selector));

	*name  = &selector->name;

	return 0;
}

#endif /* defined(CONFIG_LOGCFG_ASSERT_API) */

void
logcfg_selector_lend_name(struct logcfg_selector * __restrict selector,
                          const char *                        name)
{
	logcfg_assert_api(!logcfg_selector_check_name(name));

	int err __unused;

	dmod_object_mark_dirty(&selector->dmod);

	err = stroll_lvstr_lend(&selector->name, name);
	logcfg_assert_intern(!err);
}

void
logcfg_selector_cede_name(struct logcfg_selector * __restrict selector,
                          char *                              name)
{
	logcfg_assert_api(!logcfg_selector_check_name(name));

	int err __unused;

	dmod_object_mark_dirty(&selector->dmod);

	err = stroll_lvstr_cede(&selector->name, name);
	logcfg_assert_intern(!err);
}

int
logcfg_selector_pack(const struct logcfg_selector * __restrict selector,
                     struct dpack_encoder *                    encoder)
{
	logcfg_assert_api(selector);
	logcfg_assert_api(logcfg_selector_check(selector));
	logcfg_assert_api(encoder);

	return dpack_encode_lvstr(encoder, &selector->name);
}

int
logcfg_selector_unpack(struct logcfg_selector * __restrict selector,
                       struct dpack_decoder *              decoder)
{
	logcfg_assert_api(selector);
	logcfg_assert_api(decoder);

	ssize_t err;

	err = dpack_decode_lvstr_max(decoder,
	                             LOGCFG_SELECTOR_NAMELEN_MAX,
	                             &selector->name);
	if (err < 0)
		return (int)err;

	logcfg_assert_api(!logcfg_selector_check(selector));

	return 0;
}

int
logcfg_selector_unpackn_check(struct logcfg_selector * __restrict selector,
                              struct dpack_decoder *              decoder)
{
	logcfg_assert_api(selector);
	logcfg_assert_api(decoder);

	char *  str;
	ssize_t len;

	len = dpack_decode_strdup_max(decoder,
	                              LOGCFG_SELECTOR_NAMELEN_MAX - 1,
	                              &str);
	if (len < 0)
		return (int)len;

	if (logcfg_selector_ncheck_name(str, (size_t)len)) {
		free(str);
		return -EINVAL;
	}

	stroll_lvstr_ncede(&selector->name, str, (size_t)len);

	return 0;
}

int
logcfg_selector_init(struct logcfg_selector * __restrict selector)
{
	dmod_object_init(&selector->dmod);
	selector->id = 0;
	stroll_lvstr_init(&selector->name);

	return 0;
}

void
logcfg_selector_fini(struct logcfg_selector * __restrict selector)
{
	stroll_lvstr_fini(&selector->name);
	dmod_object_fini(&selector->dmod);
}

static struct logcfg_selector *
logcfg_selector_alloc(void)
{
	return malloc(sizeof(struct logcfg_selector));
}

static void
logcfg_selector_free(struct logcfg_selector * selector)
{
	logcfg_assert_intern(selector);

	free(selector);
}

struct logcfg_selector *
logcfg_selector_create(void)
{
	struct logcfg_selector * sel;

	sel = logcfg_selector_alloc();
	if (!sel)
		return NULL;

	logcfg_selector_init(sel);

	return sel;
}

void
logcfg_selector_destroy(struct logcfg_selector * selector)
{
	logcfg_assert_api(selector);

	logcfg_selector_fini(selector);
	logcfg_selector_free(selector);
}

/******************************************************************************
 * Selector object mapper interface
 ******************************************************************************/

struct logcfg_selector *
logcfg_selector_get_byid(struct logcfg_selector_mapper * mapper,
                         uint64_t                        id,
                         void *                          data)
{
	logcfg_selector_mapper_assert_api(mapper);

	struct logcfg_selector * sel;
	int                      err;

	sel = logcfg_selector_create();
	if (!sel)
		return NULL;

	err = logcfg_selector_load_byid(mapper, id, sel, data);
	if (err) {
		logcfg_selector_destroy(sel);
		errno = -err;
		return NULL;
	}

	return sel;
}

int
logcfg_selector_load_byid(struct logcfg_selector_mapper * mapper,
                          uint64_t                        id,
                          struct logcfg_selector *        selector,
                          void *                          data)
{
	logcfg_selector_mapper_assert_api(mapper);
	logcfg_assert_api(selector);

	return logcfg_selector_mapper_to_ops(mapper)->load_byid(mapper,
	                                                        id,
	                                                        selector,
	                                                        data);
}

int
logcfg_selector_save(struct logcfg_selector_mapper * mapper,
                     struct logcfg_selector *        selector,
                     void *                          data)
{
	logcfg_selector_mapper_assert_api(mapper);
	logcfg_assert_api(selector);

	return dmod_mapper_save(&mapper->dmod, &selector->dmod, data);
}

#include "selector.h"
#include "session.h"
#include "kvstore.h"
#include <kvstore/autorec.h>

/******************************************************************************
 * Selector database table
 ******************************************************************************/

#define LOGCFG_DBASE_SELECTOR_BASENAME "selector"

enum logcfg_dbase_selector_indx {
	LOGCFG_DBASE_SELECTOR_NAME_INDX,
	LOGCFG_DBASE_SELECTOR_INDX_NR
};

static int
logcfg_dbase_selector_open_data(struct kvs_table *       table,
                                const struct kvs_depot * depot,
                                const struct kvs_xact *  xact,
                                mode_t                   mode)
{
	return kvs_autorec_open(kvs_table_get_data_store(table),
	                        depot,
	                        xact,
	                        LOGCFG_DBASE_SELECTOR_BASENAME ".dat",
	                        mode);
}

static int
logcfg_dbase_selector_close_data(const struct kvs_table * table)
{
	return kvs_autorec_close(kvs_table_get_data_store(table));
}

static int
logcfg_dbase_selector_bind_name_indx(const struct kvs_chunk * pkey __unused,
                                     const struct kvs_chunk * item,
                                     struct kvs_chunk *       skey)
{
	logcfg_assert_intern(pkey->size);
	logcfg_assert_intern(pkey->data);
	logcfg_assert_intern(item->size >= LOGCFG_SELECTOR_PACKSZ_MIN);
	logcfg_assert_intern(item->size <= LOGCFG_SELECTOR_PACKSZ_MAX);
	logcfg_assert_intern(item->data);
	logcfg_assert_intern(item->priv);
	logcfg_assert_intern(skey);

	const struct stroll_lvstr * name = item->priv;

	skey->data = stroll_lvstr_cstr(name);
	skey->size = stroll_lvstr_len(name);

	return 0;
}

static int
logcfg_dbase_selector_open_name_indx(struct kvs_table *       table,
                                     const struct kvs_depot * depot,
                                     const struct kvs_xact *  xact,
                                     mode_t                   mode)
{
	struct kvs_store * indx;

	indx = kvs_table_get_indx_store(table, LOGCFG_DBASE_SELECTOR_NAME_INDX);

	return kvs_open_indx(indx,
	                     &table->data,
	                     depot,
	                     xact,
	                     LOGCFG_DBASE_SELECTOR_BASENAME ".idx",
	                     "name",
	                     mode,
	                     logcfg_dbase_selector_bind_name_indx);
}

static int
logcfg_dbase_selector_close_name_indx(const struct kvs_table * table)
{
	return kvs_close_indx(
		kvs_table_get_indx_store(table,
		                         LOGCFG_DBASE_SELECTOR_NAME_INDX));
}

const struct kvs_table_desc logcfg_dbase_selector_table = {
	.data_ops = {
		.open  = logcfg_dbase_selector_open_data,
		.close = logcfg_dbase_selector_close_data
	},
	.indx_nr  = LOGCFG_DBASE_SELECTOR_INDX_NR,
	.indx_ops = {
		[LOGCFG_DBASE_SELECTOR_NAME_INDX] = {
			.open  = logcfg_dbase_selector_open_name_indx,
			.close = logcfg_dbase_selector_close_name_indx
		}
	}
};

/******************************************************************************
 * Selector database mapper
 ******************************************************************************/

struct logcfg_selector_dbase_mapper {
	struct logcfg_selector_mapper base;
	const struct kvs_table *      table;
	char                          buffer[LOGCFG_SELECTOR_PACKSZ_MAX];
};

static int
logcfg_selector_dbase_load_byid(struct logcfg_selector_mapper * mapper,
                                uint64_t                        id,
                                struct logcfg_selector *        object,
                                void *                          data)
{
	logcfg_assert_intern(mapper);
	logcfg_assert_intern(object);

	struct logcfg_selector_dbase_mapper * map =
		(struct logcfg_selector_dbase_mapper *)mapper;
	struct kvs_chunk                      item;
	int                                   err;

	err = kvs_autorec_get_byid(kvs_table_get_data_store(map->table),
	                           data,
	                           id,
	                           &item);
	if (err)
		return err;

	logcfg_assert_intern(item.size >= LOGCFG_SELECTOR_PACKSZ_MIN);
	logcfg_assert_intern(item.size <= LOGCFG_SELECTOR_PACKSZ_MAX);

	dpack_decoder_init_buffer(&mapper->decoder, item.data, item.size);
	err = logcfg_selector_unpack(object, &mapper->decoder);
	dpack_decoder_fini(&mapper->decoder);
	if (err)
		return err;

	logcfg_assert_intern(dpack_decoder_data_left(&mapper->decoder) == 0);

	return 0;
}

static int
logcfg_selector_dbase_save(struct dmod_mapper * mapper,
                           struct dmod_object * object,
                           void *               data)
{
	logcfg_assert_intern(mapper);
	logcfg_assert_intern(object);

	struct logcfg_selector_dbase_mapper * map =
		(struct logcfg_selector_dbase_mapper *)mapper;
	struct logcfg_selector *              sel =
		(struct logcfg_selector *)object;
	struct kvs_chunk                      item;
	const struct kvs_xact *               xact = data;
	int                                   ret;

	logcfg_selector_mapper_assert_api(&map->base);

	ret = logcfg_selector_check(sel);
	if (ret)
		return ret;

	dpack_encoder_init_buffer(&map->base.encoder,
	                          map->buffer,
	                          LOGCFG_SELECTOR_PACKSZ_MAX);
	ret = logcfg_selector_pack(sel, &map->base.encoder);
	logcfg_assert_intern(!ret);

	item.data = map->buffer;
	item.size = dpack_encoder_space_used(&map->base.encoder);
	logcfg_assert_intern(item.size >= LOGCFG_SELECTOR_PACKSZ_MIN);
	logcfg_assert_intern(item.size <= LOGCFG_SELECTOR_PACKSZ_MAX);

	/*
	 * Store pointer to name so that logcfg_dbase_selector_bind_name_indx()
	 * may properly update name index...
	 */
	logcfg_selector_get_name(sel, (const struct stroll_lvstr **)&item.priv);

	if (!kvs_autorec_id_isok(sel->id))
		ret = kvs_autorec_add(kvs_table_get_data_store(map->table),
		                      xact,
		                      &sel->id,
		                      &item);
	else
		ret = kvs_autorec_update(kvs_table_get_data_store(map->table),
		                         xact,
		                         sel->id,
		                         &item);
	if (ret)
		goto fini;

	logcfg_assert_intern(kvs_autorec_id_isok(sel->id));

fini:
	dpack_encoder_fini(&map->base.encoder, DPACK_DONE);

	return ret;
}

const struct logcfg_selector_mapper_ops logcfg_selector_dbase_mapper_ops = {
	.dmod = {
		.save = logcfg_selector_dbase_save
	},
	.load_byid    = logcfg_selector_dbase_load_byid
};

struct logcfg_selector_mapper *
logcfg_selector_dbase_mapper_create(const struct logcfg_session * session)
{
	logcfg_assert_intern(session);

	struct logcfg_selector_dbase_mapper * map;

	map = malloc(sizeof(*map));
	if (!map)
		return NULL;

	logcfg_selector_mapper_init(&map->base,
	                            &logcfg_selector_dbase_mapper_ops);

	map->table = logcfg_dbase_get_table(
		logcfg_dbase_session_get_kvdb(session),
		LOGCFG_DBASE_SELECTOR_TID);

	return &map->base;
}

void
logcfg_selector_dbase_mapper_destroy(struct logcfg_selector_mapper * mapper)
{
	logcfg_selector_mapper_fini(mapper);

	free(mapper);
}

#include "selector.h"

#define LOGCFG_SELECTOR_BASENAME "selector"

enum logcfg_selector_indx {
	LOGCFG_SELECTOR_NAME_INDX,
	LOGCFG_SELECTOR_INDX_NR,
}

static int
logcfg_selector_open_data(struct kvs_table       *table,
                          const struct kvs_depot *depot,
                          const struct kvs_xact  *xact,
                          mode_t                  mode)
{
	return kvs_autorec_open(kvs_table_get_data_store(table),
	                        depot,
	                        xact,
	                        LOGCFG_SELECTOR_BASENAME ".dat",
	                        mode);
}

static int
logcfg_selector_close_data(const struct kvs_table *table)
{
	return kvs_autorec_close(kvs_table_get_data_store(table));
}

static int
logcfg_selector_bind_name_indx(const struct kvs_chunk *pkey __unused,
                               const struct kvs_chunk *item,
                               struct kvs_chunk       *skey)
{
	nwif_assert(pkey->size);
	nwif_assert(pkey->data);
	nwif_assert(item->size);
	nwif_assert(item->data);
	nwif_assert(skey);

	const struct nwif_iface_conf_data *data;

	data = (struct nwif_iface_conf_data *)item->data;

	nwif_assert(item->size > sizeof(*data));
	nwif_iface_conf_assert_data(data);

	if (!nwif_iface_conf_data_has_attr(data, NWIF_NAME_ATTR)) {
		skey->size = 0;
		return 0;
	}

	nwif_assert(unet_check_iface_name(data->name) > 0);

	skey->data = data->name;
	skey->size = strlen(data->name);

	return 0;
}

static int
logcfg_selector_open_name_indx(struct kvs_table       *table,
                               const struct kvs_depot *depot,
                               const struct kvs_xact  *xact,
                               mode_t                  mode)
{
	struct kvs_store *indx;

	indx = kvs_table_get_indx_store(table, LOGCFG_SELECTOR_NAME_INDX);

	return kvs_open_indx(indx,
	                     &table->data,
	                     depot,
	                     xact,
	                     LOGCFG_SELECTOR_BASENAME ".idx",
	                     "name",
	                     mode,
	                     logcfg_selector_bind_name_indx);
}

static int
logcfg_selector_close_name_indx(const struct kvs_table *table)
{
	return kvs_close_indx(
		kvs_table_get_indx_store(table, LOGCFG_SELECTOR_NAME_INDX));
}


const struct kvs_table_desc logcfg_selector_table = {
	.data_ops = {
		.open  = logcfg_selector_open_data,
		.close = logcfg_selector_close_data
	},
	.indx_nr  = LOGCFG_SELECTOR_INDX_NR,
	.indx_ops = {
		[LOGCFG_SELECTOR_NAME_INDX] = {
			.open  = logcfg_selector_open_name_indx,
			.close = logcfg_selector_close_name_indx
		}
	}
};

#include "conf.h"
#include <utils/path.h>

#define logcfg_conf_loader_assert(_loader) \
	logcfg_assert_intern((_loader)->name); \
	logcfg_assert_intern((_loader)->name[0]); \
	logcfg_assert_intern((_loader)->load)

static int __logcfg_nonull(1, 2, 3)
logcfg_conf_load_root(const config_setting_t * __restrict          setting,
                      const struct logcfg_conf_loader * __restrict loaders,
                      unsigned int                                 count)
{
	logcfg_assert_intern(setting);
	logcfg_assert_intern(config_setting_is_group(setting));
	logcfg_assert_intern(loaders);
	logcfg_assert_intern(count);

	int          cnt;
	unsigned int s;

	cnt = config_setting_length(setting);
	logcfg_assert_intern(cnt >= 0);
	if (!cnt) {
		if (config_setting_is_root(setting))
			logcfg_err("%s: empty configuration not allowed",
			           config_setting_source_file(setting));
		else
			logcfg_conf_err(setting,
			                "empty setting not allowed");
		return -ENODATA;
	}

	for (s = 0; s < (unsigned int)cnt; s++) {
		const config_setting_t * set;
		const char *             name;
		unsigned int             l;
		int                      err;

		set = config_setting_get_elem(setting, s);
		logcfg_assert_intern(set);

		/* Cannot be empty as parent setting is a group setting. */
		name = config_setting_name(set);
		logcfg_assert_intern(name);
		logcfg_assert_intern(name[0]);

		for (l = 0; l < count; l++) {
			logcfg_conf_loader_assert(&loaders[l]);
			if (!strcmp(name, loaders[l].name))
				break;
		}

		if (l == count) {
			logcfg_conf_err(set, "unknown setting");
			return -EINVAL;
		}

		err = loaders[l].load(set);
		if (err)
			return err;
	}

	return 0;
}

static int __logcfg_nonull(1, 2)
logcfg_conf_load(config_t * __restrict           conf,
                 const struct logcfg_conf_loader loaders[__restrict_arr],
                 unsigned int                    count)
{
	logcfg_assert_intern(conf);
	logcfg_assert_intern(loaders);
	logcfg_assert_intern(count);

	const config_setting_t * root;

	/*
	 * Root / top-level setting should always exist since already parsed by
	 * caller.
	 */
	root = config_root_setting(conf);
	logcfg_assert_intern(root);

	return logcfg_conf_load_root(root, loaders, count);
}

int
logcfg_conf_load_file(const char * __restrict         path,
                      const struct logcfg_conf_loader loaders[__restrict_arr],
                      unsigned int                    count)
{
	logcfg_assert_intern(upath_validate_path_name(path) > 0);
	logcfg_assert_intern(loaders);
	logcfg_assert_intern(count);

	config_t lib;
	int      err;

	config_init(&lib);

	/*
	 * Setup default parser options:
	 * - no setting value automatic type conversion,
	 * - no duplicate settings,
	 * - no required semicolon separators.
	 */
	config_set_options(&lib, 0);

	if (!config_read_file(&lib, path)) {
		switch (config_error_type(&lib)) {
		case CONFIG_ERR_FILE_IO:
			err = -errno;
			logcfg_err("%s: cannot load file: %s (%d)",
			           path,
			           strerror(-err),
			           -err);
			goto destroy;

		case CONFIG_ERR_PARSE:
			err = -EINVAL;
			logcfg_err("%s:%d: '%s'",
			           config_error_file(&lib),
			           config_error_line(&lib),
			           config_error_text(&lib));
			goto destroy;

		default:
			logcfg_assert_intern(0);
		}
	}

	err = logcfg_conf_load(&lib, loaders, count);
	if (err)
		goto destroy;

	config_destroy(&lib);

	return 0;

destroy:
	logcfg_err("%s: invalid configuration", path);
	config_destroy(&lib);

	return err;
}

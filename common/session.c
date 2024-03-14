/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "session.h"
#include "mapper.h"
#include <stdlib.h>

void
logcfg_session_init(struct logcfg_session *               session,
                    const struct logcfg_session_ops *     sess_ops,
                    const struct logcfg_mapper_repo_ops * maps_ops)
{
	logcfg_assert_intern(session);
	logcfg_session_ops_assert_intern(sess_ops);

	session->ops = sess_ops;
	logcfg_mapper_repo_init(&session->mappers, maps_ops);
}

void
logcfg_session_destroy(struct logcfg_session * session)
{
	logcfg_session_assert_api(session);

	logcfg_mapper_repo_fini(&session->mappers);
	session->ops->fini(session);
	free(session);
}

/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of LogCfg.
 * Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _LOGCFG_SESSION_H
#define _LOGCFG_SESSION_H

#include <logcfg/mapper.h>

struct logcfg_session;

typedef struct dmod_xact *
        (logcfg_session_xact_fn)(const struct logcfg_session *);

typedef void
        (logcfg_session_fini_fn)(struct logcfg_session *);

typedef const char *
        (logcfg_session_errstr_fn)(const struct logcfg_session *, int);

struct logcfg_session_ops {
	logcfg_session_xact_fn *   xact;
	logcfg_session_fini_fn *   fini;
	logcfg_session_errstr_fn * errstr;
};

#define logcfg_session_ops_assert_api(_ops) \
	logcfg_assert_api(_ops); \
	logcfg_assert_api((_ops)->xact); \
	logcfg_assert_api((_ops)->fini); \
	logcfg_assert_api((_ops)->errstr); \

struct logcfg_mapper_repo;

struct logcfg_session {
	const struct logcfg_session_ops * ops;
	struct logcfg_mapper_repo         mappers;
};

#define logcfg_session_assert_api(_session) \
	logcfg_assert_api(_session); \
	logcfg_session_ops_assert_api((_session)->ops)

static inline struct logcfg_mapper_repo *
logcfg_session_get_mappers(struct logcfg_session * session)
{
	logcfg_session_assert_api(session);

	return &session->mappers;
}

#define logcfg_session_get_mapper(_session, _mapper) \
	logcfg_mapper_get_ ## _mapper(logcfg_session_get_mappers(_session), \
	                              _session)

static inline struct dmod_xact * __logcfg_nonull(1)
logcfg_session_create_xact(const struct logcfg_session * session)
{
	logcfg_session_assert_api(session);

	return session->ops->xact(session);
}

static inline const char * __logcfg_nonull(1)
logcfg_session_strerror(const struct logcfg_session * session, int error)
{
	logcfg_session_assert_api(session);

	return session->ops->errstr(session, error);
}

extern void
logcfg_session_destroy(struct logcfg_session * session)
	__logcfg_nonull(1) __logcfg_export;

#endif /* _LOGCFG_SESSION_H */

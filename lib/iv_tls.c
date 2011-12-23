/*
 * ivykis, an event handling library
 * Copyright (C) 2011 Lennert Buytenhek
 * Dedicated to Marija Kulikova.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version
 * 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 2.1 for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License version 2.1 along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <iv_list.h>
#include <iv_tls.h>
#include <syslog.h>
#include "iv_private.h"

static int inited;
static off_t last_offset;
static struct iv_list_head iv_tls_users = IV_LIST_HEAD_INIT(iv_tls_users);

void iv_tls_thread_init(struct iv_state *st)
{
	struct iv_list_head *ilh;

	inited = 1;

	st->tls_ptr = malloc(last_offset);
	if (st->tls_ptr == NULL) {
		syslog(LOG_CRIT, "iv_tls_thread_init: out of memory");
		abort();
	}

	iv_list_for_each (ilh, &iv_tls_users) {
		struct iv_tls_user *itu;

		itu = iv_container_of(ilh, struct iv_tls_user, list);
		if (itu->init_thread != NULL)
			itu->init_thread(st->tls_ptr + itu->state_offset);
	}
}

void iv_tls_thread_deinit(struct iv_state *st)
{
	struct iv_list_head *ilh;

	iv_list_for_each (ilh, &iv_tls_users) {
		struct iv_tls_user *itu;

		itu = iv_container_of(ilh, struct iv_tls_user, list);
		if (itu->deinit_thread != NULL)
			itu->deinit_thread(st->tls_ptr + itu->state_offset);
	}

	free(st->tls_ptr);
}

void iv_tls_user_register(struct iv_tls_user *itu)
{
	if (inited) {
		syslog(LOG_CRIT, "iv_tls_user_register: called after iv_init");
		abort();
	}

	itu->state_offset = last_offset;
	last_offset = (last_offset + itu->sizeof_state + 15) & ~15;

	iv_list_add_tail(&itu->list, &iv_tls_users);
}

void *iv_tls_user_ptr(struct iv_tls_user *itu)
{
	struct iv_state *st = iv_get_state();

	return st->tls_ptr + itu->state_offset;
}
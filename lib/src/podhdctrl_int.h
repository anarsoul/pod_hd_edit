/* SPDX-License-Identifier: LGPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */


#ifndef __PODHDCTRL_INT_H
#define __PODHDCTRL_INT_H

#include <alsa/asoundlib.h>

#include "podhdctrl.h"

#define podhdctrl_debug(...)

/* Arbitrary limit, we've never seen messages that are longer than 8192 bytes */
#define PODHD_MAX_MSG_SIZE	8192

struct podhdctrl_ctx {
	snd_hwdep_t *handle;
	struct list_head recv_cb;
	unsigned char *msg_buf;
	ssize_t msg_size;
};

typedef struct podhdctrl_recv_cb_entry {
	struct list_head list;
	podhdctrl_recv_cb cb;
	void *userdata;
} podhdctrl_recv_cb_entry;

int
podhdctrl_parse_message(podhdctrl_ctx *ctx);

bool
podhdctrl_message_complete(podhdctrl_ctx *ctx);

#endif

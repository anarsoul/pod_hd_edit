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

#define MIN(a,b) ((a) < (b) ? (a) : (b))

struct podhdctrl_ctx {
	snd_hwdep_t *handle;
	unsigned char *msg_buf;
	ssize_t msg_size;
};

podhdctrl_msg*
podhdctrl_parse_message(podhdctrl_ctx *ctx);

bool
podhdctrl_message_complete(podhdctrl_ctx *ctx);

#endif

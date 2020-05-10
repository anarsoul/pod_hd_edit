/* SPDX-License-Identifier: LGPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#ifndef __PODHDCTRL_H
#define __PODHDCTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/types.h>
#include <poll.h>

enum podhdctrl_err {
	PODHDCTRL_NO_ERROR = 0,
	PODHDCTRL_INVALID_ARGUMENT = -1,
	PODHDCTRL_NO_MEMORY = -2,
	PODHDCTRL_FAILED = -3,
	PODHDCTRL_DEVICE_NOT_FOUND = -4,
	PODHDCTRL_ALREADY_EXISTS = -5,
	PODHDCTRL_NOT_FOUND = -6,
};

enum podhdctrl_msg_e {
	PODHDCTRL_MSG_RAW = 0,
	PODHDCTRL_MSG_PRESET_CHANGED,
	PODHDCTRL_MSG_SETLIST_IDX,
	PODHDCTRL_MSG_PRESET_IDX,
	PODHDCTRL_MSG_PEDAL,
	PODHDCTRL_MSG_PARAM_CHANGED,
	PODHDCTRL_MSG_EFFECT_STATE,
	PODHDCTRL_MSG_PRESET_BLOB,
};

typedef struct podhdctrl_msg {
	int type;
} podhdctrl_msg;

typedef struct podhdctrl_raw_msg {
	struct podhdctrl_msg base;
	unsigned char *buf;
	ssize_t size;
} podhdctrl_raw_msg;

typedef podhdctrl_raw_msg podhdctrl_preset_blob_msg;

typedef struct podhdctrl_int_msg {
	struct podhdctrl_msg base;
	int value;
} podhdctrl_int_msg;

typedef struct podhdctrl_float_idx_msg {
	struct podhdctrl_msg base;
	int index;
	float value;
} podhdctrl_float_idx_msg;

typedef struct podhdctrl_effect_state_msg {
	struct podhdctrl_msg base;
	int index;
	bool enabled;
} podhdctrl_effect_state_msg;

struct podhdctrl_ctx;
typedef struct podhdctrl_ctx podhdctrl_ctx;

/* Initializes the library and opens the device */
int
podhdctrl_init(podhdctrl_ctx **ctx, const char *devname);

/* De-initializes the library */
void
podhdctrl_exit(podhdctrl_ctx *ctx);

/* Sends raw message */
int
podhdctrl_send_raw_msg(podhdctrl_ctx *ctx, unsigned char *buf, int len);

/* Receives raw message, resets message buffer */
int
podhdctrl_recv_raw_msg(podhdctrl_ctx *ctx, unsigned char *buf, int buf_len);

/* Receives raw message, doesn't reset message buffer */
int
podhdctrl_peek_raw_msg(podhdctrl_ctx *ctx, unsigned char *buf, int buf_len);

/* Returns parsed message, caller now owns the message, resets message buffer */
int
podhdctrl_recv_msg(podhdctrl_ctx *ctx, podhdctrl_msg **msg);

void
podhdctrl_free_msg(podhdctrl_msg *msg);

/* Reads and processes pending events. Does not block.
 * Should be called in poll loop if FD indicates that there is pending data
 *
 * Returns true when message is ready, caller must call and recv() func in order
 * to clean message buffer since it holds only one message
 */
bool
podhdctrl_handle_events(podhdctrl_ctx *ctx);

/* Returns poll descriptors */
int
podhdctrl_poll_descriptors(podhdctrl_ctx *ctx, struct pollfd *pfds,
			   unsigned int space);

#ifdef __cplusplus
}
#endif

#endif

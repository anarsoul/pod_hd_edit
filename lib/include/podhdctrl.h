/* SPDX-License-Identifier: LGPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#ifndef __PODHDCTRL_H
#define __PODHDCTRL_H

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

typedef void (*podhdctrl_recv_cb)(podhdctrl_msg *msg, void *userdata);

/* Initializes the library and opens the device */
int
podhdctrl_init(podhdctrl_ctx **ctx, const char *devname);

/* De-initializes the library */
void
podhdctrl_exit(podhdctrl_ctx *ctx);

/* Sends raw message */
int
podhdctrl_send_raw_msg(podhdctrl_ctx *ctx, unsigned char *buf, int len);

/* Registers receive callback. Duplicate callback won't be registered */
int
podhdctrl_register_recv_cb(podhdctrl_ctx *ctx, podhdctrl_recv_cb cb, void *userdata);

/* Unregisters receive callback */
int
podhdctrl_unregister_recv_cb(podhdctrl_ctx *ctx, podhdctrl_recv_cb cb, void *userdata);

/* Reads and processes pending events. Does not block. Callbacks will be
 * called if it completes receiving a message.
 * Should be called in poll loop if FD indicates that there is pending data
 */
void
podhdctrl_handle_events(podhdctrl_ctx *ctx);

/* Returns poll descriptors */
int
podhdctrl_poll_descriptors(podhdctrl_ctx *ctx, struct pollfd *pfds,
			   unsigned int space);

#endif

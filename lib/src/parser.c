/* SPDX-License-Identifier: LGPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <endian.h>

#include "list.h"
#include "podhdctrl.h"
#include "podhdctrl_int.h"

static float
podhdctrl_buf_to_float(unsigned char *buf)
{
	union {
		unsigned u;
		float f;
	} u;

	memcpy(&u.u, buf, sizeof(u.u));
	u.u = le32toh(u.u);

	return u.f;
}

static int
podhdctrl_buf_to_int(unsigned char *buf)
{
	union {
		unsigned u;
		int i;
	} u;

	memcpy(&u.u, buf, sizeof(u.u));
	u.u = le32toh(u.u);

	return u.i;
}

bool
podhdctrl_message_complete(podhdctrl_ctx *ctx)
{
	int expected_size = (ctx->msg_buf[0] + (ctx->msg_buf[1] << 8)) * 4 + 4;
	if (expected_size == ctx->msg_size)
		return true;

	return false;
}

static podhdctrl_msg *
podhdctrl_alloc_msg(int type)
{
	podhdctrl_msg *msg;
	switch (type) {
	case PODHDCTRL_MSG_PRESET_CHANGED:
		msg = calloc(1, sizeof(podhdctrl_msg));
		break;

	case PODHDCTRL_MSG_SETLIST_IDX:
	case PODHDCTRL_MSG_PRESET_IDX:
		msg = calloc(1, sizeof(podhdctrl_int_msg));
		break;

	case PODHDCTRL_MSG_PEDAL:
		msg = calloc(1, sizeof(podhdctrl_float_idx_msg));
		break;

	case PODHDCTRL_MSG_PARAM_CHANGED:
		msg = calloc(1, sizeof(podhdctrl_float_idx_msg));
		break;

	case PODHDCTRL_MSG_EFFECT_STATE:
		msg = calloc(1, sizeof(podhdctrl_effect_state_msg));
		break;

	case PODHDCTRL_MSG_PRESET_BLOB:
		msg = calloc(1, sizeof(podhdctrl_preset_blob_msg));
		break;

	case PODHDCTRL_MSG_RAW:
	default:
		msg = calloc(1, sizeof(podhdctrl_raw_msg));
		break;
	}

	if (msg)
		msg->type = type;

	return msg;
}

static void
podhdctrl_raw_to_msg(podhdctrl_ctx *ctx, podhdctrl_msg *msg)
{
	switch (msg->type) {
	case PODHDCTRL_MSG_SETLIST_IDX:
	case PODHDCTRL_MSG_PRESET_IDX:
	{
		podhdctrl_int_msg *imsg = (podhdctrl_int_msg *)msg;
		imsg->value = podhdctrl_buf_to_int(ctx->msg_buf + 8);
		break;
	}

	case PODHDCTRL_MSG_PEDAL:
	{
		podhdctrl_float_idx_msg *fimsg = (podhdctrl_float_idx_msg *)msg;
		fimsg->index = podhdctrl_buf_to_int(ctx->msg_buf + 12);
		fimsg->value = podhdctrl_buf_to_float(ctx->msg_buf + 16);
		break;
	}
	case PODHDCTRL_MSG_PARAM_CHANGED:
	{
		podhdctrl_float_idx_msg *fimsg = (podhdctrl_float_idx_msg *)msg;
		fimsg->index = podhdctrl_buf_to_int(ctx->msg_buf + 16);
		fimsg->value = podhdctrl_buf_to_float(ctx->msg_buf + 20);
		break;
	}
	case PODHDCTRL_MSG_EFFECT_STATE:
	{
		podhdctrl_effect_state_msg *esmsg = (podhdctrl_effect_state_msg *)msg;
		esmsg->index = podhdctrl_buf_to_int(ctx->msg_buf + 12);
		esmsg->enabled = ctx->msg_buf[16] ? true : false;
		break;
	}
	case PODHDCTRL_MSG_PRESET_BLOB:
	{
		podhdctrl_preset_blob_msg *pbmsg = (podhdctrl_preset_blob_msg *)msg;
		pbmsg->buf = ctx->msg_buf;
		pbmsg->size = ctx->msg_size;
		break;
	}
	case PODHDCTRL_MSG_RAW:
	{
		podhdctrl_raw_msg *rmsg = (podhdctrl_raw_msg *)msg;
		rmsg->buf = ctx->msg_buf;
		rmsg->size = ctx->msg_size;
		break;
	}
	default:
		break;
	}
}

static int
podhdctrl_identify_msg_type(podhdctrl_ctx *ctx)
{
	int type = PODHDCTRL_MSG_RAW;

	switch(ctx->msg_buf[7]) {
	case 0x23:
		/* Preset changed */
		if (ctx->msg_size == 8)
			type = PODHDCTRL_MSG_PRESET_CHANGED;
		break;
	case 0x2c:
		/* Setlist IDX */
		if (ctx->msg_size == 12)
			type = PODHDCTRL_MSG_SETLIST_IDX;
		break;
	case 0x27:
		/* Preset IDX */
		if (ctx->msg_size == 12)
			type = PODHDCTRL_MSG_PRESET_IDX;
		break;
	case 0x35:
		/* Pedal */
		if (ctx->msg_size == 20)
			type = PODHDCTRL_MSG_PEDAL;
		break;
	case 0x16:
		/* Float param with IDX */
		if (ctx->msg_size == 24)
			type = PODHDCTRL_MSG_PARAM_CHANGED;
		break;
	case 0x13:
		/* Effect state */
		if (ctx->msg_size == 20)
			type = PODHDCTRL_MSG_EFFECT_STATE;
		break;
	case 0x01:
		/* Preset blob */
		if (ctx->msg_size == 4104)
			type = PODHDCTRL_MSG_PRESET_BLOB;
		break;
	default:
		break;
	}

	return type;
}

podhdctrl_msg *
podhdctrl_parse_message(podhdctrl_ctx *ctx)
{
	int type = PODHDCTRL_MSG_RAW;
	podhdctrl_msg *msg;

	if (ctx->msg_buf[5] == 0x40)
		type = podhdctrl_identify_msg_type(ctx);

	msg = podhdctrl_alloc_msg(type);
	if (!msg)
		return NULL;

	podhdctrl_raw_to_msg(ctx, msg);

	return msg;
}

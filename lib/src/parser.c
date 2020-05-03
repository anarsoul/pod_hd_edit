/* SPDX-License-Identifier: LGPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include "list.h"
#include "podhdctrl.h"
#include "podhdctrl_int.h"

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
		memcpy(&imsg->value, ctx->msg_buf + 8, sizeof(imsg->value));
		break;
	}

	case PODHDCTRL_MSG_PEDAL:
	{
		podhdctrl_float_idx_msg *fimsg = (podhdctrl_float_idx_msg *)msg;
		memcpy(&fimsg->index, ctx->msg_buf + 12, sizeof(fimsg->index));
		memcpy(&fimsg->value, ctx->msg_buf + 16, sizeof(fimsg->value));
		break;
	}
	case PODHDCTRL_MSG_PARAM_CHANGED:
	{
		podhdctrl_float_idx_msg *fimsg = (podhdctrl_float_idx_msg *)msg;
		memcpy(&fimsg->index, ctx->msg_buf + 16, sizeof(fimsg->index));
		memcpy(&fimsg->value, ctx->msg_buf + 20, sizeof(fimsg->value));
		break;
	}
	case PODHDCTRL_MSG_EFFECT_STATE:
	{
		podhdctrl_effect_state_msg *esmsg = (podhdctrl_effect_state_msg *)msg;
		memcpy(&esmsg->index, ctx->msg_buf + 12, sizeof(esmsg->index));
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

static void print_buf(unsigned char *buf, ssize_t start, ssize_t size)
{
	for (int i = 0; i < size; i += 16) {
		printf("%.4lx:", i + start);
		for (int j = i; (j < (i + 16)) && (j < size); j++)
			printf(" %.2x", buf[j]);
		printf("\n");
	}
}

int
podhdctrl_parse_message(podhdctrl_ctx *ctx)
{
	int type = PODHDCTRL_MSG_RAW;
	podhdctrl_msg *msg;
	podhdctrl_recv_cb_entry *entry;

	if (ctx->msg_buf[5] == 0x40)
		type = podhdctrl_identify_msg_type(ctx);

	if (ctx->msg_size < 32)
		print_buf(ctx->msg_buf, 0, ctx->msg_size);

	msg = podhdctrl_alloc_msg(type);
	if (!msg)
		return PODHDCTRL_NO_MEMORY;

	podhdctrl_raw_to_msg(ctx, msg);

	list_for_each_entry(entry, &ctx->recv_cb, list, podhdctrl_recv_cb_entry) {
		entry->cb(msg, entry->userdata);
	}

	free(msg);

	return PODHDCTRL_NO_ERROR;
}

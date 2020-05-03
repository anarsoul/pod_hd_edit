/* SPDX-License-Identifier: LGPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <alsa/asoundlib.h>

#include <stdlib.h>
#include <poll.h>

#include "list.h"
#include "podhdctrl.h"
#include "podhdctrl_int.h"

static int
podhdctrl_open_device(podhdctrl_ctx *ctx, const char *devname)
{
	int err;
	snd_hwdep_info_t *info;

	err = snd_hwdep_open(&ctx->handle, devname,
			     SND_HWDEP_OPEN_DUPLEX | SND_HWDEP_OPEN_NONBLOCK);

	if (err < 0)
		goto out_err;

	snd_hwdep_info_alloca(&info);
	if (!snd_hwdep_info(ctx->handle, info)) {
		int iface = snd_hwdep_info_get_iface(info);
		if (iface == SND_HWDEP_IFACE_LINE6)
			return PODHDCTRL_NO_ERROR;
	}

	snd_hwdep_close(ctx->handle);

out_err:
	ctx->handle = NULL;
	return PODHDCTRL_DEVICE_NOT_FOUND;
}

static int
podhdctrl_probe_device(podhdctrl_ctx *ctx)
{
	char tmpname[16];
	int card = -1;
	snd_ctl_t *ctl;

	while (!snd_card_next(&card) && card >= 0) {
		int dev;
		sprintf(tmpname, "hw:%d", card);
		if (snd_ctl_open(&ctl, tmpname, 0) < 0)
			continue;

		dev = -1;
		while (!snd_ctl_hwdep_next_device(ctl, &dev)
		       && dev >= 0) {
			sprintf(tmpname, "hw:%d,%d", card, dev);
			if (!podhdctrl_open_device(ctx, tmpname)) {
				snd_ctl_close(ctl);
				return PODHDCTRL_NO_ERROR;
			}
		}
		snd_ctl_close(ctl);
	}

	ctx->handle = NULL;
	return PODHDCTRL_DEVICE_NOT_FOUND;
}

static int
podhdctrl_init_device(podhdctrl_ctx *ctx, const char *devname)
{
	if (!devname)
		return podhdctrl_probe_device(ctx);

	return podhdctrl_open_device(ctx, devname);
}

static void
podhdctrl_close_device(podhdctrl_ctx *ctx)
{
	snd_hwdep_close(ctx->handle);
	ctx->handle = NULL;
}

int
podhdctrl_init(podhdctrl_ctx **ctx, const char *devname)
{
	int ret;

	if (!ctx)
		return PODHDCTRL_INVALID_ARGUMENT;

	*ctx = calloc(1, sizeof(podhdctrl_ctx));
	if (!*ctx) {
		ret = PODHDCTRL_NO_MEMORY;
		goto out;
	}

	list_init(&(*ctx)->recv_cb);

	(*ctx)->msg_buf = malloc(PODHD_MAX_MSG_SIZE);
	if (!(*ctx)->msg_buf) {
		ret = PODHDCTRL_NO_MEMORY;
		goto free_ctx;
	}

	ret = podhdctrl_init_device(*ctx, devname);
	if (!ret)
		return ret;

	free((*ctx)->msg_buf);
free_ctx:
	free(*ctx);
	*ctx = NULL;

out:
	return ret;
}

void
podhdctrl_exit(podhdctrl_ctx *ctx)
{
	podhdctrl_recv_cb_entry *it, *tmp;
	podhdctrl_close_device(ctx);

	list_for_each_entry_safe(it, tmp, &ctx->recv_cb, list, podhdctrl_recv_cb_entry) {
		list_del(&it->list);
		free(it);
	}

	free(ctx->msg_buf);
	free(ctx);
}

int
podhdctrl_send_raw_msg(podhdctrl_ctx *ctx, unsigned char *buf, int len)
{
	ssize_t ret;

	ret = snd_hwdep_write(ctx->handle, buf, len);
	if (ret < 0)
		return PODHDCTRL_FAILED;

	return ret;
}

int
podhdctrl_register_recv_cb(podhdctrl_ctx *ctx, podhdctrl_recv_cb cb, void *userdata)
{
	podhdctrl_recv_cb_entry *entry;

	/* Do not add duplicate entries */
	list_for_each_entry(entry, &ctx->recv_cb, list, podhdctrl_recv_cb_entry) {
		if (entry->cb == cb && entry->userdata == userdata)
			return PODHDCTRL_ALREADY_EXISTS;
	}

	entry = calloc(1, sizeof(*entry));
	if (!entry)
		return PODHDCTRL_NO_MEMORY;

	list_init(&entry->list);
	entry->cb = cb;
	entry->userdata = userdata;
	list_add_tail(&ctx->recv_cb, &entry->list);

	return PODHDCTRL_NO_ERROR;
}

int
podhdctrl_unregister_recv_cb(podhdctrl_ctx *ctx, podhdctrl_recv_cb cb, void *userdata)
{
	int ret = PODHDCTRL_NOT_FOUND;
	podhdctrl_recv_cb_entry *entry, *tmp;

	list_for_each_entry_safe(entry, tmp, &ctx->recv_cb, list, podhdctrl_recv_cb_entry) {
		if (entry->cb == cb && entry->userdata == userdata) {
			list_del(&entry->list);
			free(entry);
			ret = PODHDCTRL_NO_ERROR;
			break;
		}
	}

	return ret;
}

int
podhdctrl_poll_descriptors(podhdctrl_ctx *ctx, struct pollfd *pfds,
			   unsigned int space)
{
	return snd_hwdep_poll_descriptors(ctx->handle, pfds, space);
}

static void
podhdctrl_reset_message(podhdctrl_ctx *ctx)
{
	ctx->msg_size = 0;
}

void
podhdctrl_handle_events(podhdctrl_ctx *ctx)
{
	unsigned char tmp_buf[512];
	ssize_t ret;

	do {
		ret = snd_hwdep_read(ctx->handle, tmp_buf, sizeof(tmp_buf));
		if (ret > 0) {
			int data_size = tmp_buf[0] + (tmp_buf[1] << 8);
			if (data_size != ret - 4) {
				podhdctrl_debug("Received bogus packet of size: %d, data_size: %d\n",
						ret, data_size);
				podhdctrl_reset_message(ctx);
				continue;
			}

			if (tmp_buf[2] == 1 && ctx->msg_size != 0) {
				podhdctrl_debug("Dropping incomplete message\n");
				podhdctrl_reset_message(ctx);
			}

			if (tmp_buf[2] == 4 && ctx->msg_size == 0) {
				podhdctrl_debug("Lost head of the message, dropping the rest\n");
				continue;
			}

			if ((ctx->msg_size + data_size) > PODHD_MAX_MSG_SIZE) {
				podhdctrl_debug("Message buffer overflow!\n");
				podhdctrl_reset_message(ctx);
				continue;
			}

			memcpy(ctx->msg_buf + ctx->msg_size, tmp_buf + 4, data_size);
			ctx->msg_size += data_size;

			if (podhdctrl_message_complete(ctx)) {
				podhdctrl_parse_message(ctx);
				podhdctrl_reset_message(ctx);
			}
		}
	} while (ret >= 0);
}

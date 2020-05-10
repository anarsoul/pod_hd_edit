/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "podhdctrl.h"

static bool interrupted = false;

static void
sighandler(int signo)
{
	if (signo == SIGINT)
		interrupted  = true;
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

static void
handle_message(podhdctrl_msg *msg)
{
	switch (msg->type) {
	case PODHDCTRL_MSG_PRESET_CHANGED:
		printf("Preset changed\n");
		break;
	case PODHDCTRL_MSG_SETLIST_IDX:
	{
		podhdctrl_int_msg *imsg = (podhdctrl_int_msg *)msg;
		printf("Setlist: %d\n", imsg->value);
		break;
	}
	case PODHDCTRL_MSG_PRESET_IDX:
	{
		podhdctrl_int_msg *imsg = (podhdctrl_int_msg *)msg;
		printf("Preset: %d\n", imsg->value);
		break;
	}
	case PODHDCTRL_MSG_PEDAL:
	{
		podhdctrl_float_idx_msg *fimsg = (podhdctrl_float_idx_msg *)msg;
		printf("Pedal #%d: %f\n", fimsg->index, (double)fimsg->value);
		break;
	}
	case PODHDCTRL_MSG_PARAM_CHANGED:
	{
		podhdctrl_float_idx_msg *fimsg = (podhdctrl_float_idx_msg *)msg;
		printf("Param: %d is %f\n", fimsg->index, (double)fimsg->value);
		break;
	}
	case PODHDCTRL_MSG_EFFECT_STATE:
	{
		podhdctrl_effect_state_msg *fsmsg = (podhdctrl_effect_state_msg *)msg;
		printf("Effect: %d is %s\n", fsmsg->index, fsmsg->enabled ? "on" : "off");
		break;
	}
	case PODHDCTRL_MSG_RAW:
	{
		podhdctrl_raw_msg *rmsg = (podhdctrl_raw_msg *)msg;
		printf("Received raw msg of size %ld\n", rmsg->size);
		break;
	}
	case PODHDCTRL_MSG_PRESET_BLOB:
	{
		char name[17];
		podhdctrl_preset_blob_msg *pbmsg = (podhdctrl_preset_blob_msg *)msg;
		memcpy(name, pbmsg->buf + 8, 16);
		name[16] = '\0';
		printf("Received preset '%s' of size %ld\n", name, pbmsg->size);
		break;
	}
	default:
		break;
	}
}

int
main(int argc, char *argv[])
{
	int ret;
	const char *devname = NULL;
	podhdctrl_ctx *ctx;
	struct pollfd pfd;

	if (argc > 2) {
		fprintf(stderr, "Usage: %s [devname]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (argc == 2)
		devname = argv[1];

	signal(SIGINT, sighandler);

	ret = podhdctrl_init(&ctx, devname);
	if (ret) {
		printf("Init failed: %d\n", ret);
		goto out;
	}

	podhdctrl_poll_descriptors(ctx, &pfd, 1);

	while (!interrupted) {
		bool ready;
		int pollrc = poll(&pfd, 1, 1000);
		unsigned char buf[8192];
		int ret;
		podhdctrl_msg *msg;
		if (pollrc < 0) {
			perror("poll");
			break;
		}
		if (pfd.revents & (POLLIN | POLLRDNORM)) {
			ready = podhdctrl_handle_events(ctx);
			if (!ready)
				continue;

			/* Peek raw message, dump it if it's not too big */
			ret = podhdctrl_peek_raw_msg(ctx, buf, sizeof(buf));
			if (ret < 0) {
				printf("peek_raw_msg failed: %d\n", ret);
				break;
			}
			if (ret < 32)
				print_buf(buf, 0, ret);

			/* Receive and parse */
			ret = podhdctrl_recv_msg(ctx, &msg);
			if (ret) {
				printf("recv_msg failed: %d\n", ret);
				break;
			}
			handle_message(msg);
			podhdctrl_free_msg(msg);
		}
		if (pfd.revents & (POLLERR | POLLHUP))
			break;
	}

	podhdctrl_exit(ctx);

out:
	return ret;
}

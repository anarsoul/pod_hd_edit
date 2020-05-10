/* SPDX-License-Identifier: LGPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#ifndef __PODHD_PRESET
#define __PODHD_PRESET

#ifdef __cplusplus
extern "C" {
#endif

enum pod_hd500_effect_pos {
	POS_PRE_AMP = 0,
	POS_PRE_AMP_1ST_ROW,
	POS_PRE_AMP_2ND_ROW,
	POS_POST_AMP_1ST_ROW,
	POS_POST_AMP_2ND_ROW,
	POS_POST_AMP,
};

enum pod_hd500_effect_type {
	FX_TYPE_DYN = 0,
	FX_TYPE_DIST,
	FX_TYPE_MOD,
	FX_TYPE_FILTER,
	FX_TYPE_PITCH,
	FX_TYPE_PRE_EQ,
	FX_TYPE_DELAY,
	FX_TYPE_REVERB,
	FX_TYPE_VOLPAN,
	FX_TYPE_WAH,
	FX_TYPE_LOOP,
};

typedef struct pod_hd500_effect {
	bool enabled;
	int type;
	int model;
	/* Pre-amp or Post-amp, 1st or 2nd row */
	int pos;
	/* Position within row */
	int row_pos;
} podhd500_effect;

typedef struct pod_hd500_amp {
	bool enabled;
	int amp_model;
	int cab_model;
} podhd500_amp;

#define POD_HD500_PRESET_MAX_AMPS 2
#define POD_HD500_PRESET_MAX_FXS 8

typedef struct pod_hd500_preset {
	char name[21]; /* 20 syms + null-terminator */
	podhd500_amp amps[POD_HD500_PRESET_MAX_AMPS];
	podhd500_effect effects[POD_HD500_PRESET_MAX_FXS];
} pod_hd500_preset;

#ifdef __cplusplus
}
#endif

#endif

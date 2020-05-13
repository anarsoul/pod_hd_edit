/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#pragma once

#include "rectTextItem.h"

class FXItem : public RectTextItem {

public:
	FXItem(const QString &caption);
	virtual ~FXItem();

private:
	QString _caption;
};


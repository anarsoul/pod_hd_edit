/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <QtWidgets/QWidget>
#include "ui_amp.h"

class AmpWidget : public QWidget, Ui::AmpWidget
{
	Q_OBJECT;

public:
	AmpWidget();
	~AmpWidget();
};

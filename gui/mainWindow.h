/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QGraphicsView>
#include "ui_mainWindow.h"

#include "podhd_preset.h"

class MainWindow : public QMainWindow, Ui::MainWindow
{
	Q_OBJECT;

public:
	MainWindow();
	~MainWindow();

	void updateFXChain(pod_hd500_preset *preset = nullptr);

private:
	pod_hd500_preset current_preset;
	QGraphicsScene *scene;
	void clearFXChain();
};

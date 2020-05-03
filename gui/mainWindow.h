/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <QtWidgets/QMainWindow>
#include "ui_mainWindow.h"

class MainWindow : public QMainWindow, Ui::MainWindow
{
	Q_OBJECT;

public:
	MainWindow();
	~MainWindow();
};

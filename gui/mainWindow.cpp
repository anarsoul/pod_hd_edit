/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include "mainWindow.h"

MainWindow::MainWindow()
{
	setupUi(this);
	statusBar()->showMessage(tr("Disconnected"));
}

MainWindow::~MainWindow()
{
}

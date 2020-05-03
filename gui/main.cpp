/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <QtWidgets/QApplication>
#include "mainWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow win;
	win.show();

	return app.exec();

}

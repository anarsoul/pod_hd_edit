/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <QPair>
#include <QList>

#include <algorithm>

#include <string.h>

#include "mainWindow.h"
#include "ampItem.h"
#include "fxItem.h"
#include "mixItem.h"

MainWindow::MainWindow() : scene(nullptr)
{
	setupUi(this);
	statusBar()->showMessage(tr("Disconnected"));

	clearFXChain();
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateFXChain(pod_hd500_preset *new_preset)
{
	if (new_preset != nullptr)
		memcpy(&current_preset, new_preset, sizeof(current_preset));

	QGraphicsScene *newScene = new QGraphicsScene();
	QGraphicsItem *item;
	RectTextItem *rtitem;

	qreal x = 0;
	QList<QPair<int, pod_hd500_effect *> > pre_list;
	QList<QPair<int, pod_hd500_effect *> > post_list;
	for (int i = 0; i < POD_HD500_PRESET_MAX_FXS; i++) {
		if (current_preset.effects[i].pos == POS_PRE_AMP)
			pre_list << QPair<int, pod_hd500_effect *>(current_preset.effects[i].row_pos, &current_preset.effects[i]);
		if (current_preset.effects[i].pos == POS_POST_AMP)
			post_list << QPair<int, pod_hd500_effect *>(current_preset.effects[i].row_pos, &current_preset.effects[i]);
	}

	std::sort(pre_list.begin(), pre_list.end(), [](const QPair<int, pod_hd500_effect *>&a,
						       const QPair<int, pod_hd500_effect *>&b) -> bool
						       { return a.first < b.first; });

	std::sort(post_list.begin(), post_list.end(), [](const QPair<int, pod_hd500_effect *>&a,
							 const QPair<int, pod_hd500_effect *>&b) -> bool
							 { return a.first < b.first; });


	QPair<int, pod_hd500_effect *> list_item;

	foreach(list_item, pre_list) {
		item = newScene->addLine(x - 40, 0, x - 24, 0, QPen(QApplication::palette().text(), 2.0));
		item->setZValue(1.0);
		rtitem = new FXItem(QString("FX ") + QString::number(list_item.second->index + 1));
		rtitem->setFXEnabled(list_item.second->enabled);
		rtitem->setPos(x, 0);
		newScene->addItem(rtitem);
		x += 64;
	}

	item = newScene->addLine(x - 40, 0, x - 24, 0, QPen(QApplication::palette().text(), 2.0));
	item->setZValue(1.0);
	rtitem = new AmpItem();
	rtitem->setPos(x, 0);
	rtitem->setFXEnabled(current_preset.amps[0].enabled);
	newScene->addItem(rtitem);
	x += 64;

	item = newScene->addLine(x - 40, 0 - 16, x - 24, 0 - 16, QPen(QApplication::palette().text(), 2.0));
	item->setZValue(1.0);
	item = newScene->addLine(x - 40, 0 + 16, x - 24, 0 + 16, QPen(QApplication::palette().text(), 2.0));
	item->setZValue(1.0);
	rtitem = new MixItem();
	rtitem->setPos(x, 0);
	/* Mixer is always enabled */
	rtitem->setFXEnabled(true);
	newScene->addItem(rtitem);
	x += 64;

	foreach(list_item, post_list) {
		item = newScene->addLine(x - 40, 0, x - 24, 0, QPen(QApplication::palette().text(), 2.0));
		item->setZValue(1.0);
		rtitem = new FXItem(QString("FX ") + QString::number(list_item.second->index + 1));
		rtitem->setFXEnabled(list_item.second->enabled);
		rtitem->setPos(x, 0);
		newScene->addItem(rtitem);
		x += 64;
	}

	item = newScene->addLine(x - 40, 0, x - 24, 0, QPen(QApplication::palette().text(), 2.0));
	item->setZValue(1.0);

	QRectF bounding = newScene->itemsBoundingRect();
	item = newScene->addText(QString(current_preset.name));
	item->setPos(bounding.x() + bounding.width() / 2.0 - item->boundingRect().width() / 2.0, bounding.y() - item->boundingRect().height());

	if (this->scene)
		delete this->scene;

	this->scene = newScene;

	this->graphicsView->setScene(this->scene);
}

void MainWindow::clearFXChain()
{
	memset(&current_preset, 0, sizeof(current_preset));
	strncpy(current_preset.name, tr("New Preset").toUtf8().constData(), sizeof(current_preset.name));
	current_preset.amps[0].enabled = true;
	current_preset.amps[0].index = 0;
	current_preset.amps[1].index = 1;
	current_preset.effects[0].enabled = true;
	current_preset.effects[0].type = FX_TYPE_DIST;

	/* 1 row, half of the FXs go pre-amp, other half post-amp */
	for (int i = 0; i < POD_HD500_PRESET_MAX_FXS; i++) {
		current_preset.effects[i].index = i;
		if (i < (POD_HD500_PRESET_MAX_FXS / 2)) {
			current_preset.effects[i].pos = POS_PRE_AMP;
			current_preset.effects[i].row_pos = i;
		} else {
			current_preset.effects[i].pos = POS_POST_AMP;
			current_preset.effects[i].row_pos = i - (POD_HD500_PRESET_MAX_FXS / 2);
		}
	}

	updateFXChain();
}

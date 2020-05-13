/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <QApplication>
#include <QCursor>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>

#include "ampItem.h"

RectTextItem::RectTextItem(const QString &_text) : text(_text),
						   _fxEnabled(false)
{
	setCursor(Qt::OpenHandCursor);
	setAcceptedMouseButtons(Qt::LeftButton);
	setFlags(QGraphicsItem::ItemIsSelectable);
}

RectTextItem::~RectTextItem()
{
}


QRectF RectTextItem::boundingRect() const
{
	return QRectF(-32, -32, 64, 64);
}

void RectTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QRectF rect(-24, -24, 48, 48);
	Q_UNUSED(widget);

	if (option->state & QStyle::State_Selected)
		painter->fillRect(boundingRect(), QApplication::palette().highlight());

	if (!_fxEnabled) {
		painter->setPen(QPen(QApplication::palette().text(), 2.0));
		painter->drawRect(rect);
	} else {
		painter->fillRect(rect, QApplication::palette().text());
		painter->setPen(QPen(QApplication::palette().window(), 1.0));
	}
	painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

}

void RectTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	/* Disable multiple selection */
	if (event->modifiers() & Qt::ControlModifier)
		event->ignore();
	else {
		setCursor(Qt::ClosedHandCursor);
		QGraphicsItem::mousePressEvent(event);
	}
}

void RectTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
}

void RectTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	setCursor(Qt::OpenHandCursor);
	QGraphicsItem::mouseReleaseEvent(event);
}

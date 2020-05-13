/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) 2020 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#pragma once

#include <QtWidgets/QGraphicsItem>

class RectTextItem : public QGraphicsItem {

public:
	RectTextItem(const QString &text);
	virtual ~RectTextItem();
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	void setFXEnabled(bool enabled) { _fxEnabled = enabled; update(); };
	bool fxEnabled() { return _fxEnabled; }
	void setText(QString &_text) { text = _text; update(); };

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
	QString text;
	bool _fxEnabled;

};

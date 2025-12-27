#pragma once

#include "MainWindow.h"

class ViewOverlayFrame : public QFrame {
	Q_OBJECT
public:
	ViewOverlayFrame(QFrame* oldFrame, MainWindow* win);

	QWidget* targetWidget = nullptr;
protected:
	const float zoomSpeed = 0.001f;

	MainWindow* win;
	bool mouseOn = false;
	QPoint lastMousePos;

	void enterEvent(QEvent* e) override;
	void leaveEvent(QEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
};

class VViewOverlayFrame : public ViewOverlayFrame {
	Q_OBJECT
public:
	VViewOverlayFrame(QFrame* oldFrame, MainWindow* win);
protected:
	void paintEvent(QPaintEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
};

class HViewOverlayFrame : public ViewOverlayFrame {
	Q_OBJECT
public:
	HViewOverlayFrame(QFrame* oldFrame, MainWindow* win);
protected:
	void paintEvent(QPaintEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;
};
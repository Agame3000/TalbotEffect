#include "ViewOverlayFrame.h"

static float clamp(float min, float x, float max) {
	return x < min ? min : x > max ? max : x;
}

ViewOverlayFrame::ViewOverlayFrame(QFrame* oldFrame, MainWindow* win) : QFrame(oldFrame->parentWidget()),
win(win)
{
	setGeometry(oldFrame->geometry());
	QHBoxLayout* parentLayout = qobject_cast<QHBoxLayout*>(oldFrame->parentWidget()->layout()),
		* curLayout = new QHBoxLayout(this);
	curLayout->setContentsMargins(30, 30, 30, 30);
	setLayout(curLayout);
	setFrameShape(oldFrame->frameShape());
	setFrameShadow(oldFrame->frameShadow());

	if (parentLayout) {
		int index = -1;
		QLayoutItem* item = nullptr;
		for (int i = 0; i < parentLayout->count(); ++i) {
			if (parentLayout->itemAt(i)->widget() == oldFrame) {
				index = i;
				break;
			}
		}
		if (index != -1) {
			parentLayout->removeWidget(oldFrame);
			delete oldFrame;
			parentLayout->insertWidget(index, this);
		}
	}

	setMouseTracking(true);
}

void ViewOverlayFrame::enterEvent(QEvent* e) {
	mouseOn = true;
}

void ViewOverlayFrame::leaveEvent(QEvent* e) {
	mouseOn = false;
	update();
}

void ViewOverlayFrame::mouseMoveEvent(QMouseEvent* e) {
	lastMousePos = e->pos();
	update();
}


VViewOverlayFrame::VViewOverlayFrame(QFrame* oldFrame, MainWindow* win) : ViewOverlayFrame(oldFrame, win) {
	connect(win->viewSizeX, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
	connect(win->viewSizeY, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
	connect(win->z, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
	connect(win->depZt, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
	connect(win->offsetX, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
	connect(win->offsetY, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
	connect(win->period, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
}

void VViewOverlayFrame::paintEvent(QPaintEvent* e) {
	ViewOverlayFrame::paintEvent(e);
	QPainter painter(this);

	painter.setPen(QPen(Qt::GlobalColor::blue, 4));
	painter.drawLine(5, height() - 9, 5, 7);
	painter.drawLine(6, 7, 16, 17);
	painter.setPen(Qt::GlobalColor::blue);
	QPoint xApex[3]{
		QPoint(3, 2),
		QPoint(3, 4),
		QPoint(5, 4)
	};
	painter.drawPolygon(xApex, 3);

	painter.setPen(QPen(Qt::GlobalColor::red, 4));
	painter.drawLine(9, height() - 5, width() - 7, height() - 5);
	painter.drawLine(width() - 7, height() - 6, width() - 17, height() - 16);
	painter.setPen(Qt::GlobalColor::red);
	QPoint zApex[3]{
		QPoint(width() - 3, height() - 4),
		QPoint(width() - 5, height() - 4),
		QPoint(width() - 5, height() - 6)
	};
	painter.drawPolygon(zApex, 3);

	painter.setPen(QPen(Qt::GlobalColor::black, 4));
	painter.drawLine(5, height() - 5, 5, height() - 5);

	painter.setPen(QPen(Qt::GlobalColor::magenta, 3));
	float w = *win->viewSizeX;
	int x = 31 + (float)*win->z / w * (width() - 63);
	painter.drawLine(x, 23, x, height() - 25);

	painter.setPen(QPen(Qt::GlobalColor::darkRed, 3));
	float zt = *win->depZt, oz = *win->offsetX;
	for (float cz = (floorf(oz / zt) + 1) * zt - oz; cz <= w; cz += zt) {
		float x = 31 + cz / w * (width() - 63);
		painter.drawLine(x, 6, x, 20);
		painter.drawLine(x, height() - 18, x, height() - 22);
	}

	painter.setPen(QPen(Qt::GlobalColor::darkBlue, 3));
	float h = *win->viewSizeY, p = *win->period, ox = (float)*win->offsetY;
	for (float cx = ceilf(ox / p) * p - ox; cx <= h; cx += p) {
		float y = height() - (32 + cx / h * (height() - 63));
		painter.drawLine(17, y, 30, y);
		painter.drawLine(width() - 7, y, width() - 30, y);
	}

	QFont font = painter.font();
	if (mouseOn) {
		float x = clamp(31, lastMousePos.x(), width() - 32),
			y = clamp(31, lastMousePos.y(), height() - 32);
		painter.setPen(QPen(Qt::GlobalColor::blue, 3));
		painter.drawLine(7, y, 14, y);
		painter.setPen(QPen(Qt::GlobalColor::red, 3));
		painter.drawLine(x, height() - 8, x, height() - 15);
		painter.setPen(Qt::GlobalColor::darkMagenta);
		x = (x - 31) / (width() - 63) * w;
		y = (height() - y - 32) / (height() - 63) * h;
		font.setPixelSize(12);
		painter.setFont(font);
		QString text("(%1; %2)");
		text = text.arg(x).arg(y);
		painter.drawText(width() - 60 - QFontMetrics(font).horizontalAdvance(text), 20, text);
	}

	font.setPixelSize(18);
	font.setBold(true);
	painter.setFont(font);
	painter.setPen(Qt::GlobalColor::blue);
	painter.drawText(23, 20, "X");

	painter.setPen(Qt::GlobalColor::red);
	painter.drawText(width() - 20, height() - 23, "Z");

	painter.setPen(Qt::GlobalColor::black);
	font.setPixelSize(10);
	font.setBold(false);
	painter.setFont(font);
	painter.drawText(12, height() - 12,
		QString("(%1; %2)")
		.arg((float)*win->offsetX)
		.arg((float)*win->offsetY)
	);
}

void VViewOverlayFrame::wheelEvent(QWheelEvent* e) {
	QPoint angleDelta = e->angleDelta();
	QPointF cur(
		(e->position().x() - 30) / (width() - 60),
		(e->position().y() - 30) / (height() - 60)
	);

	float vx = *win->viewSizeX, vy = *win->viewSizeY;

	*win->offsetX = clamp(0,
		(float)*win->offsetX + vx * (float)cur.x() * zoomSpeed * angleDelta.y(),
		*win->depSizeX);
	*win->offsetY = clamp(0,
		(float)*win->offsetY + vy * (1 - (float)cur.y()) * zoomSpeed * angleDelta.y(),
		*win->depSizeY);
	*win->viewSizeX = clamp(0,
		vx - vx * zoomSpeed * angleDelta.y(),
		*win->depSizeX);
	*win->viewSizeY = clamp(0,
		vy - vy * zoomSpeed * angleDelta.y(),
		*win->depSizeY);
}

void VViewOverlayFrame::mouseMoveEvent(QMouseEvent* e) {
	if (e->buttons().testFlag(Qt::MouseButton::LeftButton)) {
		QPointF delta = e->pos() - lastMousePos;
		QPointF cur(
			delta.x() / (width() - 60),
			delta.y() / (height() - 60)
		);

		*win->offsetX = clamp(0,
			(float)*win->offsetX + -cur.x() * (float)*win->viewSizeX,
			*win->depSizeX);
		*win->offsetY = clamp(0,
			(float)*win->offsetY + cur.y() * (float)*win->viewSizeY,
			*win->depSizeY);
	}

	ViewOverlayFrame::mouseMoveEvent(e);
}


HViewOverlayFrame::HViewOverlayFrame(QFrame* oldFrame, MainWindow* win) : ViewOverlayFrame(oldFrame, win) {
	connect(win->viewSizeY, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
	connect(win->offsetY, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
	connect(win->period, &DependedVariantContainer::update,
		this, QOverload<>::of(&VViewOverlayFrame::update));
}

void HViewOverlayFrame::paintEvent(QPaintEvent* e) {
	ViewOverlayFrame::paintEvent(e);
	QPainter painter(this);

	QColor green(0, 200, 0);

	painter.setPen(QPen(green, 4));
	painter.drawLine(5, height() - 9, 5, 7);
	painter.drawLine(6, 7, 16, 17);
	painter.setPen(green);
	QPoint yApex[3]{
		QPoint(3, 2),
		QPoint(3, 4),
		QPoint(5, 4)
	};
	painter.drawPolygon(yApex, 3);

	painter.setPen(QPen(Qt::GlobalColor::blue, 4));
	painter.drawLine(9, height() - 5, width() - 7, height() - 5);
	painter.drawLine(width() - 7, height() - 6, width() - 17, height() - 16);
	painter.setPen(Qt::GlobalColor::blue);
	QPoint xApex[3]{
		QPoint(width() - 3, height() - 4),
		QPoint(width() - 5, height() - 4),
		QPoint(width() - 5, height() - 6)
	};
	painter.drawPolygon(xApex, 3);

	painter.setPen(QPen(Qt::GlobalColor::black, 4));
	painter.drawLine(5, height() - 5, 5, height() - 5);

	painter.setPen(QPen(Qt::GlobalColor::darkBlue, 3));
	float h = *win->viewSizeY, p = *win->period, ox = (float)*win->offsetY;
	for (float cx = ceilf(ox / p) * p - ox; cx <= h; cx += p) {
		float x = 31 + cx / h * (width() - 63);
		painter.drawLine(x, 6, x, 29);
		painter.drawLine(x, height() - 18, x, height() - 30);
	}

	QFont font = painter.font();
	if (mouseOn) {
		float x = clamp(31, lastMousePos.x(), width() - 32);
		painter.setPen(QPen(Qt::GlobalColor::blue, 3));
		painter.drawLine(x, height() - 8, x, height() - 15);
		painter.setPen(Qt::GlobalColor::darkMagenta);
		x = (x - 31) / (width() - 63) * h;
		font.setPixelSize(12);
		painter.setFont(font);
		QString text("x: %1");
		text = text.arg(x);
		painter.drawText(width() - 60 - QFontMetrics(font).horizontalAdvance(text), 20, text);
	}

	font.setPixelSize(18);
	font.setBold(true);
	painter.setFont(font);
	painter.setPen(green);
	painter.drawText(23, 20, "Y");

	painter.setPen(Qt::GlobalColor::blue);
	painter.drawText(width() - 20, height() - 23, "X");

	painter.setPen(Qt::GlobalColor::black);
	font.setPixelSize(10);
	font.setBold(false);
	painter.setFont(font);
	painter.drawText(12, height() - 12,
		QString("x: %1")
		.arg((float)*win->offsetY)
	);
}

void HViewOverlayFrame::wheelEvent(QWheelEvent* e) {
	QPoint angleDelta = e->angleDelta();
	float x = (e->position().x() - 30) / (width() - 60), vx = *win->viewSizeY;

	*win->offsetY = clamp(0,
		(float)*win->offsetY + vx * x * zoomSpeed * angleDelta.y(),
		*win->depSizeY);
	*win->viewSizeY = clamp(0,
		vx - vx * zoomSpeed * angleDelta.y(),
		*win->depSizeY);
}
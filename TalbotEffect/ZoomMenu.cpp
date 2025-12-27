#include "ZoomMenu.h"

#include "MainWindow.h"

ZoomMenu::ZoomMenu(MainWindow* parent) : QWidget(parent) {
	ui.setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);

	{
		connect(parent->depSizeX, &DependedVariantContainer::update, [&, parent]() {
			float m = *parent->depSizeX;

			QSignalBlocker blocker1(ui.zsDBox);
			QSignalBlocker blocker2(parent->viewSizeX);
			ui.zsDBox->setMaximum(m);
			float cur = ui.zsSlider->value() * m / 1000.0;
			ui.zsDBox->setValue(cur);
			*parent->viewSizeX = cur;
			}
		);

		connect(ui.zsDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&, parent](double v) {
			*parent->viewSizeX = (float)v;
			}
		);

		connect(ui.zsSlider, &QSlider::valueChanged, [&, parent](int v) {
			*parent->viewSizeX = v * (float)*parent->depSizeX / 1000.0f;
			QSignalBlocker blocker(ui.zsSlider);
			ui.zsSlider->setValue(v);
			}
		);

		connect(ui.zzDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&, parent](double v) {
			*parent->viewSizeX = (float)*parent->depSizeX / (float)v;
			}
		);

		connect(parent->viewSizeX, &DependedVariantContainer::update, [&, parent]() {
			QSignalBlocker blocker1(ui.zsDBox);
			QSignalBlocker blocker2(ui.zsSlider);
			QSignalBlocker blocker3(ui.zzDBox);
			float v = *parent->viewSizeX, m = *parent->depSizeX;

			ui.zsDBox->setValue(v);
			if (v != 0) {
				ui.zsSlider->setValue(v / m * 1000.0);
				ui.zzDBox->setValue(m / v);
			}
			}
		);
	}

	{
		connect(parent->depSizeY, &DependedVariantContainer::update, [&, parent]() {
			float m = *parent->depSizeY;

			QSignalBlocker blocker(ui.xsDBox);
			ui.xsDBox->setMaximum(m);
			ui.xsDBox->setValue(ui.xsSlider->value() * m / 1000.0);

			*parent->viewSizeY = (float)ui.xsDBox->value();
			}
		);

		connect(ui.xsDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&, parent](double v) {
			*parent->viewSizeY = (float)v;
			}
		);

		connect(ui.xsSlider, &QSlider::valueChanged, [&, parent](int v) {
			*parent->viewSizeY = v * (float)*parent->depSizeY / 1000.0f;
			QSignalBlocker blocker(ui.xsSlider);
			ui.xsSlider->setValue(v);
			}
		);

		connect(ui.xzDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&, parent](double v) {
			*parent->viewSizeY = (float)*parent->depSizeY / (float)v;
			}
		);

		connect(parent->viewSizeY, &DependedVariantContainer::update, [&, parent]() {
			QSignalBlocker blocker1(ui.xsDBox);
			QSignalBlocker blocker2(ui.xsSlider);
			QSignalBlocker blocker3(ui.xzDBox);
			float v = *parent->viewSizeY, m = *parent->depSizeY;

			ui.xsDBox->setValue(v);
			if (v != 0) {
				ui.xsSlider->setValue(v / m * 1000.0);
				ui.xzDBox->setValue(m / v);
			}
			}
		);
	}

	parent->depSizeX->calculate();
	parent->depSizeY->calculate();
}

ZoomMenu::~ZoomMenu() {

}
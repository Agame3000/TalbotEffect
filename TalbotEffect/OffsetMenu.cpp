#include "OffsetMenu.h"
#include "MainWindow.h"

OffsetMenu::OffsetMenu(MainWindow* parent) : QWidget(parent) {
	ui.setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);

	{
		connect(parent->depSizeX, &DependedVariantContainer::update, [&, parent]() {
			float m = *parent->depSizeX;

			QSignalBlocker blocker1(ui.zDBox);
			QSignalBlocker blocker2(*parent->offsetX);
			ui.zDBox->setMaximum(m);
			float cur = ui.zSlider->value() * m / 1000.0;
			ui.zDBox->setValue(cur);
			*parent->offsetX = cur;
			}
		);

		connect(ui.zSlider, &QSlider::valueChanged, [&, parent](int v) {
			*parent->offsetX = v * (float)*parent->depSizeX / 1000.0f;
			QSignalBlocker blocker(ui.zSlider);
			ui.zSlider->setValue(v);
			}
		);

		connect(ui.zDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&, parent](double v) {
			*parent->offsetX = (float)v;
			}
		);

		connect(parent->offsetX, &DependedVariantContainer::update, [&, parent]() {
			QSignalBlocker blocker1(ui.zDBox);
			QSignalBlocker blocker2(ui.zSlider);
			float v = *parent->offsetX;

			ui.zDBox->setValue(v);
			ui.zSlider->setValue(v * 1000.0 / (float)*parent->depSizeX);
			}
		);
	}

	{
		connect(parent->depSizeY, &DependedVariantContainer::update, [&, parent]() {
			float m = *parent->depSizeY;

			QSignalBlocker blocker1(ui.xDBox);
			QSignalBlocker blocker2(*parent->offsetY);
			ui.xDBox->setMaximum(m);
			float cur = ui.xSlider->value() * m / 1000.0;
			ui.xDBox->setValue(cur);
			*parent->offsetY = cur;
			}
		);

		connect(ui.xSlider, &QSlider::valueChanged, [&, parent](int v) {
			*parent->offsetY = v * (float)*parent->depSizeY / 1000.0f;
			QSignalBlocker blocker(ui.xSlider);
			ui.xSlider->setValue(v);
			}
		);

		connect(ui.xDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&, parent](double v) {
			*parent->offsetY = (float)v;
			}
		);

		connect(parent->offsetY, &DependedVariantContainer::update, [&, parent]() {
			QSignalBlocker blocker1(ui.xDBox);
			QSignalBlocker blocker2(ui.xSlider);
			float v = *parent->offsetY;

			ui.xDBox->setValue(v);
			ui.xSlider->setValue(v * 1000.0 / (float)*parent->depSizeY);
			}
		);
	}

	parent->depSizeX->calculate();
	parent->depSizeY->calculate();
}

OffsetMenu::~OffsetMenu() {

}
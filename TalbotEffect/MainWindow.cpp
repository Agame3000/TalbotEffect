#include "MainWindow.h"
#include "OffsetMenu.h"
#include "ZoomMenu.h"
#include "ViewGLWidget.h"
#include "ViewOverlayFrame.h"
#include <qmath.h>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QColorDialog>

static float sinc(float x) {
	if (!x) return 1;
	return sin(x) / x;
}

QSplitter* setupSplitter(QFrame* oldFrame, Qt::Orientation orientation) {
	QWidget* parent = oldFrame->parentWidget();
	QRect geometry = oldFrame->geometry();
	QLayout* parentLayout = parent->layout();

	QSplitter* splitter = new QSplitter(orientation, parent);

	QList<QWidget*> children;
	QLayout* layout = oldFrame->layout();
	for (int i = 0; i < layout->count(); ++i) {
		QLayoutItem* item = layout->itemAt(i);
		if (item && item->widget()) {
			children.append(item->widget());
		}
	}

	for (QWidget* child : children) {
		child->setParent(splitter);
		splitter->addWidget(child);
	}

	if (parentLayout) {
		int index = -1;
		QLayoutItem* item = nullptr;
		QBoxLayout* boxLayout = qobject_cast<QBoxLayout*>(parentLayout);
		for (int i = 0; i < boxLayout->count(); ++i) {
			if (boxLayout->itemAt(i)->widget() == oldFrame) {
				index = i;
				break;
			}
		}
		if (index != -1) {
			boxLayout->removeWidget(oldFrame);
			boxLayout->insertWidget(index, splitter);
		}
	}

	splitter->setGeometry(geometry);
	delete oldFrame;

	return splitter;
}

MainWindow::MainWindow() : QMainWindow() {
	ui.setupUi(this);
	VViewOverlayFrame* vo = new VViewOverlayFrame(ui.vViewArea, this);
	HViewOverlayFrame* ho = new HViewOverlayFrame(ui.hViewArea, this);
	setupSplitter(ui.viewAreaFakeSplitter, Qt::Horizontal);
	setupSplitter(ui.splitterPartFakeSplitter, Qt::Vertical)->setSizes({300, 100});

	ui.lengthDBox->setValue(0.0000005);
	ui.lengthDBox->setSingleStep(0.00000001);
	ui.lengthDBox->setMinimum(0.00000001);
	ui.colorViewFrame->setAutoFillBackground(true);
	updateColorView();
	an = new std::vector<QVector2D>();

	vGLView = new ViewGLWidget(vo, &vUniData, this, ViewGLWidget::Vertical);
	hGLView = new ViewGLWidget(ho, &hUniData, this, ViewGLWidget::Horizontal);
	vo->layout()->addWidget(vGLView);
	vo->targetWidget = vGLView;
	ho->layout()->addWidget(hGLView);
	ho->targetWidget = hGLView;
	QSurfaceFormat format;
	format.setSamples(16);
	vGLView->setFormat(format);
	hGLView->setFormat(format);
	vGLView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	hGLView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	vGLView->setMouseTracking(true);
	hGLView->setMouseTracking(true);

	connect(ui.playButton, &QPushButton::pressed, [&]() {
		stopped = false;
		ui.stopButton->setEnabled(true);
		ui.playButton->setEnabled(false);
		ui.frameButton->setEnabled(false);
		vGLView->update();
		hGLView->update();
		}
	);

	connect(ui.stopButton, &QPushButton::pressed, [&]() {
		stopped = true;
		ui.playButton->setEnabled(true);
		ui.stopButton->setEnabled(false);
		ui.frameButton->setEnabled(true);
		}
	);

	connect(ui.frameButton, &QPushButton::pressed, [&]() {
		vGLView->update();
		hGLView->update();
		}
	);

	connect(ui.offsetButton, &QPushButton::pressed, [&]() {
		offsetMenu->show();
		offsetMenu->raise();
		offsetMenu->activateWindow();
		}
	);

	connect(ui.axisButton, &QPushButton::pressed, [&]() {
		zoomMenu->show();
		zoomMenu->raise();
		zoomMenu->activateWindow();
		}
	);

	{
		connect(viewSizeX, &DependedVariantContainer::update, [&]() {
			float m = *viewSizeX;

			QSignalBlocker blocker1(ui.cutDBox);
			QSignalBlocker blocker2(z);
			ui.cutDBox->setMaximum(m);
			float cur = ui.cutSlider->value() * m / 1000.0;
			ui.cutDBox->setValue(cur);
			*z = cur;
			}
		);

		connect(ui.cutSlider, &QSlider::valueChanged, [&](int v) {
			*z = v * (float)*viewSizeX / 1000.0f;
			QSignalBlocker blocker(ui.cutSlider);
			ui.cutSlider->setValue(v);
			}
		);

		connect(ui.cutDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double v) {
			*z = (float)v;
			}
		);

		connect(z, &DependedVariantContainer::update, [&]() {
			QSignalBlocker blocker1(ui.cutDBox);
			QSignalBlocker blocker2(ui.cutSlider);
			float v = *z;

			ui.cutDBox->setValue(v);
			ui.cutSlider->setValue(v * 1000.0 / (float)*viewSizeX);
			}
		);
	}

	connect(colorR, &MetadataComponent::update, this, &MainWindow::updateColorView);
	connect(colorG, &MetadataComponent::update, this, &MainWindow::updateColorView);
	connect(colorB, &MetadataComponent::update, this, &MainWindow::updateColorView);

	connect(ui.pickButton, &QPushButton::pressed, [&]() {
		int r, g, b;
		QColorDialog::getColor(qRgb(*colorR, *colorG, *colorB),
			this, QString::fromLocal8Bit("Выберите цвет")).getRgb(&r, &g, &b);
		ui.redBox->setValue(r); ui.greenBox->setValue(g); ui.blueBox->setValue(b);
		}
	);

	connect(ui.dDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double D) {
		*period = (float)D;
		}
	);
	connect(period, &MetadataComponent::update, [&]() {
		ui.dDBox->setValue((float)*period);
		}
	);
	connect(ui.widthDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double d) {
		*holeWidth = (float)d;
		}
	);
	connect(holeWidth, &MetadataComponent::update, [&]() {
		ui.widthDBox->setValue((float)*holeWidth);
		}
	);
	connect(ui.lengthDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double L) {
		*wavelength = (float)L;
		}
	);
	connect(wavelength, &MetadataComponent::update, [&]() {
		ui.lengthDBox->setValue((float)*wavelength);
		}
	);
	connect(ui.amplRButton, &QRadioButton::clicked, [&]() {
		*type = 0;
		}
	);
	connect(ui.phaseRButton, &QRadioButton::clicked, [&]() {
		*type = 1;
		}
	);
	connect(type, &MetadataComponent::update, [&]() {
		if ((int)*type == 0)
			ui.amplRButton->setChecked(true);
		else
			ui.phaseRButton->setChecked(true);
		}
	);

	connect(ui.redBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int r) {
		*colorR = r;
		}
	);
	connect(ui.greenBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int g) {
		*colorG = g;
		}
	);
	connect(ui.blueBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int b) {
		*colorB = b;
		}
	);
	connect(ui.cDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double c) {
		*this->c = (float)c;
		}
	);
	connect(c, &MetadataComponent::update, [&]() {
		ui.cDBox->setValue((float)*c);
		}
	);

	connect(ui.xDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double X) {
		*fieldSizeX = (float)X;
		}
	);
	connect(fieldSizeX, &MetadataComponent::update, [&]() {
		ui.xDBox->setValue((float)*fieldSizeX);
		}
	);
	connect(ui.nDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double Y) {
		*fieldSizeY = (float)Y;
		}
	);
	connect(fieldSizeY, &MetadataComponent::update, [&]() {
		ui.nDBox->setValue((float)*fieldSizeY);
		}
	);

	connect(ui.ztPBox, &QCheckBox::toggled, [&](bool xChecked) {
		*isXinZ = xChecked;
		}
	);
	connect(isXinZ, &MetadataComponent::update, [&]() {
		ui.ztPBox->setChecked(*isXinZ);
		}
	);
	connect(ui.yCBox, &QCheckBox::toggled, [&](bool yChecked) {
		*isYInN = yChecked;
		}
	);
	connect(isYInN, &MetadataComponent::update, [&]() {
		ui.yCBox->setChecked(*isYInN);
		}
	);

	connect(ui.precisionBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int P) {
		*precision = P;
		}
	);
	connect(precision, &MetadataComponent::update, [&]() {
		ui.precisionBox->setValue((int)*precision);
		}
	);
	connect(ui.phiDBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double step) {
		*phaseStep = (float)step;
		}
	);
	connect(phaseStep, &MetadataComponent::update, [&]() {
		ui.phiDBox->setValue((float)*phaseStep);
		}
	);

	connect(ui.saveAct, &QAction::triggered, [&]() {saveFile(false); });
	connect(ui.saveAsAct, &QAction::triggered, [&]() {saveFile(true); });
	connect(ui.openAct, &QAction::triggered, this, &MainWindow::openFile);

#define VUNIFORM(type, id, function, ...) \
vUniData.add(UniformDataComponent::make<type>(#id, [&](VariantContainer& u) {function}, {__VA_ARGS__}))
#define HUNIFORM(type, id, function, ...) \
hUniData.add(UniformDataComponent::make<type>(#id, [&](VariantContainer& u) {function}, {__VA_ARGS__}))
#define VHUNIFORM(type, id, function, ...) \
VUNIFORM(type, id, function, __VA_ARGS__); HUNIFORM(type, id, function, __VA_ARGS__)

	VUNIFORM(QVector2D, fieldSize,
		u = QVector2D(*viewSizeX, *viewSizeY);
	, viewSizeX, viewSizeY);
	HUNIFORM(float, w, u = (float)*viewSizeY; , viewSizeY);
	HUNIFORM(float, z, u = (float)*z; , z);
	VHUNIFORM(QVector2D, offset, u = QVector2D(*offsetX, *offsetY); , offsetX, offsetY);
	VHUNIFORM(float, d, u = (float)*period; , period);
	VHUNIFORM(int, type, u = (int)*type; , type);
	VHUNIFORM(float, c, u = (float)*c; , c);
	VHUNIFORM(int, k, u = (int)*precision; , precision);
	VHUNIFORM(float, zT, u = (float)*depZt;, depZt);
	VHUNIFORM(QVector3D, color,
		u = QVector3D((int)*colorR / 255.0f, (int)*colorG / 255.0f, (int)*colorB / 255.0f);
	, colorR, colorG, colorB);
	VHUNIFORM(QVector2D, a0,
		u = calcA0();
	, holeWidth, period, phaseStep, type);

	for (MetadataComponent* c : { holeWidth, period, phaseStep, precision, type })
		connect(c, &MetadataComponent::update, this, &MainWindow::recalculateAn);
	recalculateAn();

	depZt->calculate();
	depSizeX->calculate();
	depSizeY->calculate();

	vUniData.forEach([&](UniformDataComponent* u, int i) {
		u->calculate();
		connect(u, &UniformDataComponent::update, [&]() {
			if (!stopped)
				vGLView->update();
			}
		);
		}
	);
	hUniData.forEach([&](UniformDataComponent* u, int i) {
		u->calculate();
		connect(u, &UniformDataComponent::update, [&]() {
			if (!stopped)
				hGLView->update();
			}
		);
		}
	);

	offsetMenu = new OffsetMenu(this);
	offsetMenu->hide();
	zoomMenu = new ZoomMenu(this);
	zoomMenu->hide();

	connect(z, &MetadataComponent::update, vGLView, QOverload<>::of(&ViewGLWidget::update));

	for (MetadataComponent* c : metaComps) {
		connect(c, &MetadataComponent::update, [&]() {
			saved = false;
			updateWindowTitle();
			}
		);
	}
}

void MainWindow::saveFile(bool saveAs) {
	QString path = savePath;
	if (saveAs || path.isEmpty()) {
		path = QFileDialog::getSaveFileName(
			this,
			QString::fromLocal8Bit("Сохранить файл текущей конфигурации"),
			QString(),
			QString::fromLocal8Bit(
				"Файлы метаданных (*.meta);;"
				"Все файлы (*.*)"
			)
		);
	}

	if (!path.isEmpty() && (saveAs || !saved)) {
		qDebug() << QString::fromLocal8Bit("Файл будет сохранен как: ") << path;
		QFile file(path);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			qWarning() << QString::fromLocal8Bit("Не удалось открыть файл для записи:") <<
				file.errorString();
			return;
		}

		for (MetadataComponent* c : metaComps)
			qDebug() << file.write((char*)c->getData()->data(), QMetaType(c->type).sizeOf());

		file.close();
		qDebug() << QString::fromLocal8Bit("Файл сохранён: ") << path;
		savePath = path;
		saved = true;
		updateWindowTitle();
	}
}

void MainWindow::openFile() {
	QString path = QFileDialog::getOpenFileName(
		this,
		QString::fromLocal8Bit("Выберите файл конфигурации для открытия"),
		QString(),
		QString::fromLocal8Bit(
			"Файлы метаданных (*.meta);;"
			"Все файлы (*.*)"
		)
	);

	if (!path.isEmpty()) {
		qDebug() << QString::fromLocal8Bit("Выбран файл: ") << path;
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qWarning() << QString::fromLocal8Bit("Не удалось открыть файл для чтения:") <<
				file.errorString();
			return;
		}

		for (MetadataComponent* c : metaComps) {
			qDebug() << file.read((char*)c->getData()->data(), QMetaType(c->type).sizeOf());
			c->update();
		}

		file.close();
		qDebug() << QString::fromLocal8Bit("Файл прочитан: ") << path;
		savePath = path;
		saved = true;
		updateWindowTitle();
	}
}

void MainWindow::updateWindowTitle() {
	setWindowTitle(
		QString::fromLocal8Bit("Эффект Тальбота") +
		(savePath.isEmpty() ? "" : QString::fromLocal8Bit(" - ") + savePath) +
		(saved ? "" : "*")
	);
}

void MainWindow::updateWindowStatus() {
	ui.statusbar->showMessage(QString::fromLocal8Bit("Последняя отрисовка: ") +
		(vGLView->ready ? QString::number(vGLView->time) : "*") + "/" +
		(hGLView->ready ? QString::number(hGLView->time) : "*"),
		5000
	);
}

void MainWindow::updateColorView() {
	QPalette palette = ui.colorViewFrame->palette();
	palette.setColor(QPalette::Window, QColor::fromRgb(*colorR, *colorG, *colorB));
	ui.colorViewFrame->setPalette(palette);
	ui.redBox->setValue(*colorR);
	ui.greenBox->setValue(*colorG);
	ui.blueBox->setValue(*colorB);
}

void MainWindow::recalculateAn() {
	an->clear();
	int k = *precision, t = (int)*type;
	float w = *holeWidth, d = *period, phi = (float)*phaseStep * (float)M_PI;
	for (int n = -k; n < k; n++) {
		if (n == 0) an->push_back(QVector2D());
		else {
			if (t == 0) an->push_back(QVector2D(sinc(n * w / d), 0));
			else if (t == 1){
				float t = w / d * sinc(n * w / d);
				an->push_back(QVector2D((cos(phi) - 1) * t, sin(phi) * t));
			}
		}
	}
	if (!stopped) {
		vGLView->update();
		hGLView->update();
	}
}

QVector2D MainWindow::calcA0() {
	if((int)*type == 0)
		return QVector2D(1, 0);
	else if ((int)*type == 1) {
		float w = *holeWidth; float d = *period; float phi = (float)*phaseStep + (float)M_PI;
		return QVector2D(1.0 + w / d * (cos(phi) - 1.0), w / d * sin(phi));
	}
	return QVector2D();
}

MainWindow::~MainWindow() {
	for (MetadataComponent* c : metaComps)
		delete c;
	delete an;
	delete depZt;
	delete depSizeX;
	delete depSizeY;
}
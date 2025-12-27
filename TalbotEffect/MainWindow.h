#pragma once

#include "UniformDataStructure.h"
class ViewGLWidget;
class OffsetMenu;
class ZoomMenu;
#include <QMainWindow>
#include <QSplitter>
#include <QPainter>
#include <QMouseEvent>

#include "ui_MainWindow.h"

#define METADATA(name, value) \
MetadataComponent* name = MetadataComponent::make(value, metaComps)

class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
	std::vector<MetadataComponent*> metaComps;
public:
	MainWindow();
	~MainWindow();

	QVector2D calcA0();
	void updateWindowStatus();

	METADATA(period, 0.5f); // период d решётки
	METADATA(holeWidth, 0.05f); // ширина отверстий
	METADATA(wavelength, 0.0000005f); // длина волны
	METADATA(type, 0);

	METADATA(colorR, 255);
	METADATA(colorG, 255); // цвета волн будут использованы для одного uniform значения
	METADATA(colorB, 255);
	METADATA(c, 0.01f);

	METADATA(fieldSizeX, 2.0f); // размер поля по X
	METADATA(fieldSizeY, 3.0f); // размер поля по Y

	METADATA(isXinZ, true); // задан ли X в Zt
	METADATA(isYInN, true); // задан ли Y в n (количестве отверстий)

	METADATA(precision, 50); // точность
	METADATA(phaseStep, 0.1f); // фазовый шаг

	METADATA(offsetX, 0.0f);
	METADATA(offsetY, 0.0f);

	METADATA(viewSizeX, 2000000.0f);
	METADATA(viewSizeY, 1.5f);

	METADATA(z, 0.0f); // расстояние до решётки для правого отображения

	DependedVariantContainer* depZt = DependedVariantContainer::make<float>(
		[&](VariantContainer& c) {
			float d = *period, lam = *wavelength;
			c = 2.0f * d * d / lam;
		}
	, { period, wavelength });
	DependedVariantContainer* depSizeX = DependedVariantContainer::make<float>(
		[&](VariantContainer& c) {
			float x = *fieldSizeX;
			c = *isXinZ ? x * (float)*depZt : x;
		}
	, { fieldSizeX, isXinZ, depZt });
	DependedVariantContainer* depSizeY = DependedVariantContainer::make<float>(
		[&](VariantContainer& c) {
			float y = *fieldSizeY;
			c = *isYInN ? y * (float)*period : y;
		}
	, { fieldSizeY, isYInN, period });

	std::vector<QVector2D>* an;
private:
	Ui::MainWindowForm ui;

	void saveFile(bool saveAs);
	void openFile();
	void updateWindowTitle();

	bool saved = false;
	QString savePath;

	ViewGLWidget* vGLView;
	ViewGLWidget* hGLView;

	OffsetMenu* offsetMenu;
	ZoomMenu* zoomMenu;

	bool stopped = true;

	UniformDataStructure vUniData;
	UniformDataStructure hUniData;
private slots:
	void updateColorView();
	void recalculateAn();
};
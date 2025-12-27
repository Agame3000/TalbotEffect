#pragma once

class MainWindow;

#include "ui_ZoomMenu.h"

class ZoomMenu : public QWidget {
	Q_OBJECT
public:
	ZoomMenu(MainWindow* parent);
	~ZoomMenu();
private:
	Ui::AxisMenuForm ui;
};
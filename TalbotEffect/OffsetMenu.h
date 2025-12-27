#pragma once

class MainWindow;

#include "ui_OffsetMenu.h"

class OffsetMenu : public QWidget {
	Q_OBJECT
public:
	OffsetMenu(MainWindow* parent);
	~OffsetMenu();
private:
	Ui::OffsetMenuForm ui;
};
#include "MainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow* win = new MainWindow();
	win->show();
	int ret = a.exec();
	delete win;
	return ret;
}

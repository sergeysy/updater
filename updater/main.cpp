#include "updater.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	updater mainWindow;

	mainWindow.show();
	return a.exec();
}

#include <QTextCodec>
#include <QtWidgets/QApplication>

#include "updater.h"

int main(int argc, char *argv[])
{
    // For correct UTF-8 strings interpretation.
    QTextCodec * codec = QTextCodec::codecForName("UTF-8");
    if(nullptr != codec)
    {
        QTextCodec::setCodecForLocale(codec);
    }

	QApplication a(argc, argv);
	updater mainWindow;

	mainWindow.show();
	return a.exec();
}

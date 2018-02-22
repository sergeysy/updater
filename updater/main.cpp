#include <string>

#include <boost/filesystem.hpp>

#include <QTextCodec>
#include <QtWidgets/QApplication>

#include "gui/dialogauthorise.h"
#include "updater.h"

class ManagerLogger
{
public:
    ManagerLogger(const boost::filesystem::path& path)
    {
		freopen((path/"1.log").string().c_str(), "w", stdout);
        freopen((path/"2.log").string().c_str(), "w", stderr);
	}
	~ManagerLogger()
	{
		fclose(stdout);
		fclose(stderr);
	}
};

int main(int argc, char *argv[])
{
    ManagerLogger logger(boost::filesystem::path(argv[0]).parent_path());
    // For correct UTF-8 strings interpretation.
    QTextCodec * codec = QTextCodec::codecForName("UTF-8");
    if(nullptr != codec)
    {
        QTextCodec::setCodecForLocale(codec);
    }

	QApplication a(argc, argv);
    DialogAuthorise dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        updater mainWindow;

        mainWindow.show();
    }


	return a.exec();
}

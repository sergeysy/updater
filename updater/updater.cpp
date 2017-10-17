#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <QPushButton>
#include <QSettings>
#include <QProcess>
#include <QString>

#include "pinger.hpp"
#include "logger.hpp"
#include "updater.h"

updater::updater(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	/*try
	{

		boost::asio::io_service io_service;
		pinger p(io_service, "127.0.0.1");
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}*/
	connections();
}
//QCoreApplication::arguments()
void updater::connections()
{
	connect(ui.pbFindValidators, &QPushButton::clicked, this, &updater::findValidators);
}

void updater::findValidators()
{
	ui.listWidget->clear();
	const auto argsApplication = QCoreApplication::arguments();

	const auto pathBinaryFile = argsApplication.front();

	//boost::filesystem::path full_path(boost::filesystem::initial_path<boost::filesystem::path>());

	boost::filesystem::path full_path = boost::filesystem::system_complete(boost::filesystem::path(pathBinaryFile.toStdString().c_str()));

	const auto folder = full_path.stem();
	QSettings settings(QString::fromStdString(folder.string()), QSettings::Format::IniFormat);
	const auto IPStartSetting = settings.value(nameIPStartSetting, QString::fromLatin1("127.0.0.1")).toString();
	const auto IPEndSetting = settings.value(nameIPEndSetting, QString::fromLatin1("127.0.0.1")).toString();


	const auto login = settings.value(nameLogin, QString::fromLatin1("root")).toString();
	
	QString statusAuth;
	QString statusPing(QString::fromLatin1("online"));
	if (readIdValidator(login, IPStartSetting, QString::fromLatin1("/validator/settings/client-id")))
	{
		statusAuth = QString::fromLatin1("ID");
	}
	else
	{
		statusAuth = QString::fromLatin1("no ID");
	}

	QString result = QString::fromLatin1("IP:%1 - %2/ %3").arg(IPStartSetting).arg(statusPing).arg(statusAuth);
	ui.listWidget->addItem(result);
	//QString pathFileCopy;
	/*path p("/bin/bash");
	if (is_regular_file(p))*/
	boost::filesystem::path pathFileCopy("");
	
	
	
	
	/*if(login.isEmpty() || )
	//const auto password = settings.value(nameLogin, QString::fromLatin1(""));
	//scp ~/validator/build/release/validator/validator $LOGIN@$DEST_MACHINE:/validator/bin
	auto pingProcess = new QProcess();
	int exitCode = pingProcess->execute(QString::fromLatin1("scp"), pathFile<< );
	if (exitCode == 0) {
		s_ApplyBoard->addItem(bdName[i]);
	}*/
}
bool updater::readIdValidator(const QString& login, const QString& ip, const QString& file)
{
	//ssh $LOGIN@$DEST_MACHINE '/etc/init.d/validator.sh stop'
	auto process = new QProcess();
	const auto params = QStringList() << QString::fromLatin1("%1@%2").arg(login).arg(ip) << QString::fromLatin1("'%1'").arg(file);
	int exitCode = process->execute(QString::fromLatin1("ssh"), params);
	if (exitCode == 0) {
		const auto result = process->readAllStandardOutput();
		std::cerr << logger() << std::endl;
		return true;
	}

	return false;
}
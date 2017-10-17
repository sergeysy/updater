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
	const auto folder = full_path.parent_path();
	const auto pathSettingsFile = folder/"settings.ini";//full_path.replace_extension("ini");
	QSettings settings(QString::fromStdString(pathSettingsFile.string()), QSettings::Format::IniFormat);
	const auto IPStartSetting = settings.value(nameIPStartSetting, QString::fromLatin1("127.0.0.1")).toString();
	const auto IPEndSetting = settings.value(nameIPEndSetting, QString::fromLatin1("127.0.0.1")).toString();


	const auto login = settings.value(nameLogin, QString::fromLatin1("root")).toString();
	
	QString statusPing(QString::fromLatin1("offline"));
	try
	{

		boost::asio::io_service io_service;
		pinger ping(io_service, IPStartSetting.toStdString());
		io_service.run();

		if (ping.isAvailableDestination())
		{
			statusPing = QString::fromLatin1("");
		}
	}
	catch (std::exception& e)
	{
		std::cerr << logger() << "Exception: " << e.what() << std::endl;
	}

	QString idValidator;
	std::string tmpFile("client_id");
	//auto  destinationFile = folder/ tmpFile;
	auto  destinationFile = folder/ tmpFile;
	switch (readIdValidator(login, IPStartSetting, QString::fromLatin1("/validator/settings/client_id"), QString::fromStdString(folder.string())))
	{
		case 0:
		{
			if (boost::filesystem::is_regular_file(destinationFile) && boost::filesystem::exists(destinationFile))
			{
				std::ifstream in(destinationFile.string().c_str());
				std::string contents;
				std::getline(in, contents);

				idValidator = QString::fromStdString(contents);
			}
			else
			{
				idValidator = tr("NO ID");
			}
			break;
		}
		case -1:
		case -2:
		{
			idValidator = tr("no ID");
			statusPing = tr("no ping");
			break;
		}
		default:
		{
			idValidator = tr("no ID");
		}
	}

	QString result = QString::fromLatin1("IP:%1 - %2 %3").arg(IPStartSetting).arg(statusPing).arg(idValidator);
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
int updater::readIdValidator(const QString& login, const QString& ip, const QString& fileSource, const QString&  fileDestination)
{
#if defined(unix)
	//linux
	//ssh $LOGIN@$DEST_MACHINE '/etc/init.d/validator.sh stop'
	auto process = new QProcess();
	const auto params = QStringList() << QString::fromLatin1("%1@%2").arg(login).arg(ip) << QString::fromLatin1("'%1'").arg(fileSource);
	int exitCode = process->execute(QString::fromLatin1("ssh"), params);
	if (exitCode == 0)
	{
		const auto result = process->readAllStandardOutput();
		std::cerr << logger() << std::endl;
	}

	return exitCode;
#endif //end LINUX
	
#if defined(_WIN32) || defined(WIN32)
	//windows
	//pscp -scp root@10.25.153.15:/validator/bin/validator d:/temp
	auto process = new QProcess();
	/*const auto params = QStringList() << QString::fromLatin1("-scp %1@%2:%3").arg(login).arg(ip).arg(fileSource) << QString::fromLatin1("'%1'").arg(fileDestination);
	int exitCode = process->execute(QString::fromLatin1("pscp"), params);*/
	auto command = QString::fromLatin1("pscp -scp %1@%2:%3 %4").arg(login).arg(ip).arg(fileSource).arg(fileDestination);
	int exitCode = process->execute(command);
	if (exitCode >= 0)
	{
		const auto result = process->readAllStandardOutput();
		std::cerr << logger() << std::endl;
	}

	return exitCode;
#endif //WINDOWS

}
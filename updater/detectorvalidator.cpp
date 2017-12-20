#include <fstream>

#include <boost/asio.hpp>

#include <QCoreApplication>
#include <QJsonObject>
#include <QString>

#include "functor.hpp"
#include "pinger.hpp"
#include "logger.hpp"
#include "detectorvalidator.hpp"

QString DetectorValidator::noID = QT_TR_NOOP(QString::fromLatin1("no ID"));
QString DetectorValidator::noValidator = QT_TRANSLATE_NOOP(QString::fromLatin1("empty"), QString::fromLatin1("empty"));
QString DetectorValidator::pathSettings = QString::fromLatin1("/validator/settings");

DetectorValidator::DetectorValidator()
    : QObject(nullptr)
{
    noID = (tr("no ID"));
    noValidator = (tr("empty"));

}
DetectorValidator::~DetectorValidator()
{
}

DetectorValidator& DetectorValidator::setLogin(const QString& login)
{
    login_ = login;
    return *this;
}

DetectorValidator& DetectorValidator::setIp(const QString& ipString)
{
    ipString_ = ipString;
    return *this;
}
DetectorValidator& DetectorValidator::setPath(const boost::filesystem::path& path)
{
    path_ = path;
    return *this;
}

void DetectorValidator::process()
{
    std::cerr << logger() << "Process: " << ipString_.toStdString() << std::endl;
    QString statusPing;
    QString idValidator;
    process_ = new QProcess();
    statusPing = getStatusPing(ipString_);

    const auto result = readSettingsValidator(login_, ipString_, pathSettings, QString::fromStdString((path_/ipString_.toStdString()).string()));
    const auto pathDest = path_/ipString_.toStdString();
    idValidator = getIdValidator(result, pathDest);
    std::cerr << logger() <<"result="<<result<<std::endl;
    const auto timezone = getTimezone(result, pathDest);
    const auto config_version = getSettingValue(pathDest, "config_version");

    QJsonObject data;
    data.insert(QString::fromLatin1("idValidator"), idValidator);
    data.insert(QString::fromLatin1("timezone"), timezone);
    data.insert(QString::fromLatin1("config_version"), config_version);
    setSystemInfo(data, pathDest);

    emit haveData(ipString_, data);
    emit finished();
    return ;
}

void DetectorValidator::stop()
{

}

QString DetectorValidator::getIdValidator(const int result, const boost::filesystem::path& folder)
{
    QString idValidator;
    std::string tmpFile("client_id");
    auto  destinationFile = folder/ tmpFile;
    //std::cerr << logger() << ipString.toStdString() << ":" << result<< std::endl;
    switch (result)
    {
        case 0:
        {
            if (boost::filesystem::is_regular_file(destinationFile) && boost::filesystem::exists(destinationFile))
            {
                std::cerr << logger() << "Opening: " << destinationFile.string() << std::endl;
                std::ifstream in(destinationFile.string().c_str());
                std::string contents;
                std::getline(in, contents);

                idValidator = QString::fromStdString(contents);
            }
            else
            {
                idValidator = noID;
            }
            break;
        }
        case -1:
        case -2:
        {
            idValidator = noID;
            //statusPing = tr("no ping");
            break;
        }
        case 1://General error in file copy
        case 6://File does not exist
        {
            idValidator = noID;
            break;
        }
        default:
        {
            idValidator = noValidator;
        }
    }

    return idValidator;
}
QString DetectorValidator::getSettingValue(const boost::filesystem::path& folder, const std::string& name)
{
#if defined(unix)
    auto  destinationFile = folder/ name;
    QString value;
    if (boost::filesystem::is_regular_file(destinationFile) && boost::filesystem::exists(destinationFile))
    {
        std::cerr << logger() << "Opening: " << destinationFile.string() << std::endl;
        std::ifstream in(destinationFile.string().c_str());
        std::string contents;
        std::getline(in, contents);

        value = QString::fromStdString(contents);
    }

    std::cerr<<logger()<< name <<"="<<value.toStdString()<<std::endl;
    return value;
#else //end LINUX
    #error Not implemented upload transactions from validator on this platform
#endif
}

QString DetectorValidator::getTimezone(const int result, const boost::filesystem::path& folder)
{
#if defined(unix)
    std::cerr<<logger() << __FUNCTION__<<std::endl;
    QString timezone;
    if(0 != result)
    {
        std::cerr<<logger()<<"Error " << __FUNCTION__<<std::endl;
        return timezone;
    }
    std::string tmpFile("timezone");
    //auto  destinationFile = folder/ tmpFile;
    auto  destinationFile = folder/ tmpFile;
    std::cerr<<logger()<<destinationFile.string()<<std::endl;
    if (boost::filesystem::is_regular_file(destinationFile) && boost::filesystem::exists(destinationFile))
    {
        std::cerr << logger() << "Opening: " << destinationFile.string() << std::endl;
        std::ifstream in(destinationFile.string().c_str());
        std::string contents;
        std::getline(in, contents);

        timezone = QString::fromStdString(contents);
    }

    std::cerr<<logger()<<"timezone="<<timezone.toStdString()<<std::endl;
    return timezone;
#else //end LINUX
    #error Not implemented upload transactions from validator on this platform
#endif
}

QString DetectorValidator::getStatusPing(const QString& ipString)
{
    QString statusPing(QString::fromLatin1(""));
    //QString statusPing(QString::fromLatin1("offline"));
    try
    {

        boost::asio::io_service io_service;
        pinger ping(io_service, ipString.toStdString());
        io_service.run();

        if (ping.isAvailableDestination())
        {
            statusPing = QString::fromLatin1("");
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << logger() << __FILE__ << ":" << __LINE__ << ": " << e.what() << std::endl;
    }
    return statusPing;
}
int DetectorValidator::readSettingsValidator(const QString& login, const QString& ip, const QString& folderSource, const QString&  folderDestination)
{
#if defined(unix)
    //linux
    //ssh $LOGIN@$DEST_MACHINE '/etc/init.d/validator.sh stop'
//echo password | ssh id@server
    //ssh-keygen -f "/home/savin/.ssh/known_hosts" -R 10.25.153.15
    //ssh-keyscan -t ecdsa 10.25.153.15 >> ~/.ssh/known_hosts

    auto updateKey = QString::fromLatin1("\"ssh-keyscan -t ecdsa %1 >> ~/.ssh/known_hosts\"").arg(ip);
    //auto updateKey = QString::fromLatin1("ssh-keyscan -t ecdsa %1").arg(ip);
    process_->setProgram(QString::fromLatin1("/bin/bash"));
    process_->start();
    std::cout << logger() << "Update host: " << updateKey.toStdString()<<std::endl;
    int exitCode = process_->write(updateKey.toStdString().c_str());
    if (exitCode <= 0)
    {
        std::cerr << logger() << "ERROR execute: "<< updateKey.toStdString() << std::endl;
        return exitCode;
    }

    //"ssh -q root@10.25.153.16 exit"
    const auto checkConnectionParams = QStringList()<<QString::fromLatin1("-oStrictHostKeyChecking=no")  << QString::fromLatin1("-q") << QString::fromLatin1("%1@%2").arg(login).arg(ip)<< QString::fromLatin1("echo");
    std::cerr << logger() << "ssh " <<  checkConnectionParams.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    exitCode = process_->execute(QString::fromLatin1("ssh"), checkConnectionParams);
    if(exitCode != 0)
    {
        std::cerr << logger() << "Fail connection to " << ip.toStdString() << " status error code: " << exitCode << std::endl;
        return -3;
    }

    exitCode = prepareSystemInfo(login, ip, folderSource);
    if(exitCode != 0)
    {
        std::cerr << logger() << "Fail get system info validator" << std::endl;
    }

    std::cout << logger() << "copy settings validator" <<std::endl;
    boost::filesystem::path pathDestination(folderDestination.toStdString());
    if(boost::filesystem::is_directory(pathDestination) && boost::filesystem::exists(pathDestination))
    {
        boost::filesystem::remove_all(pathDestination);
    }
    if(boost::filesystem::is_directory(pathDestination) && !boost::filesystem::exists(pathDestination))
    {
        if(!boost::filesystem::create_directories(pathDestination))
        {
            std::cerr << logger() << "Can not create \""<< pathDestination.string() << "\"" << std::endl;
            return -1;
        }
    }

    const auto params = QStringList() << QString::fromLatin1("-r") << QString::fromLatin1("%1@%2:%3").arg(login).arg(ip).arg(folderSource)<<QString::fromLatin1("%1").arg(folderDestination);
    std::cerr << "scp " <<  params.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    exitCode = process_->execute(QString::fromLatin1("scp"), params);
    if(exitCode != 0)
    {
        std::cerr << logger() << "Fail copy settings validator" << std::endl;
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

int DetectorValidator::prepareSystemInfo(const QString& login, const QString& ip, const QString& folderSource)
{
    /*
     uname -or; egrep 'MemTotal|MemFree' /proc/meminfo; ls /mnt/dom/transaction -1 | wc -l
     */
//ssh root@10.25.153.15 "exec > /validator/settings/systeminfo;uname -or && egrep 'MemTotal|MemFree' /proc/meminfo && ls /mnt/dom/transaction -1 | wc -l"
    auto params = QStringList()<< QString::fromLatin1("%1@%2").arg(login).arg(ip)
                 << QString::fromLatin1("exec > %1/%2;uname -or &&  grep -E 'MemTotal|MemFree' /proc/meminfo | grep -oE '[0-9]+.*' && ls /mnt/dom/transaction -1 | wc -l")
                    .arg(folderSource).arg(QString::fromStdString(filenameSystemInfo_));
    std::cerr << logger() << "System info: ssh " << params.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    auto exitCode = process_->execute(QString::fromLatin1("ssh"), params);
    if(exitCode != 0)
    {
        std::cerr << logger() << "Fail get system info validator" << std::endl;
    }

    return exitCode;
}

void DetectorValidator::setSystemInfo(QJsonObject& data, const boost::filesystem::path& path)
{

    try
    {
        const auto destinationFile = path/filenameSystemInfo_;
        if(boost::filesystem::exists(destinationFile) && boost::filesystem::is_regular_file(destinationFile))
        {
            std::cerr << logger() << "Opening: " << destinationFile.string() << std::endl;
            std::ifstream in(destinationFile.string().c_str());
            std::string contents;
            if(!std::getline(in, contents))
            {
                return;
            }
            data.insert(QString::fromLatin1("osVersion"), QString::fromStdString(contents));

            if(!std::getline(in, contents))
            {
                return;
            }
            data.insert(QString::fromLatin1("memTotal"), QString::fromStdString(contents));

            if(!std::getline(in, contents))
            {
                return;
            }
            data.insert(QString::fromLatin1("memFree"), QString::fromStdString(contents));

            if(!std::getline(in, contents))
            {
                return;
            }
            data.insert(QString::fromLatin1("transactions"), QString::fromStdString(contents));

        }
    }
    catch(const boost::filesystem::filesystem_error & e)
    {

    }
}

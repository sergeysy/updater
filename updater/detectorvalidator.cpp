#include <fstream>

#include <boost/asio.hpp>

#include <QString>

#include "functor.hpp"
#include "pinger.hpp"
#include "logger.hpp"
#include "detectorvalidator.hpp"

QString DetectorValidator::noID = QT_TR_NOOP(QString::fromLatin1("no ID"));
QString DetectorValidator::noValidator = QT_TR_NOOP(QString::fromLatin1("empty"));
QString DetectorValidator::pathClient_id = QString::fromLatin1("/validator/settings/client_id");

DetectorValidator::DetectorValidator()
    : QObject(nullptr)
{
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

    statusPing = getStatusPing(ipString_);
    idValidator = getIdValidator(login_, ipString_, path_);


    emit haveData(ipString_, statusPing, idValidator);
    emit finished();
    return ;
}
void DetectorValidator::stop()
{

}
QString DetectorValidator::getIdValidator(const QString& login, const QString& ipString, const boost::filesystem::path& folder)
{
    QString idValidator;
    std::string tmpFile("client_id");
    //auto  destinationFile = folder/ tmpFile;
    auto  destinationFile = folder/ tmpFile;
    const auto result = readIdValidator(login, ipString, pathClient_id, QString::fromStdString(folder.string()));
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
    catch (std::exception& e)
    {
        std::cerr << logger() << __FILE__ << ":" << __LINE__ << ": " << e.what() << std::endl;
    }
    return statusPing;
}
int DetectorValidator::readIdValidator(const QString& login, const QString& ip, const QString& fileSource, const QString&  fileDestination)
{
#if defined(unix)
    //linux
    //ssh $LOGIN@$DEST_MACHINE '/etc/init.d/validator.sh stop'
//echo password | ssh id@server
    //ssh-keygen -f "/home/savin/.ssh/known_hosts" -R 10.25.153.15
    //ssh-keyscan -t ecdsa 10.25.153.15 >> ~/.ssh/known_hosts
    auto process = new QProcess();
    auto updateKey = QString::fromLatin1("ssh-keyscan -t ecdsa %1 >> ~/.ssh/known_hosts").arg(ip);
    //auto updateKey = QString::fromLatin1("ssh-keyscan -t ecdsa %1").arg(ip);
    process->setProgram(QString::fromLatin1("/bin/bash"));
    process->start();
    std::cout << logger() << "Update host" <<std::endl;
        int exitCode = process->write(updateKey.toStdString().c_str());
        if (exitCode <= 0)
        {
            std::cerr << logger() << "ERROR execute: "<< updateKey.toStdString() << std::endl;
            return exitCode;
        }

    //"ssh -q root@10.25.153.16 exit"
    const auto checkConnectionParams = QStringList() << QString::fromLatin1("-q") << QString::fromLatin1("%1@%2").arg(login).arg(ip)<< QString::fromLatin1("echo");
    std::cerr << logger() << "ssh " <<  checkConnectionParams.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    exitCode = process->execute(QString::fromLatin1("ssh"), checkConnectionParams);
    if(exitCode != 0)
    {
        std::cerr << logger() << "Fail connection to " << ip.toStdString() << " status error code: " << exitCode << std::endl;
        return -3;
    }

    std::cout << logger() << "Get ID validator" <<std::endl;
    boost::filesystem::path pathDestination(fileDestination.toStdString());
    if(boost::filesystem::is_directory(pathDestination) && !boost::filesystem::exists(pathDestination))
    {
        if(!boost::filesystem::create_directories(pathDestination))
        {
            std::cerr << logger() << "Can not create \""<< pathDestination.string() << "\"" << std::endl;
            return -1;
        }
    }

    const auto params = QStringList() << QString::fromLatin1("%1@%2:%3").arg(login).arg(ip).arg(fileSource)<<QString::fromLatin1("%1").arg(fileDestination);
    std::cerr << "scp " <<  params.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    exitCode = process->execute(QString::fromLatin1("scp"), params);
    if(exitCode != 0)
    {
        std::cerr << logger() << "Fail get ID validator" << std::endl;
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



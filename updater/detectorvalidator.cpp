#include <fstream>

#include <boost/asio.hpp>

#include <QString>

#include "functor.hpp"
#include "pinger.hpp"
#include "logger.hpp"
#include "detectorvalidator.hpp"

QString DetectorValidator::noID = QT_TR_NOOP(QString::fromLatin1("no ID"));
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
    switch (readIdValidator(login, ipString, pathClient_id, QString::fromStdString(folder.string())))
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
                idValidator = tr("NO ID");
            }
            break;
        }
        case -1:
        case -2:
        {
            idValidator = tr("no ID");
            //statusPing = tr("no ping");
            break;
        }
        default:
        {
            idValidator = tr("no ID");
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
    auto process = new QProcess();
    //ssh-keygen -f "/home/savin/.ssh/known_hosts" -R 10.25.153.15
    //ssh-keyscan -t ecdsa 10.25.153.15 >> ~/.ssh/known_hosts
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
    //int exitCode = process->execute(updateKey);

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


Transactions::Transactions(
        const QString &login,
        const QString &ipString,
        const QString& idString,
        const QString& sourceFolder,
        const QString& destFolder)
    : QObject(nullptr)
    , login_(login)
    , ipString_(ipString)
    , idString_(idString)
    , sourceFolder_(sourceFolder)
    , destFolder_(destFolder)
{

}

Transactions::~Transactions()
{
}

void Transactions::process()
{
#if defined(unix)
    auto f = [this]()
    {
        emit finished();
    };
    AutoCall0 autoCallFinished(std::bind(f));

    emit updateProcess(0, ipString_);
    auto process = new QProcess();
    auto updateKey = QString::fromLatin1("ssh-keyscan -t ecdsa %1 >> ~/.ssh/known_hosts").arg(ipString_);
    //auto updateKey = QString::fromLatin1("ssh-keyscan -t ecdsa %1").arg(ip);
    process->setProgram(QString::fromLatin1("/bin/bash"));
    process->start();
    std::cout << logger() << "Update host" <<std::endl;
        int exitCode = process->write(updateKey.toStdString().c_str());
        if (exitCode <= 0)
        {
            const auto message = tr("ERROR execute: %1").arg(updateKey);
            std::cerr << logger() << message.toStdString() << std::endl;
            emit error(idString_, message);
            return;
        }
    //int exitCode = process->execute(updateKey);

    emit updateProcess(10, ipString_);
    std::cerr << logger() << "Copy transactions..." <<std::endl;
    boost::filesystem::path pathDestination(destFolder_.toStdString());
    if(/*boost::filesystem::is_directory(pathDestination) &&*/ !boost::filesystem::exists(pathDestination))
    {
        if(!boost::filesystem::create_directories(pathDestination))
        {
            const auto message = tr("Can not create \"%1\"").arg(QString::fromStdString(pathDestination.string()));
            std::cerr << logger() << message.toStdString() << std::endl;
            emit error(idString_, message);
            return;
        }
    }
    emit updateProcess(25, ipString_);
    std::cerr << logger() << pathDestination.string() << std::endl;
    const auto paramsScp = QStringList() << QString::fromLatin1("-r") << QString::fromLatin1("%1@%2:%3").arg(login_).arg(ipString_).arg(sourceFolder_)<<QString::fromLatin1("%1").arg(destFolder_);
    std::cerr << logger() << "scp " <<  paramsScp.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    exitCode = process->execute(QString::fromLatin1("scp"), paramsScp);
    if(exitCode != 0)
    {
        QString message(tr("Fail copy transactions from %1. Error '%2'").arg(idString_).arg(exitCode));
        std::cerr << logger() << message.toStdString() << std::endl;
        emit error(idString_, message);
        return;
    }
    emit updateProcess(50, ipString_);

    //ssh username@domain.com 'rm /some/where/some_file.war'
    //const auto paramsRemove = QStringList() << QString::fromLatin1("%1@%2").arg(login_).arg(ipString_)<<QString::fromLatin1("'rm %1/*'").arg(sourceFolder_);
    //std::cerr << logger() << "ssh " <<  paramsRemove.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    //exitCode = process->execute(QString::fromLatin1("ssh"), paramsRemove);
    const auto paramsRemove = QStringList() << QString::fromLatin1("ssh %1@%2").arg(login_).arg(ipString_)<<QString::fromLatin1("\"rm -f %1/*\"").arg(sourceFolder_);
    const auto commanadRemoveTransactions = paramsRemove.join(QString::fromLatin1(" ")).toStdString();
    std::cerr << logger() << commanadRemoveTransactions << std::endl;
    exitCode = process->write(commanadRemoveTransactions.c_str());
    if(exitCode < 0)
    {
        QString message(tr("Fail remove transactions from %1. Error '%2'").arg(idString_).arg(exitCode));
        std::cerr << logger() << message.toStdString() << std::endl;
        emit error(idString_, message);
        return;
    }
    emit updateProcess(100, ipString_);


#else
#error Not implemented upload transactions from validator on this platform
#endif
}

void Transactions::stop()
{

}

Upload::Upload(const QString &login,
        const QString &ipString,
        const QString& idString,
        const QString& pathSourceSoftware,
        const QString& pathSourceWhitelist)
    : QObject(nullptr)
    , login_(login)
    , ipString_(ipString)
    , idString_(idString)
    , pathSourceSoftware_(pathSourceSoftware)
    , pathSourceWhitelist_(pathSourceWhitelist)
{

}

Upload::~Upload()
{

}

std::string Upload::findIpk(const boost::filesystem::path& path)
{
    std::cerr << logger() << path.string() <<std::endl;

    if(boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
    {
        std::cerr << logger() << __FILE__ << ":" << __LINE__ <<std::endl;
        for (boost::filesystem::directory_iterator itr(path); itr!=boost::filesystem::directory_iterator(); ++itr)
        {
            std::cerr << logger() << __FILE__ << ":" << __LINE__ <<std::endl;
            if (boost::filesystem::is_regular_file(itr->status()))
            {
                std::cerr << logger() << __FILE__ << ":" << __LINE__ <<std::endl;
                return itr->path().filename().string();
            }
        }
    }
    std::cerr << logger() << __FILE__ << ":" << __LINE__ <<std::endl;
    return std::string();
}

void Upload::mkDirOnValidator(const QString& path)
{
    const auto mkDirCommand = QString::fromLatin1("ssh %1@%2 \"mkdir -p %3\"").arg(login_).arg(ipString_).arg(path);
    std::cerr << logger() << mkDirCommand.toStdString() << std::endl;
    auto exitCode = process_->write(mkDirCommand.toStdString().c_str());
    if(exitCode < 0)
    {
        std::string message(tr("Fail create directory on %1. Error '%2'").arg(idString_).arg(exitCode).toStdString());
        //std::cerr << logger() << message << std::endl;
        throw std::logic_error(message);
    }
}

void Upload::copy(const QString& sourceFile, const QString& destFolder)
{
    const auto pathFile(sourceFile.toStdString());
    if(!boost::filesystem::exists(pathFile) || !boost::filesystem::is_regular_file(pathFile))
    {
        throw std::logic_error(QString::fromLatin1("File \"%1\" not found ").arg(sourceFile).toStdString());
    }
    const auto paramsScp = QStringList() << QString::fromLatin1("-r") << QString::fromLatin1("%1").arg(sourceFile)
                                         <<QString::fromLatin1("%1@%2:%3").arg(login_).arg(ipString_).arg(destFolder);
    std::cerr << logger() << "scp " <<  paramsScp.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    const auto exitCode = process_->execute(QString::fromLatin1("scp"), paramsScp);
    if(exitCode != 0)
    {
        std::string message(tr("Fail copy \"%3\" to \"%4\" on %1. Error '%2'").arg(idString_).arg(exitCode).arg(sourceFile).arg(destFolder).toStdString());
        std::cerr << logger() << message << std::endl;
        throw std::logic_error(message);
    }
}

void Upload::installIpk(const QString& pathIpk)
{
    return;
    const auto installIpkCommand = QString::fromLatin1("ssh %1@%2 \"mkdir -p %3\"").arg(login_).arg(ipString_).arg(pathIpk);
    std::cerr << logger() << installIpkCommand.toStdString() << std::endl;
    auto exitCode = process_->write(installIpkCommand.toStdString().c_str());
    if(exitCode < 0)
    {
        std::string message(tr("Fail create directory on %1. Error '%2'").arg(idString_).arg(exitCode).toStdString());
        //std::cerr << logger() << message << std::endl;
        throw std::logic_error(message);
    }
}

void Upload::process()
{
#if defined(unix)
    auto f = [this](){emit finished();};
    AutoCall0 autoCallFinished(std::bind(f));

    try
    {
        process_ = new QProcess();
        process_->setProgram(QString::fromLatin1("/bin/bash"));
        process_->start();

        if(!pathSourceSoftware_.isEmpty())
        {
            mkDirOnValidator(pathUpdateSoftware_);

            boost::filesystem::path pathIpk(pathSourceSoftware_.toStdString());
            std::string filenameIpk;
            filenameIpk = findIpk(pathIpk);

            if(!filenameIpk.empty())
            {
                auto fullPathIpk = QString::fromStdString((pathIpk/filenameIpk).string());
                copy(fullPathIpk, pathUpdateSoftware_);
                installIpk(fullPathIpk);
            }
        }
        else
        {
            std::cerr << logger() << "not need install ipk" << std::endl;
        }

        if(!pathSourceWhitelist_.isEmpty())
        {
            mkDirOnValidator(pathDestinationWhitelist_);
            copy(pathSourceWhitelist_, pathDestinationWhitelist_);
            //installWhitelist();
        }
        else
        {
            std::cerr << logger() << "Skip install whitelist" << std::endl;
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << logger() << ex.what() <<std::endl;
        emit error(idString_, QString::fromStdString(ex.what()));
        return;
    }


#else
#error Not implemented install ipk and update whitelist
#endif
}

void Upload::stop()
{

}


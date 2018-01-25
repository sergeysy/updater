#include "functor.hpp"
#include "logger.hpp"

#include "commands.hpp"

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

    auto message = tr("Copy data");
    emit updateProcess(0, message, ipString_);
    process_ = new QProcess();
    process_->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(process_, &QProcess::readyReadStandardError, this, &Transactions::readyReadStandardError, Qt::QueuedConnection);
    connect(process_, &QProcess::readyReadStandardOutput, this, &Transactions::readyReadStandardOutput, Qt::QueuedConnection);
    connect(process_, &QProcess::stateChanged, this, &Transactions::stateChanged, Qt::QueuedConnection);

    auto updateKey = QString::fromLatin1("ssh-keyscan -t ecdsa %1 >> ~/.ssh/known_hosts").arg(ipString_);
    //auto updateKey = QString::fromLatin1("ssh-keyscan -t ecdsa %1").arg(ip);
    process_->setProgram(QString::fromLatin1("/bin/bash"));
    process_->start();
    std::cout << logger() << "Update host" <<std::endl;
        int exitCode = process_->write(updateKey.toStdString().c_str());
        if (exitCode <= 0)
        {
            const auto message = tr("ERROR execute: %1.").arg(updateKey);
            std::cerr << logger() << message.toStdString() << std::endl;
            emit error(ipString_, message);
            return;
        }
    //int exitCode = process->execute(updateKey);

    emit updateProcess(10, message, ipString_);
    std::cerr << logger() << "Copy transactions..." <<std::endl;
    boost::filesystem::path pathDestination(destFolder_.toStdString());
    /*if(
            //boost::filesystem::is_directory(pathDestination) &&
            !boost::filesystem::exists(pathDestination))
    {
        if(!boost::filesystem::create_directories(pathDestination))
        {
            const auto message = tr("Can not create \"%1\".").arg(QString::fromStdString(pathDestination.string()));
            std::cerr << logger() << message.toStdString() << std::endl;
            emit error(ipString_, message);
            return;
        }
    }
    emit updateProcess(25, message, ipString_);*/
    std::cerr << logger() << pathDestination.string() << std::endl;
    const auto paramsScp = QStringList()<<QString::fromLatin1("-oStrictHostKeyChecking=no") << QString::fromLatin1("-r") << QString::fromLatin1("%1@%2:%3").arg(login_).arg(ipString_).arg(sourceFolder_)<<QString::fromLatin1("%1").arg(destFolder_);
    std::cerr << logger() << "scp " <<  paramsScp.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    exitCode = process_->execute(QString::fromLatin1("scp"), paramsScp);
    if(exitCode != 0)
    {
        QString message(tr("Fail copy transactions from %1. Error '%2'.").arg(idString_).arg(exitCode));
        std::cerr << logger() << message.toStdString() << std::endl;
        emit error(ipString_, message);
        return;
    }
    message = tr("Delete data");
    emit updateProcess(50, message, ipString_);

    //ssh username@domain.com 'rm /some/where/some_file.war'
    //const auto paramsRemove = QStringList() << QString::fromLatin1("%1@%2").arg(login_).arg(ipString_)<<QString::fromLatin1("'rm %1/*'").arg(sourceFolder_);
    //std::cerr << logger() << "ssh " <<  paramsRemove.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    //exitCode = process->execute(QString::fromLatin1("ssh"), paramsRemove);
    const auto paramsRemove = QStringList() << QString::fromLatin1("ssh -oStrictHostKeyChecking=no %1@%2").arg(login_).arg(ipString_)<<QString::fromLatin1("rm -f %1/*").arg(sourceFolder_);
    const auto commanadRemoveTransactions = paramsRemove.join(QString::fromLatin1(" ")).toStdString();
    std::cerr << logger() << commanadRemoveTransactions << std::endl;
    exitCode = process_->write(commanadRemoveTransactions.c_str());
    if(exitCode < 0)
    {
        QString message(tr("Fail remove transactions from %1. Error '%2'.").arg(idString_).arg(exitCode));
        std::cerr << logger() << message.toStdString() << std::endl;
        emit error(ipString_, message);
        return;
    }
    message = tr("Finished");
    emit updateProcess(100, message, ipString_);

#else
#error Not implemented upload transactions from validator on this platform
#endif
}

void Transactions::stateChanged(QProcess::ProcessState /*newState*/)
{
    std::cerr<<"new state "<< std::endl;
}

void Transactions::readyReadStandardError()
{
    std::cerr << "111111111111: "<< QString::fromLatin1(process_->readAllStandardError()).toStdString();
}

void Transactions::readyReadStandardOutput()
{
    std::cout << "111111111111: " << QString::fromLatin1(process_->readAllStandardOutput()).toStdString();
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
    std::cerr << logger()<<"Upload::findIpk: " << path.string() <<std::endl;

    if(boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
    {
        //std::cerr << logger() << __FILE__ << ":" << __LINE__ <<std::endl;
        for (boost::filesystem::directory_iterator itr(path); itr!=boost::filesystem::directory_iterator(); ++itr)
        {
            //std::cerr << logger() << __FILE__ << ":" << __LINE__ <<std::endl;
            if (boost::filesystem::is_regular_file(itr->status()) && itr->path().extension().string().compare(".zip")==0)
            {
                //std::cerr << logger() << __FILE__ << ":" << __LINE__ <<std::endl;
                std::cerr << logger() << "founded ipk: " << itr->path().filename().string() << std::endl;
                return itr->path().filename().string();
            }
        }
    }
    //std::cerr << logger() << __FILE__ << ":" << __LINE__ <<std::endl;
    return std::string();
}

void Upload::mkDirOnValidator(const QString& path)
{
    const auto mkDirCommand = QStringList()<<QString::fromLatin1("-oStrictHostKeyChecking=no") <<QString::fromLatin1("%1@%2").arg(login_).arg(ipString_) <<QString::fromLatin1("mkdir -p %1").arg(path);
    std::cerr << logger() << mkDirCommand.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    auto exitCode = process_->execute(QString::fromLatin1("ssh"),mkDirCommand);
    if(exitCode != 0)
    {
        std::string message(tr("Fail create directory on %1. Error '%2'.").arg(idString_).arg(exitCode).toStdString());
        //std::cerr << logger() << message << std::endl;
        throw std::logic_error(message);
    }
}

void Upload::copy(const QString& sourceFile, const QString& destFolder)
{
    std::cerr<<logger()<<"Start copy..."<<std::endl;
    const auto pathFile(sourceFile.toStdString());
    if(!boost::filesystem::exists(pathFile) || !boost::filesystem::is_regular_file(pathFile))
    {
        throw std::logic_error(QString::fromLatin1("File \"%1\" not found.").arg(sourceFile).toStdString());
    }
    const auto paramsScp = QStringList()<<QString::fromLatin1("-oStrictHostKeyChecking=no")
                                       << QString::fromLatin1("-r") << QString::fromLatin1("%1").arg(sourceFile)
                                         <<QString::fromLatin1("%1@%2:%3").arg(login_).arg(ipString_).arg(destFolder);
    std::cerr << logger() << "scp " <<  paramsScp.join(QString::fromLatin1(" ")).toStdString() << std::endl;
    const auto exitCode = process_->execute(QString::fromLatin1("scp"), paramsScp);
    if(exitCode != 0)
    {
        std::string message(tr("Fail copy \"%3\" to \"%4\" on %1. Error '%2'.").arg(idString_).arg(exitCode).arg(sourceFile).arg(destFolder).toStdString());
        std::cerr << logger() << message << std::endl;
        throw std::logic_error(message);
    }
}

void Upload::installIpk(const QString& pathIpk)
{
    std::cerr<<logger()<<"Start install..."<<std::endl;
    const auto installIpkCommandParams = QStringList()
            <<QString::fromLatin1("-oStrictHostKeyChecking=no")
            << QString::fromLatin1("%1@%2").arg(login_).arg(ipString_)
               << QString::fromLatin1("unzip -o %1/%2 -d %1").arg(pathUpdateSoftware_).arg(QString::fromStdString(boost::filesystem::path(pathIpk.toStdString()).filename().string()));
    std::cerr << logger() << "ssh " << installIpkCommandParams.join(QString::fromLatin1(" ")).toStdString() << std::endl;

    auto exitCode = process_->execute(QString::fromLatin1("ssh"), installIpkCommandParams);
    if(exitCode != 0)
    {
        std::string message(tr("Fail create directory on %1. Error '%2'.").arg(idString_).arg(exitCode).toStdString());
        throw std::logic_error(message);
    }

    const auto installIpkCommandParams2 = QStringList()
            <<QString::fromLatin1("-oStrictHostKeyChecking=no")
            << QString::fromLatin1("%1@%2").arg(login_).arg(ipString_)
               << QString::fromLatin1("bash %1/install.sh").arg(pathUpdateSoftware_);
    std::cerr << logger() << "ssh " << installIpkCommandParams2.join(QString::fromLatin1(" ")).toStdString() << std::endl;

    exitCode = process_->execute(QString::fromLatin1("ssh"), installIpkCommandParams2);
    if(exitCode != 0)
    {
        std::string message(tr("Fail execute install.sh on %1. Error '%2'.").arg(idString_).arg(exitCode).toStdString());
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
        auto message = tr("Update software");
        emit updateProcess(0, message, ipString_);
        process_ = new QProcess();
        process_->setProgram(QString::fromLatin1("/bin/bash"));
        process_->start();

        if(!pathSourceSoftware_.isEmpty())
        {
            mkDirOnValidator(pathUpdateSoftware_);

            boost::filesystem::path pathIpk(pathSourceSoftware_.toStdString());
            std::string filenameIpk;
            //filenameIpk = findIpk(pathIpk);
            filenameIpk = pathIpk.string();

            if(!filenameIpk.empty())
            {
                /*auto fullPathIpk = QString::fromStdString((pathIpk/filenameIpk).string());
                message = tr("update software");
                emit updateProcess(0, message, ipString_);
                copy(fullPathIpk, pathUpdateSoftware_);
                emit updateProcess(25, message, ipString_);
                installIpk(QString::fromStdString((boost::filesystem::path(pathUpdateSoftware_.toStdString())/filenameIpk).string()));
                emit updateProcess(50, message, ipString_);*/
                auto fullPathIpk = QString::fromStdString(filenameIpk);
                message = tr("update software");
                emit updateProcess(0, message, ipString_);
                //copy(fullPathIpk, pathUpdateSoftware_);
                copy(fullPathIpk, pathUpdateSoftware_);
                emit updateProcess(25, message, ipString_);
                installIpk(QString::fromStdString((boost::filesystem::path(pathUpdateSoftware_.toStdString())/filenameIpk).string()));
                emit updateProcess(50, message, ipString_);
            }
            else
            {
                std::cerr<<logger()<<"file ipk empty"<<std::endl;
            }
        }
        else
        {
            std::cerr << logger() << "not need install ipk" << std::endl;
        }

        message = tr("Update \"whitelist\"");
        emit updateProcess(50, message, ipString_);
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

        message = tr("Finished");
        emit updateProcess(100, message, ipString_);
    }
    catch(const std::exception& ex)
    {
        std::cerr << logger() << ex.what() <<std::endl;
        emit error(ipString_, QString::fromStdString(ex.what()));
        return;
    }


#else
#error Not implemented install ipk and update whitelist
#endif
}

void Upload::stop()
{

}

DownloadUpdates::DownloadUpdates(const QString &scriptFile, const QString& sourcePathSW, const QString& destinationPath)
    :scriptFile_(scriptFile)
    , sourcePathSW_(sourcePathSW)
    , destinationPath_(destinationPath)
{

}

DownloadUpdates::~DownloadUpdates()
{

}

void DownloadUpdates::process()
{
#if defined(unix)
    QString message = tr("Start download updates...");
    std::cout << logger() << message.toStdString() << std::endl;
    int percent = 0;
    emit updateProcess(percent, message);
    auto f = [this, &percent]()
    {
        emit finished();
    };
    AutoCall0 autoCallFinished(std::bind(f));

    try
    {
        process_ = new QProcess();
        connect(process_, &QProcess::readyReadStandardError, this, &DownloadUpdates::readyReadStandardError, Qt::QueuedConnection);
        connect(process_, &QProcess::readyReadStandardOutput, this, &DownloadUpdates::readyReadStandardOutput, Qt::QueuedConnection);
        process_->setProgram(QString::fromLatin1("/bin/bash"));
        process_->start();

        const auto params = QStringList() << sourcePathSW_ << destinationPath_;
        std::cerr << logger() << scriptFile_.toStdString() << " " << params.join(QStringLiteral(" ")).toStdString() << std::endl;
        auto exitCode = process_->execute(scriptFile_, params);
        if(exitCode != 0)
        {
            std::string message(tr("Fail download updates. Error code '%1'.").arg(exitCode).toStdString());
            throw std::logic_error(message);
        }
        percent = 100;
        message = tr("Finished download updates.");
        emit updateProcess(percent, message);
    }
    catch(const std::exception& ex)
    {
        std::cerr << logger() << ex.what() <<std::endl;
        emit error(QString::fromStdString(ex.what()));
    }

#else
#error Not implemented download updates
#endif
}

void DownloadUpdates::stop()
{

}

void DownloadUpdates::readyReadStandardError()
{
    std::cerr << "111111111111: "<< QString::fromLatin1(process_->readAllStandardError()).toStdString();
}

void DownloadUpdates::readyReadStandardOutput()
{
    std::cout << "111111111111: " << QString::fromLatin1(process_->readAllStandardOutput()).toStdString();
}


#include "functor.hpp"
#include "logger.hpp"

#include "commands.hpp"


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
            filenameIpk = pathIpk.string();

            if(!filenameIpk.empty())
            {
                auto fullPathIpk = QString::fromStdString(filenameIpk);
                message = tr("update software");
                emit updateProcess(0, message, ipString_);
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

ScriptExecute::ScriptExecute(const QString &fileNameScript,
        const QStringList &paramsScript,
        const QString &ipString,
        const QString& startMessage,
        const QString& finishMessage,
        const QString& errorMessage)
    : fileNameScript_(fileNameScript)
    , paramsScript_(paramsScript)
    , ipString_(ipString)
    , startMessage_(startMessage)
    , finishMessage_(finishMessage)
    , errorMessage_(errorMessage)
{

}

ScriptExecute::~ScriptExecute()
{

}

void ScriptExecute::process()
{
    auto f = [this]()
    {
        emit finished();
    };
    AutoCall0 autoCallFinished(std::bind(f));

    try{
        emit updateProcess(10, startMessage_, ipString_);
        std::cerr << logger() << startMessage_.toStdString() <<std::endl;
        process_ = new QProcess(this);
        process_->setProcessChannelMode(QProcess::ForwardedChannels);
        connect(process_, &QProcess::readyReadStandardError, this, &ScriptExecute::readyReadStandardError, Qt::QueuedConnection);
        connect(process_, &QProcess::readyReadStandardOutput, this, &ScriptExecute::readyReadStandardOutput, Qt::QueuedConnection);
        connect(process_, &QProcess::stateChanged, this, &ScriptExecute::stateChanged, Qt::QueuedConnection);

        std::cerr << logger() << fileNameScript_.toStdString() << " " <<  paramsScript_.join(QString::fromLatin1(" ")).toStdString() << std::endl;
        const auto exitCode = process_->execute(fileNameScript_, paramsScript_);
        if(exitCode != 0)
        {
            QString message(tr("%1: %3. Error code '%2'.").arg(ipString_).arg(exitCode).arg(errorMessage_));
            std::cerr << logger() << message.toStdString() << std::endl;
            emit error(ipString_, message);
            return;
        }
        emit updateProcess(100, finishMessage_, ipString_);
    }
    catch(const std::exception& ex)
    {
        QString message(tr("%3: %1. Error message '%2'").arg(errorMessage_).arg(QString::fromStdString(ex.what())).arg(ipString_));
        std::cerr << logger() << message.toStdString() <<std::endl;
        emit error(ipString_, message);
    }
}

void ScriptExecute::readyReadStandardError()
{
    std::cerr<<"new state "<< std::endl;
}

void ScriptExecute::readyReadStandardOutput()
{
    std::cerr << "111111111111: "<< QString::fromLatin1(process_->readAllStandardError()).toStdString();
}

void ScriptExecute::stateChanged(QProcess::ProcessState)
{
    std::cout << "111111111111: " << QString::fromLatin1(process_->readAllStandardOutput()).toStdString();
}

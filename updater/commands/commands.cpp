#include <QtCore/QtCore>

#include "functor.hpp"
#include "logger.hpp"

#include "commands.hpp"


Upload::Upload(const QString &login,
        const QString &ipString,
        const QString& idString,
        const QString &scriptInstallApplication,
        const QString &sourceFolderApplication,
        const QString &scriptInstallWhitelist,
        const QString &sourceFolderWhitelist)
    : QObject(nullptr)
    , login_(login)
    , ipString_(ipString)
    , idString_(idString)
    , scriptInstallApplication_(scriptInstallApplication)
    , sourceFolderApplication_(sourceFolderApplication)
    , scriptInstallWhitelist_(scriptInstallWhitelist)
    , sourceFolderWhitelist_(sourceFolderWhitelist)
{

}

Upload::~Upload()
{

}

void Upload::process()
{
    auto f = [this](){emit finished();};
    AutoCall0 autoCallFinished(std::bind(f));

    try
    {
        auto message = tr("Update software");
        emit updateProcess(0, message, ipString_);
        process_ = new QProcess();
        process_->setProgram(QString::fromLatin1("/bin/bash"));
        process_->start();

        if(!sourceFolderApplication_.isEmpty())
        {
            const auto params = QStringList()<< login_<< ipString_ << sourceFolderApplication_;
            std::cerr << logger() << scriptInstallApplication_.toStdString() << " " <<  params.join(QString::fromLatin1(" ")).toStdString() << std::endl;
            const auto exitCode = process_->execute(scriptInstallApplication_, params);
            if(exitCode != 0)
            {
                std::string message(tr("%1: Fail install application. Error '%2'.").arg(idString_).arg(exitCode).toStdString());
                std::cerr << logger() << message << std::endl;
                throw std::logic_error(message);
            }
        }
        else
        {
            std::cerr << logger() << "ipk not need install" << std::endl;
        }

        message = tr("Update \"whitelist\"");
        emit updateProcess(50, message, ipString_);
        if(!sourceFolderWhitelist_.isEmpty())
        {
            const auto params = QStringList()<< login_ << ipString_ << sourceFolderWhitelist_;
            std::cerr << logger() << scriptInstallWhitelist_.toStdString() << " " <<  params.join(QString::fromLatin1(" ")).toStdString() << std::endl;
            const auto exitCode = process_->execute(scriptInstallWhitelist_, params);
            if(exitCode != 0)
            {
                std::string message(tr("%1: Fail install whitelist. Error '%2'.").arg(idString_).arg(exitCode).toStdString());
                std::cerr << logger() << message << std::endl;
                throw std::logic_error(message);
            }
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
        std::cerr << "Let's Go";
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
        auto buf1= process_->readAllStandardOutput();
        auto buf2 = process_->readAllStandardError();
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

    QProcess *p = (QProcess *)sender();
      QByteArray buf = p->readAllStandardOutput();

      //std::cerr << buf;
}

void ScriptExecute::readyReadStandardOutput()
{
    std::cerr << "111111111111: "<< QString::fromLatin1(process_->readAllStandardError()).toStdString();

    QProcess *p = (QProcess *)sender();
      QByteArray buf = p->readAllStandardOutput();

      //std::cerr << buf;
}

void ScriptExecute::stateChanged(QProcess::ProcessState)
{
    std::cout << "111111111111: " << QString::fromLatin1(process_->readAllStandardOutput()).toStdString();
}

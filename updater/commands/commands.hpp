#pragma once

#include <boost/filesystem.hpp>

#include <QProcess>
#include <QString>
#include <QObject>
#include <QCoreApplication>

class Transactions : public QObject
{
    Q_OBJECT
public:
    Transactions(const QString& login, const QString& ipString, const QString& idString, const QString &sourceFolder, const QString &destFolder);
    virtual ~Transactions();

public slots:
    void process();
    void stop();

signals:
    void finished();
    void error(const QString idString, const QString);
    void updateProcess(int percent, const QString message, const QString ipString);

private:
    QString login_;
    QString ipString_;
    QString idString_;
    QString sourceFolder_;
    QString destFolder_;

    QProcess *process_;
private slots:
    void readyReadStandardError();
    void readyReadStandardOutput();
    void stateChanged(QProcess::ProcessState /*newState*/);
};

class Upload : public QObject
{
    Q_OBJECT
public:
    Upload(const QString &login, const QString &ipString, const QString &idString, const QString &pathSourceSoftware, const QString &pathSourceWhitelist);
    virtual ~Upload();

public slots:
    void process();
    void stop();

signals:
    void finished();
    void error(const QString idString, const QString message);
    void updateProcess(int percent, const QString message, const QString ipString);

private:
    QString login_;
    QString ipString_;
    QString idString_;
    QString pathSourceSoftware_;
    QString pathSourceWhitelist_;

    QString pathUpdateSoftware_ = QString::fromLatin1("/home/updater/updates");
    QString pathDestinationWhitelist_ = QString::fromLatin1("/mnt/dom");
    QProcess* process_;


    std::string findIpk(const boost::filesystem::path& path);
    void mkDirOnValidator(const QString& path);

    void copy(const QString &sourceFile, const QString &destFolder);
    void installIpk(const QString &pathIpk);

    void copyWhitelist();

};

class DownloadUpdates : public QObject
{
    Q_OBJECT
public:
    DownloadUpdates(const QString& scriptFile, const QString& sourcePathSW, const QString& destinationPath);
    ~DownloadUpdates();

public slots:
    void process();
    void stop();

signals:
    void finished();
    void error(const QString);
    void updateProcess(int percent, const QString message);

private:
    QString scriptFile_;
    QString sourcePathSW_;
    QString destinationPath_;
    QProcess *process_;

private slots:
    void readyReadStandardError();
    void readyReadStandardOutput();
};

#pragma once

#include <boost/filesystem.hpp>

#include <QProcess>
#include <QString>
#include <QObject>

class DetectorValidator : public QObject
{
    Q_OBJECT
public:
    DetectorValidator();
    virtual ~DetectorValidator();

    DetectorValidator& setLogin(const QString& login);

    DetectorValidator& setIp(const QString& ipString);
    DetectorValidator& setPath(const boost::filesystem::path& path);

public slots:
    void process();
    void stop();
signals:
    void haveData(const QString ipString, const QString statusPing, const QString idValidator);
    void finished();

private:
    QString login_;
    QString ipString_;
    boost::filesystem::path path_;

    QString getIdValidator(const QString& login, const QString& ipString, const boost::filesystem::path& folder);

    QString getStatusPing(const QString& ipString);
    int readIdValidator(const QString& login, const QString& ip, const QString& fileSource, const QString&  fileDestination);
};

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
    void updateProcess(int percent, const QString idString);

private:
    QString login_;
    QString ipString_;
    QString idString_;
    QString sourceFolder_;
    QString destFolder_;
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

private:
    QString login_;
    QString ipString_;
    QString idString_;
    QString pathSourceSoftware_;
    QString pathSourceWhitelist_;

    QString pathUpdateSoftware_ = QString::fromLatin1("/validator/update");
    QString pathDestinationWhitelist_ = QString::fromLatin1("/mnt/sda");
    QProcess* process_;


    std::string findIpk(const boost::filesystem::path& path);
    void mkDirOnValidator(const QString& path);

    void copy(const QString &sourceFile, const QString &destFolder);
    void installIpk(const QString &pathIpk);

    void copyWhitelist();

};

#pragma once

#include <boost/filesystem.hpp>

#include <QProcess>
#include <QString>
#include <QObject>
#include <QCoreApplication>

class ScriptExecute final : public QObject
{
    Q_OBJECT
public:
    ScriptExecute(const QString& fileNameScript,
            const QStringList& paramsScript,
            const QString& ipString,
            const QString &startMessage,
            const QString &finishMessage,
            const QString &errorMessage);
    virtual ~ScriptExecute();

public slots:
    void process();

signals:
    void finished();
    void error(const QString,
               const QString);
    void updateProcess(int percent,
                       const QString message,
                       const QString ipString);

private:
    QString fileNameScript_;
    QStringList paramsScript_;
    QString ipString_;
    const QString startMessage_;
    const QString finishMessage_;
    const QString errorMessage_;



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
    Upload(const QString &login,
           const QString &ipString,
           const QString &idString,
           const QString &pathSourceSoftware,
           const QString &pathSourceWhitelist);
    virtual ~Upload();

public slots:
    void process();
    void stop();

signals:
    void finished();
    void error(const QString idString,
               const QString message);
    void updateProcess(int percent,
                       const QString message,
                       const QString ipString);

private:
    QString login_;
    QString ipString_;
    QString idString_;
    QString pathSourceSoftware_;
    QString pathSourceWhitelist_;

    QString pathUpdateSoftware_ = QString::fromLatin1("/home/updater/updates");
    QString pathDestinationWhitelist_ = QString::fromLatin1("/mnt/dom");
    QProcess* process_;


    void mkDirOnValidator(const QString& path);

    void copy(const QString &sourceFile,
              const QString &destFolder);
    void installIpk(const QString &pathIpk);

    void copyWhitelist();

};


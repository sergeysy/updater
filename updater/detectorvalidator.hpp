#pragma once

#include <boost/filesystem.hpp>

#include <QJsonObject>
#include <QProcess>
#include <QString>
#include <QObject>
#include <QCoreApplication>


class DetectorValidator : public QObject
{
    Q_OBJECT
public:
    static QString noID;
    static QString noValidator;
    DetectorValidator();
    virtual ~DetectorValidator();

    DetectorValidator& setLogin(const QString& login);

    DetectorValidator& setIp(const QString& ipString);
    DetectorValidator& setPath(const boost::filesystem::path& path);

public slots:
    void process();
    void stop();
signals:
    void haveData(const QString ipString, const QJsonObject data);
    void finished();

private:
    QString login_;
    QString ipString_;
    boost::filesystem::path path_;
    QProcess *process_;
    std::string filenameSystemInfo_ = "systeminfo";

    QString getIdValidator(const int result, const boost::filesystem::path& folder);
    QString getTimezone(const int result, const boost::filesystem::path& folder);
    int prepareSystemInfo(const QString& login, const QString& ip, const QString& folderSource);
    QString getStatusPing(const QString& ipString);
    int readSettingsValidator(const QString& login, const QString& ip, const QString& folderSource, const QString&  folderDestination);

    QString getSettingValue(const boost::filesystem::path &folder, const std::string &name);
    void setSystemInfo(QJsonObject &data, const boost::filesystem::path &path);
public:
    static QString pathSettings;
};


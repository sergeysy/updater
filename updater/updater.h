#pragma once

#include <boost/filesystem.hpp>

#include <QJsonObject>

#include <QSettings>
#include <QtWidgets/QMainWindow>
#include <QAtomicInt>

#include "account.h"
#include "model/validatorlistmodel.hpp"
#include "ui_updater.h"

class updater : public QMainWindow
{
	Q_OBJECT

public:
	updater(QWidget *parent = Q_NULLPTR);
    ~updater();

	public slots:
    void commnadFindValidators();
    void commandUploadTransactions();
    void updateListDevices(const QString ipString,
                           const QJsonObject jsonObject);
    void updateProcessTransactions(int percent,
                                   const QString message,
                                   const QString idString);
    void finishedTransactions();

    void commandUpdateValidator();
    void finishedUpdateValidator();

    void showInfoValidator(const QModelIndex &index);
    void commandUploadTransactionToServer();
    void errorProcess(const QString ipString, const QString);

    void commandChangeValidatorId();

    void setAccount(const Account& account);

signals:
    void stopAll();

private:
	Ui::updaterClass ui;
    //ValidatorListModel* model_;
    //ValidatorProcessUpdateProxyModel* proxy_;
    void connections();
    void init();

	/**/
	QString nameIPStartSetting = QString::fromLatin1("IPStart");
	QString nameIPEndSetting = QString::fromLatin1("IPEnd");
	QString nameLogin = QString::fromLatin1("login");
	QString namePassword = QString::fromLatin1("password");
    QString nameServiceTransactions = QString::fromLatin1("serviceTransactions");
    QString namePathUploadSoftware = QString::fromLatin1("pathUploadSoftware");
    QString namePathUploadWhitelist = QString::fromLatin1("pathUploadWhitelist");
    //QString nameTranslatorFile = QString::fromLatin1("translator");

    boost::filesystem::path folderAplication_;
    std::string folderTransactionStore_ = "transactions";
    std::string localSubFolderUpdateSoftware_ = "updates";
    QSettings *settings_;
    QString getIdValidator(const QString& login,
                           const QString& ipString,
                           const boost::filesystem::path& folder);

    /*If the process cannot be started, -2 is returned. If the process crashes, -1 is returned. Otherwise, the process' exit code is returned.*/
    int readIdValidator(const QString& login,
                        const QString& ip,
                        const QString& file,
                        const QString& fileDestination);

    QAtomicInt countQueryIp_;
    QTimer *timerDetecting_;

    bool haveError_ = false;

    void setStateFindValidator();
    void modelUpdateId(const QString& idString,
                       const QString &ipString);

    void updateInfoValidator(const QString &idValidator);
    void fillListUpdateSoftware();

    bool extractDataDevice(const QModelIndex& index,
                           QString &ipString,
                           QString &idString);
    void createTransactionProcess(const QString &login,
                                  const QString &ipString,
                                  const QString &idString);

    void createUploadSoftwareProcess(const QString &login,
                                     const QString &ipString,
                                     const QString &idString,
                                     const QString &pathUploadSoftware,
                                     const QString &pathUploadWhitelist);

    Account account_;

private slots:
    void updateStatusDetecting();
    void modelDataChanged(const QModelIndex &topLeft,
                          const QModelIndex &bottomRight,
                          const QVector<int> &roles = QVector<int>());
    void modelReseted();

    void commandDownloadUpdates();
    void updateProcessDownloadUpdates(int percent,
                                      const QString message);
    void finishedDownloadUpdates();
    void errorProcessDownloadUpdates(const  QString ip, const QString message);
};

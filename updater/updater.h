#pragma once

#include <boost/filesystem.hpp>

#include <QSettings>
#include <QtWidgets/QMainWindow>
#include <QAtomicInt>

#include "ui_updater.h"

class updater : public QMainWindow
{
	Q_OBJECT

public:
	updater(QWidget *parent = Q_NULLPTR);
    ~updater();

	public slots:
	void findValidators();
    void uploadTransactions();
    void updateListDevices(const QString ipString, const QString statusPing, const QString idValidator);
    void updateProcessTransactions(int percent, const QString message, const QString idString);
    void finishedTransactions();

    void uploadUpdate();
    void finishedUpdate();

    void showInfoValidator(const QModelIndex &index);
    void uploadTransactionToServer();
    void errorProcess(const QString idString, const QString);

    void changeValidatorId();

signals:
    void stopAll();

private:
	Ui::updaterClass ui;
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

    boost::filesystem::path folderAplication_;
    std::string folderTransactionStore_ = "transactions";
    QSettings *settings_;
    QString getIdValidator(const QString& login, const QString& ipString, const boost::filesystem::path& folder);

    /*If the process cannot be started, -2 is returned. If the process crashes, -1 is returned. Otherwise, the process' exit code is returned.*/
	int readIdValidator(const QString& login, const QString& ip, const QString& file, const QString&  fileDestination);

    QAtomicInt countQueryIp_;
    QTimer *timerDetecting_;

    bool haveError_ = false;

    void setStateFindValidator();
    void modelUpdateId(const QString& idString, const QString ipString);

    void updateInfoValidator(const QString &idValidator);
private slots:
    void updateStatusDetecting();
    void modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void modelReseted();
};

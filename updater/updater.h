#pragma once

#include <boost/filesystem.hpp>

#include <QtWidgets/QMainWindow>
#include "ui_updater.h"

class updater : public QMainWindow
{
	Q_OBJECT

public:
	updater(QWidget *parent = Q_NULLPTR);
    ~updater();

	public slots:
	void findValidators();
    void serviceValidators();
    void updateListDevices(const QString ipString, const QString statusPing, const QString idValidator);
    void showInfoValidator(const QModelIndex &index);
signals:
    void stopAll();

private:
	Ui::updaterClass ui;
	void connections();

	/**/
	QString nameIPStartSetting = QString::fromLatin1("IPStart");
	QString nameIPEndSetting = QString::fromLatin1("IPEnd");
	QString nameLogin = QString::fromLatin1("login");
	QString namePassword = QString::fromLatin1("password");
    QString getIdValidator(const QString& login, const QString& ipString, const boost::filesystem::path& folder);

    /*If the process cannot be started, -2 is returned. If the process crashes, -1 is returned. Otherwise, the process' exit code is returned.*/
	int readIdValidator(const QString& login, const QString& ip, const QString& file, const QString&  fileDestination);
};

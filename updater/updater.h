#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_updater.h"

class updater : public QMainWindow
{
	Q_OBJECT

public:
	updater(QWidget *parent = Q_NULLPTR);

	public slots:
	void findValidators();
private:
	Ui::updaterClass ui;
	void connections();

	/**/
	QString nameIPStartSetting = QString::fromLatin1("IPStart");
	QString nameIPEndSetting = QString::fromLatin1("IPEnd");
	QString nameLogin = QString::fromLatin1("login");
	QString namePassword = QString::fromLatin1("password");

	bool readIdValidator(const QString& login, const QString& ip, const QString& file);
};

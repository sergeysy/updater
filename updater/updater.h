#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_updater.h"

class updater : public QMainWindow
{
	Q_OBJECT

public:
	updater(QWidget *parent = Q_NULLPTR);

private:
	Ui::updaterClass ui;
};

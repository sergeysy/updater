#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <QString>
#include <QByteArray>

struct Account
{
    QString login_;
    QByteArray hashPassword_;
    QString name_;
};


#endif // ACCOUNT_H

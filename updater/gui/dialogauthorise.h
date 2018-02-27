#ifndef DIALOGAUTHORISE_H
#define DIALOGAUTHORISE_H

#include <QDialog>

#include "account.h"

namespace Ui {
class DialogAuthorise;
}

class DialogAuthorise : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAuthorise(QWidget *parent = 0);
    ~DialogAuthorise();

    Account getAccount() const;

    void setAccounts(const std::vector<Account> &accounts);

private:
    Ui::DialogAuthorise *ui;
    void connections();

    std::vector<Account> accounts_;

    bool visibleRetypePassword_ = false;
    void showSignUp();
    void showSignIn();
    void showSign(const QString& labelSign, const QString &textButton, const bool visible);

private slots:
    void checkInputLogin(const QString& login);
    void checkSign();
    void checkSignUp();
    void checkSignIn();


};

#endif // DIALOGAUTHORISE_H

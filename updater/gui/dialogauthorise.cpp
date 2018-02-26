#include "logger.hpp"

#include "dialogauthorise.h"
#include "ui_dialogauthorise.h"

DialogAuthorise::DialogAuthorise(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAuthorise)
{
    ui->setupUi(this);

    connections();

    showSignIn();
}

DialogAuthorise::~DialogAuthorise()
{
    delete ui;
}

void DialogAuthorise::connections()
{
    connect(ui->leLogin, &QLineEdit::textChanged, this, &DialogAuthorise::checkInputLogin, Qt::QueuedConnection);
    connect(ui->pushButton, &QPushButton::clicked, this, &DialogAuthorise::checkSign, Qt::QueuedConnection);
}

void DialogAuthorise::checkInputLogin(const QString &loginNeedCheck)
{
    const auto const_it = std::find_if(logins.cbegin(), logins.cend(),
                                       [&loginNeedCheck](const std::pair<QString, QString>& login)
    {
        return login.first == loginNeedCheck;
    });

    if(const_it != logins.cend() && const_it->second.isEmpty())
    {
        showSignUp();
    }
    else
    {
        showSignIn();
    }

    const auto visibleRetypePassword = false;
    ui->labelInfoPassword->setVisible(visibleRetypePassword);
    ui->labelInfoPassword->setEnabled(visibleRetypePassword);
}

void DialogAuthorise::checkSign()
{
    if(visibleRetypePassword_)
    {
        checkSignUp();
    }
    else
    {
        checkSignIn();
    }
}

void DialogAuthorise::checkSignUp()
{
    const auto password = ui->lePassword->text();
    const auto retypePassword = ui->leRetypePassword->text();
    if(password.isEmpty() || retypePassword.isEmpty())
    {
        return;
    }
    if(password == retypePassword)
    {
        if(!disconnect(ui->pushButton, 0, 0, 0))
        {
            std::cerr<<"cant disconnect"<<std::endl;
        }
        QDialog::accept();
    }
    else
    {
        const auto visibleRetypePassword = true;
        ui->labelInfoPassword->setVisible(visibleRetypePassword);
        ui->labelInfoPassword->setEnabled(visibleRetypePassword);

        ui->lePassword->setText(tr(""));
        ui->leRetypePassword->setText(tr(""));
    }
}

void DialogAuthorise::checkSignIn()
{
    const auto password = ui->lePassword->text();
    const auto loginNeedCheck=ui->leLogin->text();
    if(password.isEmpty() || loginNeedCheck.isEmpty())
    {
        return;
    }
    const auto const_it = std::find_if(logins.cbegin(), logins.cend(),
                                       [loginNeedCheck](const std::pair<QString, QString>& login)
    {
        return login.first == loginNeedCheck;
    });

    if(const_it != logins.cend())
    {
        if(const_it->second == password)
        {
            QDialog::accept();
            return;
        }
    }
    const auto visibleRetypePassword = true;
    ui->labelInfoPassword->setVisible(visibleRetypePassword);
    ui->labelInfoPassword->setEnabled(visibleRetypePassword);
    ui->lePassword->setText(tr(""));
}


void DialogAuthorise::showSignUp()
{
    visibleRetypePassword_ = true;

    showSign(tr("Sign Up"), visibleRetypePassword_);
}

void DialogAuthorise::showSignIn()
{
    visibleRetypePassword_ = false;
    showSign(tr("Sign In"), visibleRetypePassword_);
}

void DialogAuthorise::showSign(const QString& labelSign, const bool visible)
{
    ui->labelSign->setText(labelSign);

    ui->labelRetypePassword->setVisible(visible);
    ui->labelRetypePassword->setEnabled(visible);

    ui->leRetypePassword->setVisible(visible);
    ui->leRetypePassword->setEnabled(visible);
    ui->iconShowPass2->setVisible(visible);
    ui->iconShowPass2->setEnabled(visible);

    ui->labelInfoPassword->setVisible(visible);
    ui->labelInfoPassword->setEnabled(visible);
}


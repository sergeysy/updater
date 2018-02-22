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

void DialogAuthorise::checkSignUp()
{
    if(ui->lePassword->text() == ui->leRetypePassword->text())
    {
        done(QDialog::Accepted);
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
    const auto loginNeedCheck=ui->leLogin->text();
    const auto const_it = std::find_if(logins.cbegin(), logins.cend(),
                                       [loginNeedCheck](const std::pair<QString, QString>& login)
    {
        return login.first == loginNeedCheck;
    });

    if(const_it != logins.cend())
    {
        if(const_it->second == ui->lePassword->text())
        {
            done(QDialog::Accepted);
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
    const auto visibleRetypePassword = true;

    showSign(tr("Sign Up"), visibleRetypePassword);

    disconnect(ui->pushButton, &QPushButton::clicked, 0, 0);
    connect(ui->pushButton, &QPushButton::clicked, this, &DialogAuthorise::checkSignUp, Qt::QueuedConnection);
}

void DialogAuthorise::showSignIn()
{
    const auto visibleRetypePassword = false;
    showSign(tr("Sign In"), visibleRetypePassword);
    disconnect(ui->pushButton, &QPushButton::clicked, 0, 0);
    connect(ui->pushButton, &QPushButton::clicked, this, &DialogAuthorise::checkSignIn, Qt::QueuedConnection);
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

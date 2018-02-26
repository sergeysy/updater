#include "authorise.h"
#include "ui_authorise.h"

Authorise::Authorise(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Authorise)
{
    ui->setupUi(this);
    connections();

    showSignIn();
    //showIncorectedPassword(false);
}

Authorise::~Authorise()
{
    delete ui;
}

void Authorise::connections()
{
    connect(ui->pbSiginIn, &QPushButton::clicked, this, &Authorise::accept);
    connect(ui->leLogin_2, &QLineEdit::textChanged, this, &Authorise::loginChanged);
}

void Authorise::showSign(const bool visible, const QString& textHeader, const QString& textButton)
{
    ui->leRetypePassword_2->setEnabled(visible);
    ui->leRetypePassword_2->setVisible(visible);

    ui->lbRetypePassword->setEnabled(visible);
    ui->lbRetypePassword->setVisible(visible);
    ui->lbIconRetypePassword->setEnabled(visible);
    ui->lbIconRetypePassword->setVisible(visible);



    ui->lbSign->setText(textHeader);
    ui->pbSiginIn->setText(textButton);
}

void Authorise::showIncorectedPassword(const bool visible)
{
    ui->labelIncorectedPassword->setEnabled(visible);
    ui->labelIncorectedPassword->setVisible(visible);
}

void Authorise::showSignIn()
{
    showSign(false, tr("Sign In"), tr("Sign In"));
}

void Authorise::showSignUp()
{
    showSign(true, tr("Sign Up"), tr("Sign Up"));
}

void Authorise::accept()
{
    QDialog::accept();
}

void Authorise::loginChanged(const QString &login)
{return;
    const auto const_it_account = std::find_if(accounts_.cbegin(), accounts_.cend(), [login](const std::pair<QString, QString> account)
    {
        return account.first == login;
    });

    if(const_it_account!= accounts_.cend() && const_it_account->second.isEmpty())
    {
        showSignUp();
        showIncorectedPassword(false);
    }
    else
    {
        showSignIn();
    }
}

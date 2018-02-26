#ifndef AUTHORISE_H
#define AUTHORISE_H

#include <vector>
#include <QDialog>

namespace Ui {
class Authorise;
}

class Authorise : public QDialog
{
    Q_OBJECT

public:
    explicit Authorise(QWidget *parent = 0);
    ~Authorise();

private:
    Ui::Authorise *ui;
    void connections();
    void showSign(const bool visible, const QString& textHeader, const QString& textButton);
    void showSignIn();
    void showSignUp();
    void showIncorectedPassword(const bool visible);

    std::vector<std::pair<QString, QString>> accounts_;
    /*enum Sign
    {
        SignIn,
        SignUp,
    };

    Sign typeSign_ = Sign::SignIn;*/

public slots:
    void accept() override;

private slots:
    void loginChanged(const QString& login);

};

#endif // AUTHORISE_H

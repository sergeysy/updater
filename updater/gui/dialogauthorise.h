#ifndef DIALOGAUTHORISE_H
#define DIALOGAUTHORISE_H

#include <QDialog>

namespace Ui {
class DialogAuthorise;
}

class DialogAuthorise : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAuthorise(QWidget *parent = 0);
    ~DialogAuthorise();

private:
    Ui::DialogAuthorise *ui;
    void connections();

    std::vector<std::pair<QString, QString>> logins={{tr("root"),tr("")}};

    void showSignUp();
    void showSignIn();
    void showSign(const QString& labelSign, const bool visible);

private slots:
    void checkInputLogin(const QString& login);
    void checkSignUp();
    void checkSignIn();

};

#endif // DIALOGAUTHORISE_H

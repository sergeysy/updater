#ifndef CHANGEIDDIALOG_H
#define CHANGEIDDIALOG_H

#include <QDialog>
#include <QString>
#include <QIntValidator>

namespace Ui {
class ChangeIdDialog;
}

class ChangeIdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeIdDialog(QWidget *parent = 0);
    ~ChangeIdDialog();

    void setEditText(const QString& text);
    QString text() const;

private:
    Ui::ChangeIdDialog *ui;
    QIntValidator* intValidator_;
};

#endif // CHANGEIDDIALOG_H

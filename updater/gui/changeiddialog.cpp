#include "changeiddialog.h"
#include "ui_changeiddialog.h"

ChangeIdDialog::ChangeIdDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeIdDialog)
{
    ui->setupUi(this);
    intValidator_ = new QIntValidator(0, 64536, this);
    ui->lineEdit->setValidator(intValidator_);
}

void ChangeIdDialog::setEditText(const QString& text)
{
    QString temp = text;
    int pos = 0;
    if(intValidator_->validate(temp, pos) == QValidator::Acceptable)
    {
        ui->lineEdit->setText(text);
    }
}

QString ChangeIdDialog::text() const
{
    return ui->lineEdit->text();
}
ChangeIdDialog::~ChangeIdDialog()
{
    delete ui;
}

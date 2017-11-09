#include "mainwindow.h"
#include <QApplication>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if(argc!=2)
    {
        return -1;
    }
    MainWindow w;
    w.setText(QString::fromLatin1(argv[1]));
    w.show();

    return a.exec();
}

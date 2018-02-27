#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <memory>

#include <boost/filesystem.hpp>

#include <QObject>
#include <QSettings>
#include <QString>
#include <QApplication>
#include <QTranslator>

#include "logger.hpp"

class Translator : QObject
{
    Q_OBJECT
public:
    Translator(QObject *parent = nullptr);
    QSettings* settings() const;
private:
    std::shared_ptr<QTranslator> myTranslator;
    boost::filesystem::path folderAplication_;
    QSettings* settings_;
    QString nameTranslatorFile = QString::fromLatin1("translator");

    void loadTranslate();
};
#endif // TRANSLATOR_H

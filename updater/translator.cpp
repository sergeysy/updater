#include "translator.h"

Translator::Translator(QObject *parent) : QObject(parent)
{
    const auto argsApplication = QCoreApplication::arguments();

    const auto pathBinaryFile = argsApplication.front();

    boost::filesystem::path full_path = boost::filesystem::system_complete(boost::filesystem::path(pathBinaryFile.toStdString().c_str()));
    folderAplication_ = full_path.parent_path();
    const auto pathSettingsFile = folderAplication_/"settings.ini";//full_path.replace_extension("ini");
    std::cerr << logger() << "File settings opening..." << std::endl;
    settings_ = new QSettings(QString::fromStdString(pathSettingsFile.string()), QSettings::Format::IniFormat, this);
    std::cerr << logger() << "File settings opened" << std::endl;

    loadTranslate();
}

QSettings *Translator::settings() const
{
    return settings_;
}

void Translator::loadTranslate()
{
    myTranslator = std::make_shared<QTranslator>();
    std::cerr << logger() << "Locale: " << QLocale::system().name().toStdString() << " .Load traslator file: " <<settings_->value(nameTranslatorFile).toString().toStdString()
              << " from " << folderAplication_.string() <<std::endl;
    if(!myTranslator->load(settings_->value(nameTranslatorFile).toString(), QString::fromStdString(folderAplication_.string())))
    {
        std::cerr << logger() << "Not loaded file translator" << std::endl;
        return;
    }
    std::cerr << logger() << "File translator was loaded" << std::endl;

    QCoreApplication::instance()->installTranslator(myTranslator.get());
    QApplication::instance()->processEvents();
}

#include <string>

#include <boost/filesystem.hpp>

#include <QTextCodec>
#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QCryptographicHash>

#include "account.h"
#include "logger.hpp"
#include "translator.h"
#include "gui/dialogauthorise.h"
#include "gui/authorise.h"
#include "updater.h"

class ManagerLogger
{
public:
    ManagerLogger(const boost::filesystem::path& path)
    {
		freopen((path/"1.log").string().c_str(), "w", stdout);
        freopen((path/"2.log").string().c_str(), "w", stderr);
	}
	~ManagerLogger()
	{
		fclose(stdout);
		fclose(stderr);
	}
};

void saveHashPassword(QSettings* settings, const QString& login, const QByteArray& hashPassword)
{

    const auto groups = settings->childGroups();
    const auto it = std::find_if(groups.cbegin(), groups.cend(),
                 [&login, &settings](const QString& group)
    {
        settings->beginGroup(group);
        const auto isFounded = settings->value(QString::fromLatin1("login"))==login;
        settings->endGroup();
        return isFounded;
    });

    if(it != groups.cend())
    {
        settings->beginGroup(*it);
        settings->setValue(QString::fromLatin1("password"), QVariant::fromValue(hashPassword));
        settings->endGroup();
    }

}

std::vector<Account> loadAccounts(QSettings* settings)
{
    {
        QTextCodec * codec = QTextCodec::codecForName("KOI8-R");
        if(nullptr != codec)
        {
            QTextCodec::setCodecForLocale(codec);
        }
    }
settings->setIniCodec(QTextCodec::codecForLocale());
    std::vector<Account> result;
    const auto groups = settings->childGroups();
    for(const auto group:groups)
    {
        settings->beginGroup(group);
        const auto value = settings->value(QString::fromLatin1("login")).toString();
        const auto isFounded = !value.isEmpty();

        if(isFounded)
        {
            const auto val =
                        QString::fromUtf8(settings->value(QString::fromLatin1("name")).toByteArray());
            auto name=QString::fromUtf8(QTextCodec::codecForLocale()->fromUnicode(val));
            result.push_back({
                        settings->value(QString::fromLatin1("login")).toString(),
                        settings->value(QString::fromLatin1("password")).toByteArray(),
                        name
                        }
                        );

        }
        settings->endGroup();

    }

    {
        QTextCodec * codec = QTextCodec::codecForName("UTF-8");
        if(nullptr != codec)
        {
            QTextCodec::setCodecForLocale(codec);
        }
    }
    return result;
}

int main(int argc, char *argv[])
{
    ManagerLogger logger(boost::filesystem::path(argv[0]).parent_path());

    // For correct UTF-8 strings interpretation.
    QTextCodec * codec = QTextCodec::codecForName("UTF-8");
    if(nullptr != codec)
    {
        QTextCodec::setCodecForLocale(codec);
    }

	QApplication a(argc, argv);
    Translator translator;
    auto* settings = translator.settings();

    updater mainWindow;
    DialogAuthorise dlg;
    const auto accounts = loadAccounts(settings);
    dlg.setAccounts(accounts);
    const auto result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        const auto account = dlg.getAccount();
        saveHashPassword(settings, account.login_, account.hashPassword_);

        mainWindow.setAccount(account);
        mainWindow.show();
        return a.exec();
    }

    return -1;
}


#include <QThread>
#include <QPushButton>
#include <QSettings>
#include <QProcess>
#include <QString>
#include <QHostAddress>

#include "detectorvalidator.hpp"
#include "model/validatorlistmodel.hpp"
#include "logger.hpp"
#include "updater.h"

updater::updater(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connections();
    auto model = new ValidatorListModel(this);

    ui.listView->setModel(model);
}

void updater::connections()
{
    connect(ui.pbFindValidators, &QPushButton::clicked, this, &updater::findValidators, Qt::QueuedConnection);
    connect(ui.pbServiceValidators, &QPushButton::clicked, this, &updater::serviceValidators, Qt::QueuedConnection);
    connect(ui.listView, &QListView::clicked, this, &updater::showInfoValidator, Qt::QueuedConnection);
}

updater::~updater()
{
    emit stopAll();
}


void updater::findValidators()
{
	const auto argsApplication = QCoreApplication::arguments();

	const auto pathBinaryFile = argsApplication.front();

	//boost::filesystem::path full_path(boost::filesystem::initial_path<boost::filesystem::path>());

	boost::filesystem::path full_path = boost::filesystem::system_complete(boost::filesystem::path(pathBinaryFile.toStdString().c_str()));
	const auto folder = full_path.parent_path();
	const auto pathSettingsFile = folder/"settings.ini";//full_path.replace_extension("ini");
    std::cerr << logger() << "File settings opening..." << std::endl;
    QSettings settings(QString::fromStdString(pathSettingsFile.string()), QSettings::Format::IniFormat);
    std::cerr << logger() << "File settings opened" << std::endl;
	const auto IPStartSetting = settings.value(nameIPStartSetting, QString::fromLatin1("127.0.0.1")).toString();
	const auto IPEndSetting = settings.value(nameIPEndSetting, QString::fromLatin1("127.0.0.1")).toString();

    std::cerr << logger() << "IPStartSetting="<<IPStartSetting.toStdString() << " IPEndSetting=" << IPEndSetting.toStdString() << std::endl;

	const auto login = settings.value(nameLogin, QString::fromLatin1("root")).toString();
	
    auto ipStart = QHostAddress(IPStartSetting).toIPv4Address();
    auto ipEnd = QHostAddress(IPEndSetting).toIPv4Address();
    if(ipStart > ipEnd)
    {
        std::swap(ipEnd, ipStart);
    }

    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    model->removeRows(0, model->rowCount(QModelIndex()));
    for(auto ip = ipStart; ip <= ipEnd; ++ip)
    {
        const auto ipString = QHostAddress(ip).toString();
        DetectorValidator *device = new DetectorValidator(nullptr);
        device->setLogin(login).setIp(ipString).setPath(folder);

        QThread* thread = new QThread;
        device->moveToThread(thread);

        connect(thread, &QThread::started, device, &DetectorValidator::process, Qt::QueuedConnection);
        connect(device, &DetectorValidator::haveData, this, &updater::updateListDevices, Qt::QueuedConnection);
        connect(device, &DetectorValidator::finished, thread, &QThread::quit, Qt::QueuedConnection);
        connect(this, &updater::stopAll, device, &DetectorValidator::stop, Qt::QueuedConnection);
        connect(device, &DetectorValidator::finished, device, &DetectorValidator::deleteLater, Qt::QueuedConnection);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);

        thread->start();
    }
}

void updater::serviceValidators()
{

}

void updater::updateListDevices(const QString ipString, const QString statusPing, const QString idValidator)
{
    std::ignore = statusPing;
    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    model->addDevice(Validator(ipString, idValidator));
}

void updater::showInfoValidator(const QModelIndex &index)
{
    const auto ipString = index.data(ValidatorListModel::deviceRole::IPRole).toString();
}



#include <boost/range/iterator_range.hpp>

#include <QTimer>
#include <QThread>
#include <QPushButton>
#include <QProcess>
#include <QString>
#include <QHostAddress>

#include "facadeStorageTransactions.hpp"
#include "transport.hpp"

#include "detectorvalidator.hpp"
#include "model/validatorlistmodel.hpp"
#include "logger.hpp"
#include "updater.h"

updater::updater(QWidget *parent)
	: QMainWindow(parent)
    , countQueryIp_(0)
{
    init();
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
    connect(ui.pbUploadTransactionToServer, &QPushButton::clicked, this, &updater::uploadTransactionToServer, Qt::QueuedConnection);

    timerDetecting_ = new QTimer(this);
    const int intervalUpdateMSec = 100;
    timerDetecting_->setInterval(intervalUpdateMSec);
    connect(timerDetecting_, &QTimer::timeout, this, &updater::updateStatusDetecting);
}

void updater::init()
{
    const auto argsApplication = QCoreApplication::arguments();

    const auto pathBinaryFile = argsApplication.front();

    boost::filesystem::path full_path = boost::filesystem::system_complete(boost::filesystem::path(pathBinaryFile.toStdString().c_str()));
    folderAplication_ = full_path.parent_path();
    const auto pathSettingsFile = folderAplication_/"settings.ini";//full_path.replace_extension("ini");
    std::cerr << logger() << "File settings opening..." << std::endl;
    settings_ = new QSettings(QString::fromStdString(pathSettingsFile.string()), QSettings::Format::IniFormat, this);
    std::cerr << logger() << "File settings opened" << std::endl;
}

updater::~updater()
{
    emit stopAll();
}


void updater::findValidators()
{
    const auto IPStartSetting = settings_->value(nameIPStartSetting, QString::fromLatin1("127.0.0.1")).toString();
    const auto IPEndSetting = settings_->value(nameIPEndSetting, QString::fromLatin1("127.0.0.1")).toString();

    std::cerr << logger() << "IPStartSetting="<<IPStartSetting.toStdString() << " IPEndSetting=" << IPEndSetting.toStdString() << std::endl;

    const auto login = settings_->value(nameLogin, QString::fromLatin1("root")).toString();
	
    auto ipStart = QHostAddress(IPStartSetting).toIPv4Address();
    auto ipEnd = QHostAddress(IPEndSetting).toIPv4Address();
    if(ipStart > ipEnd)
    {
        std::swap(ipEnd, ipStart);
    }

    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    model->removeRows(0, model->rowCount(QModelIndex()));

    emit stopAll();
    countQueryIp_ = 0;
    for(auto ip = ipStart; ip <= ipEnd; ++ip)
    {
        const auto ipString = QHostAddress(ip).toString();
        DetectorValidator *device = new DetectorValidator;
        device->setLogin(login).setIp(ipString).setPath(folderAplication_);

        QThread* thread = new QThread;
        device->moveToThread(thread);

        connect(thread, &QThread::started, device, &DetectorValidator::process, Qt::QueuedConnection);
        connect(device, &DetectorValidator::haveData, this, &updater::updateListDevices, Qt::QueuedConnection);
        connect(device, &DetectorValidator::finished, thread, &QThread::quit, Qt::QueuedConnection);
        connect(this, &updater::stopAll, device, &DetectorValidator::stop, Qt::QueuedConnection);
        connect(device, &DetectorValidator::finished, device, &DetectorValidator::deleteLater, Qt::QueuedConnection);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);

        thread->start();
        ++countQueryIp_;
    }
    timerDetecting_->start();
}

void updater::serviceValidators()
{
    const auto login = settings_->value(nameLogin, QString::fromLatin1("root")).toString();
    //TODO get folder from settings of validator
    const QString folderTransactionsOnValidator(QString::fromLatin1("/mnt/sda/transaction"));
    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    for(int i = 0; i < model->rowCount(QModelIndex()); ++i)
    {
        const auto ipString = model->index(i).data(ValidatorListModel::deviceRole::IPRole).toString();
        const auto idString = model->index(i).data(ValidatorListModel::deviceRole::IdRole).toString();
        bool isValidId = false;
        idString.toLong(&isValidId, 10);
        if(!isValidId)
        {
            std::cerr << logger() << "Skip get transactions from " << ipString.toStdString() << std::endl;
            continue;
        }
        const auto pathDestination = folderAplication_/folderTransactionStore_/idString.toStdString();
        const QString folderDestination(QString::fromStdString(pathDestination.string()));
        Transactions *transactionProcess = new Transactions(login, ipString, idString, folderTransactionsOnValidator, folderDestination);


        QThread* thread = new QThread;
        transactionProcess->moveToThread(thread);

        connect(thread, &QThread::started, transactionProcess, &Transactions::process, Qt::QueuedConnection);
        connect(transactionProcess, &Transactions::error, this, &updater::errorProcessTransactions, Qt::QueuedConnection);
        connect(transactionProcess, &Transactions::finished, thread, &QThread::quit, Qt::QueuedConnection);
        connect(this, &updater::stopAll, transactionProcess, &Transactions::stop, Qt::QueuedConnection);
        connect(transactionProcess, &Transactions::finished, transactionProcess, &Transactions::deleteLater, Qt::QueuedConnection);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);

        thread->start();
    }
}

void updater::updateListDevices(const QString ipString, const QString statusPing, const QString idValidator)
{
    std::ignore = statusPing;
    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    model->addDevice(Validator(ipString, idValidator));

    --countQueryIp_;
    if( 0 == countQueryIp_)
    {
        timerDetecting_->stop();
        ui.labelStatusDetect->setText(tr("Validators detected"));
    }
}

void updater::showInfoValidator(const QModelIndex &index)
{
    const auto ipString = index.data(ValidatorListModel::deviceRole::IPRole).toString();
    const auto idString = index.data(ValidatorListModel::deviceRole::IdRole).toString();

    ui.labelIdValidatorValue->setText(idString);
}

void updater::uploadTransactionToServer()
{
    auto folders = folderAplication_/folderTransactionStore_;
    auto path = boost::filesystem::path(folders);
    const auto serviceTransactions = settings_->value(nameServiceTransactions).toString().toStdString();
    try
    {
        if(boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
        {
            for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(path), {}))
            {
                if(boost::filesystem::is_directory(entry))
                {
                    FacadeStorageTransaction storage(entry.path().string());
                    const auto transactions = storage.getTransactions();
                    if(!transactions.empty())
                    {
                        std::string tsvTransactions;
                        for(const auto& transaction : transactions)
                        {
                            tsvTransactions += transaction;
                        }
                        if(tsvTransactions.back()=='\n')
                        {
                            tsvTransactions.pop_back();
                        }
                        try
                        {
                            Transport::TransportImpl transport;
                            // if posthttp code not 200, method throw
                            const auto result = transport.postTsvHttp(serviceTransactions, tsvTransactions);
                            std::cerr << logger() << "Result sent transactions: " << result << std::endl;

                            storage.markTransactionAsSent();
                        }
                        catch(const std::exception& ex)
                        {
                            std::cerr << logger() << "Error: " << ex.what() << std::endl;
                        }
                    }
                }
            }
        }
    }
    catch(const boost::filesystem::filesystem_error& ex)
    {
        std::cerr << logger() << "Error: " << ex.what() << std::endl;
    }
}

void updater::errorProcessTransactions(const QString message)
{
    ui.labelStatusDetect->setText(message);
}

void updater::updateStatusDetecting()
{
    if(countQueryIp_>0)
    {
        QString status(tr("Detecting validators"));
        static int i = 0;
        i++;
        switch (i%4) {
        case 1:
            status += QString::fromLatin1(".");
            break;
        case 2:
            status += QString::fromLatin1("..");
            break;
        case 3:
            status += QString::fromLatin1("...");
            break;
        default:
            break;
        }
        ui.labelStatusDetect->setText(status);
    }
}

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
    ValidatorProcessUpdateProxyModel* proxy = new ValidatorProcessUpdateProxyModel(this);
    proxy->setSourceModel(model);
    ui.lvStatusValidator->setModel(proxy);
    connect(model, &ValidatorListModel::dataChanged, proxy, &ValidatorProcessUpdateProxyModel::dataChanged,Qt::QueuedConnection);
    connect(model, &ValidatorListModel::rowsInserted, proxy, &ValidatorProcessUpdateProxyModel::rowsInserted,Qt::QueuedConnection);
}

void updater::connections()
{
    connect(ui.pbProcessValidators, &QPushButton::clicked, this, &updater::findValidators, Qt::QueuedConnection);
    //connect(ui.pbServiceValidators, &QPushButton::clicked, this, &updater::serviceValidators, Qt::QueuedConnection);
    connect(ui.listView, &QListView::clicked, this, &updater::showInfoValidator, Qt::QueuedConnection);
    connect(ui.pbUploadTransactionToServer, &QPushButton::clicked, this, &updater::uploadTransactionToServer, Qt::QueuedConnection);

    timerDetecting_ = new QTimer(this);
    const int intervalUpdateMSec = 100;
    timerDetecting_->setInterval(intervalUpdateMSec);
    connect(timerDetecting_, &QTimer::timeout, this, &updater::updateStatusDetecting);
    ui.cbUploadSoftware->setVisible(false);
    ui.cbUploadWhitelist->setVisible(false);
    ui.lvStatusValidator->setVisible(false);
    ui.infoValidatorWidget->setVisible(false);
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
    disconnect(ui.pbProcessValidators, &QPushButton::clicked, 0, 0);
    ui.pbProcessValidators->setEnabled(false);
    ui.pbProcessValidators->setText(tr("Load"));
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
        countQueryIp_.fetchAndAddAcquire(1);
    }
    timerDetecting_->start();
}

void updater::uploadTransactions()
{
    ui.pbProcessValidators->setEnabled(false);

    const auto login = settings_->value(nameLogin, QString::fromLatin1("root")).toString();
    //TODO get folder from settings of validator
    const QString folderTransactionsOnValidator(QString::fromLatin1("/mnt/sda/transaction"));
    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    int i;
    for(i = 0; i < model->rowCount(QModelIndex()); ++i)
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
        connect(transactionProcess, &Transactions::error, this, &updater::errorProcess, Qt::QueuedConnection);
        connect(transactionProcess, &Transactions::updateProcess, this, &updater::updateProcessTransactions, Qt::QueuedConnection);
        connect(transactionProcess, &Transactions::finished, thread, &QThread::quit, Qt::QueuedConnection);
        connect(transactionProcess, &Transactions::finished, this, &updater::finishedTransactions, Qt::QueuedConnection);
        connect(this, &updater::stopAll, transactionProcess, &Transactions::stop, Qt::QueuedConnection);
        connect(transactionProcess, &Transactions::finished, transactionProcess, &Transactions::deleteLater, Qt::QueuedConnection);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);

        thread->start();
        countQueryIp_.fetchAndAddAcquire(1);
    }

    if(countQueryIp_.fetchAndAddAcquire(0) == 0)
    {
        setStateFindValidator();
        errorProcess(tr(""), tr("Not found validators"));
    }
    else
    {
        disconnect(ui.pbProcessValidators, &QPushButton::clicked, 0, 0);
        ui.pbProcessValidators->setText(tr("Process transactions"));

        {
            ui.lvStatusValidator->setVisible(true);
        }
    }
}

void updater::updateProcessTransactions(int percent, const QString idString)
{
    //auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    auto model = ui.listView->model();
    if(model == nullptr)
    {
        return;
    }
    auto list = model->match(model->index(0, 0), ValidatorListModel::deviceRole::UpdatePercentJobRole, idString);
    for(auto& item : list)
    {
        model->setData(item, percent, ValidatorListModel::deviceRole::UpdatePercentJobRole);
    }
}

void updater::updateListDevices(const QString ipString, const QString statusPing, const QString idValidator)
{
    std::ignore = statusPing;
    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    model->addDevice(Validator(ipString, idValidator));

    const auto value = countQueryIp_.fetchAndAddAcquire(-1);
    if( 1 == value)
    {
        timerDetecting_->stop();
        ui.labelStatusDetect->setText(tr("Validators detected"));

        connect(ui.pbProcessValidators, &QPushButton::clicked, this, &updater::uploadTransactions, Qt::QueuedConnection);
        ui.pbProcessValidators->setEnabled(true);
        ui.pbProcessValidators->setText(tr("Process transactions"));
        ui.pbProcessValidators->adjustSize();

    }
}

void updater::finishedTransactions()
{
    const auto value = countQueryIp_.fetchAndAddAcquire(-1);
    if( 1 == value)
    {
        ui.labelStatusDetect->setText(tr("Successful process transactions"));

        connect(ui.pbProcessValidators, &QPushButton::clicked, this, &updater::uploadUpdate, Qt::QueuedConnection);
        ui.pbProcessValidators->setEnabled(true);
        ui.pbProcessValidators->setText(tr("Upadate"));
        ui.pbProcessValidators->adjustSize();
        ui.cbUploadSoftware->setVisible(true);
        ui.cbUploadWhitelist->setVisible(true);
    }
}

void updater::uploadUpdate()
{
    disconnect(ui.pbProcessValidators, &QPushButton::clicked, 0, 0);
    ui.pbProcessValidators->setEnabled(false);
    ui.pbProcessValidators->setText(tr("Process upload"));

    {
        ui.lvStatusValidator->setVisible(false);
    }
    const auto isNeedUpdateSoftware = ui.cbUploadSoftware->isChecked();
    const auto isNeedUploadWhitelist = ui.cbUploadWhitelist->isChecked();
    ui.cbUploadSoftware->setVisible(false);
    ui.cbUploadWhitelist->setVisible(false);

    const auto login = settings_->value(nameLogin, QString::fromLatin1("root")).toString();
    //TODO get folder from settings of validator
    const QString folderTransactionsOnValidator(QString::fromLatin1("/mnt/sda/transaction"));
    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    QString pathUploadSoftware;
    if(isNeedUpdateSoftware)
    {
        pathUploadSoftware = settings_->value(namePathUploadSoftware).toString();
    }

    QString pathUploadWhitelist;
    if(isNeedUploadWhitelist)
    {
        pathUploadWhitelist = settings_->value(namePathUploadWhitelist).toString();
    }
    std::cerr << logger() << "Path update software " << pathUploadSoftware.toStdString() << std::endl;
    std::cerr << logger() << "Path update whitelist " << pathUploadWhitelist.toStdString() << std::endl;
    std::cerr << logger() << "Setting keys: " << settings_->allKeys().join(tr(" ")).toStdString() << std::endl;
    haveError_ = false;
    const auto percent = 0;
    for(int i = 0; i < model->rowCount(QModelIndex()); ++i)
    {
        model->setData(model->index(i), percent, ValidatorListModel::deviceRole::UpdatePercentJobRole);
        const auto ipString = model->index(i).data(ValidatorListModel::deviceRole::IPRole).toString();
        const auto idString = model->index(i).data(ValidatorListModel::deviceRole::IdRole).toString();
        bool isValidId = false;
        idString.toLong(&isValidId, 10);
        if(!isValidId)
        {
            std::cerr << logger() << "Skip upload to " << ipString.toStdString() << std::endl;
            continue;
        }
        Upload *transactionProcess = new Upload(login, ipString, idString, pathUploadSoftware, pathUploadWhitelist);

        QThread* thread = new QThread;
        transactionProcess->moveToThread(thread);

        connect(thread, &QThread::started, transactionProcess, &Upload::process, Qt::QueuedConnection);
        connect(transactionProcess, &Upload::error, this, &updater::errorProcess, Qt::QueuedConnection);
        connect(transactionProcess, &Upload::finished, thread, &QThread::quit, Qt::QueuedConnection);
        connect(transactionProcess, &Upload::finished, this, &updater::finishedUpdate, Qt::QueuedConnection);
        connect(this, &updater::stopAll, transactionProcess, &Upload::stop, Qt::QueuedConnection);
        connect(transactionProcess, &Upload::finished, transactionProcess, &Upload::deleteLater, Qt::QueuedConnection);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);

        thread->start();
        countQueryIp_.fetchAndAddAcquire(1);
    }
}

void updater::setStateFindValidator()
{
    connect(ui.pbProcessValidators, &QPushButton::clicked, this, &updater::findValidators, Qt::QueuedConnection);
    ui.pbProcessValidators->setEnabled(true);
    ui.pbProcessValidators->setText(tr("Find validators"));
    ui.pbProcessValidators->adjustSize();
}

void updater::finishedUpdate()
{
    const auto value = countQueryIp_.fetchAndAddAcquire(-1);
    if( 1 == value)
    {
        if(!haveError_)
        {
            ui.labelStatusDetect->setText(tr("Update finished"));
        }

        setStateFindValidator();
    }
}

void updater::showInfoValidator(const QModelIndex &index)
{
    ui.infoValidatorWidget->setVisible(true);
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

void updater::errorProcess(const QString idString, const QString message)
{
    std::ignore = idString;
    ui.labelStatusDetect->setText(message);
    haveError_ = true;
}

void updater::updateStatusDetecting()
{
    if(countQueryIp_.fetchAndAddAcquire(0)>0)
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

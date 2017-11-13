#include <fstream>

#include <boost/range/iterator_range.hpp>

#include <QTimer>
#include <QThread>
#include <QPushButton>
#include <QProcess>
#include <QString>
#include <QHostAddress>
#include <QJsonObject>
#include <QPair>
#include <QJsonDocument>

#include <ZipFile.h>
#include <streams/memstream.h>

#include "facadeStorageTransactions.hpp"
#include "transport.hpp"

#include "commands/commands.hpp"
#include "gui/changeiddialog.h"
#include "detectorvalidator.hpp"
#include "logger.hpp"
#include "updater.h"

updater::updater(QWidget *parent)
	: QMainWindow(parent)
    , countQueryIp_(0)
{
    init();
	ui.setupUi(this);
    model_ = new ValidatorListModel(this);

    ui.listView->setModel(model_);
    proxy_ = new ValidatorProcessUpdateProxyModel(this);
    proxy_->setSourceModel(model_);
    ui.lvStatusValidator->setModel(proxy_);
    connections();

    fillListUpdateSoftware();
}

void updater::connections()
{
    connect(ui.pbProcessValidators, &QPushButton::clicked, this, &updater::commnadFindValidators, Qt::QueuedConnection);
    //connect(ui.pbServiceValidators, &QPushButton::clicked, this, &updater::serviceValidators, Qt::QueuedConnection);
    connect(ui.listView, &QListView::clicked, this, &updater::showInfoValidator, Qt::QueuedConnection);
    connect(ui.pbUploadTransactionToServer, &QPushButton::clicked, this, &updater::commandUploadTransactionToServer, Qt::QueuedConnection);

    timerDetecting_ = new QTimer(this);
    const int intervalUpdateMSec = 100;
    timerDetecting_->setInterval(intervalUpdateMSec);
    connect(timerDetecting_, &QTimer::timeout, this, &updater::updateStatusDetecting);
    ui.lvStatusValidator->setVisible(false);
    ui.infoValidatorWidget->setVisible(false);

    connect(ui.pbChangeId, &QPushButton::clicked, this, &updater::commandChangeValidatorId, Qt::QueuedConnection);
    connect(model_, &ValidatorListModel::dataChanged, proxy_, &ValidatorProcessUpdateProxyModel::dataChanged,Qt::QueuedConnection);
    connect(model_, &ValidatorListModel::dataChanged, this, &updater::modelDataChanged,Qt::QueuedConnection);
    connect(model_, &ValidatorListModel::modelReset, this, &updater::modelReseted,Qt::QueuedConnection);
    connect(model_, &ValidatorListModel::modelReset, proxy_, &ValidatorProcessUpdateProxyModel::modelReset, Qt::QueuedConnection);
    connect(model_, &ValidatorListModel::rowsInserted, proxy_, &ValidatorProcessUpdateProxyModel::rowsInserted, Qt::QueuedConnection);

    connect(ui.pbDownloadUpdates, &QPushButton::clicked, this, &updater::commandDownloadUpdates, Qt::QueuedConnection);
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

    loadTranslate();
}

updater::~updater()
{
    emit stopAll();
}


void updater::commnadFindValidators()
{
    disconnect(ui.pbProcessValidators, &QPushButton::clicked, 0, 0);
    ui.pbProcessValidators->setEnabled(false);
    ui.pbProcessValidators->setText(tr("Load"));
    ui.lvStatusValidator->setVisible(false);

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
    model->clear();


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

void updater::commandUploadTransactions()
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

void updater::updateProcessTransactions(int percent, const QString message, const QString ipString)
{
    //auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    auto model = ui.listView->model();
    if(model == nullptr)
    {
        return;
    }
    QJsonObject jsonObject;
    jsonObject.insert(tr("message"), message);
    jsonObject.insert(tr("percent"), percent);
    jsonObject.insert(tr("ip"), ipString);

    QJsonDocument doc(jsonObject);
    QString strJson(QString::fromLatin1(doc.toJson(QJsonDocument::Compact)));
    //std::cerr << logger() << "json: " << strJson.toStdString() << std::endl;

    auto list = model->match(model->index(0, 0), ValidatorListModel::deviceRole::IPRole, ipString);
    for(auto& item : list)
    {
        model->setData(item, jsonObject, ValidatorListModel::deviceRole::UpdatePercentJobRole);
    }
}

void updater::updateListDevices(const QString ipString, const QJsonObject data)
{
    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    model->addDevice(Validator(ipString, data));

    const auto value = countQueryIp_.fetchAndAddAcquire(-1);
    if( 1 == value)
    {
        timerDetecting_->stop();
        ui.labelStatusDetect->setText(tr("Validators detected"));

        connect(ui.pbProcessValidators, &QPushButton::clicked, this, &updater::commandUploadTransactions, Qt::QueuedConnection);
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

        connect(ui.pbProcessValidators, &QPushButton::clicked, this, &updater::commandUpdateValidator, Qt::QueuedConnection);
        ui.pbProcessValidators->setEnabled(true);
        ui.pbProcessValidators->setText(tr("Upadate"));
        ui.pbProcessValidators->adjustSize();
    }
}

void updater::commandUpdateValidator()
{
    disconnect(ui.pbProcessValidators, &QPushButton::clicked, 0, 0);
    ui.pbProcessValidators->setEnabled(false);
    ui.pbProcessValidators->setText(tr("Process upload"));

    {
        //ui.lvStatusValidator->setVisible(false);
    }
    const auto isNeedUpdateSoftware = ui.comboBoxUpdateSoftware->currentIndex()!=-1;
    const auto isNeedUploadWhitelist = ui.comboBoxUpdateWhitelist->currentIndex()!=-1;

    const auto login = settings_->value(nameLogin, QString::fromLatin1("root")).toString();
    //TODO get folder from settings of validator
    const QString folderTransactionsOnValidator(QString::fromLatin1("/mnt/sda/transaction"));
    auto model = static_cast<ValidatorListModel*>(ui.listView->model());
    QString pathUploadSoftware;
    if(isNeedUpdateSoftware)
    {
        const auto data = ui.comboBoxUpdateSoftware->currentData();
        if(!data.isNull() && data.isValid())
        {
            pathUploadSoftware = data.toString();//settings_->value(namePathUploadSoftware).toString();
        }
    }

    QString pathUploadWhitelist;
    if(isNeedUploadWhitelist)
    {
        const auto data = ui.comboBoxUpdateWhitelist->currentData();
        if(!data.isNull() && data.isValid())
        {
            pathUploadWhitelist = data.toString();//settings_->value(namePathUploadWhitelist).toString();
        }
    }
    std::cerr << logger() << "Path update software " << pathUploadSoftware.toStdString() << std::endl;
    std::cerr << logger() << "Path update whitelist " << pathUploadWhitelist.toStdString() << std::endl;
    std::cerr << logger() << "Setting keys: " << settings_->allKeys().join(tr(" ")).toStdString() << std::endl;
    haveError_ = false;
    const auto percent = 0;
    for(int i = 0; i < model->rowCount(QModelIndex()); ++i)
    {
        const auto ipString = model->index(i).data(ValidatorListModel::deviceRole::IPRole).toString();
        const auto idString = model->index(i).data(ValidatorListModel::deviceRole::IdRole).toString();

        QJsonObject jsonObject;
        jsonObject.insert(tr("message"), tr("Update software"));
        jsonObject.insert(tr("percent"), percent);
        jsonObject.insert(tr("ip"), ipString);

        model->setData(model->index(i), percent, ValidatorListModel::deviceRole::UpdatePercentJobRole);
        bool isValidId = false;
        idString.toLong(&isValidId, 10);
        if(!isValidId)
        {
            std::cerr << logger() << "Skip upload to " << ipString.toStdString() << std::endl;
            continue;
        }
        Upload *uploadProcess = new Upload(login, ipString, idString, pathUploadSoftware, pathUploadWhitelist);

        QThread* thread = new QThread;
        uploadProcess->moveToThread(thread);

        connect(thread, &QThread::started, uploadProcess, &Upload::process, Qt::QueuedConnection);
        connect(uploadProcess, &Upload::updateProcess, this, &updater::updateProcessTransactions, Qt::QueuedConnection);
        connect(uploadProcess, &Upload::error, this, &updater::errorProcess, Qt::QueuedConnection);
        connect(uploadProcess, &Upload::finished, thread, &QThread::quit, Qt::QueuedConnection);
        connect(uploadProcess, &Upload::finished, this, &updater::finishedUpdateValidator, Qt::QueuedConnection);
        connect(this, &updater::stopAll, uploadProcess, &Upload::stop, Qt::QueuedConnection);
        connect(uploadProcess, &Upload::finished, uploadProcess, &Upload::deleteLater, Qt::QueuedConnection);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);

        thread->start();
        countQueryIp_.fetchAndAddAcquire(1);
    }
}

void updater::setStateFindValidator()
{
    connect(ui.pbProcessValidators, &QPushButton::clicked, this, &updater::commnadFindValidators, Qt::QueuedConnection);
    ui.pbProcessValidators->setEnabled(true);
    ui.pbProcessValidators->setText(tr("Find validators"));
    ui.pbProcessValidators->adjustSize();
}

void updater::finishedUpdateValidator()
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
    const auto timezoneString = index.data(ValidatorListModel::deviceRole::TimezoneRole).toString();
    ui.pbChangeId->setVisible(idString != DetectorValidator::noValidator);

    ui.labelIdValidatorValue->setText(idString);
    ui.labelTimezoneValue->setText(timezoneString);
}

void updater::commandUploadTransactionToServer()
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

void updater::errorProcess(const QString ipString, const QString message)
{
    std::ignore = ipString;
    ui.labelStatusDetect->setText(message);
    haveError_ = true;

    auto model = ui.listView->model();
    if(model == nullptr)
    {
        return;
    }
    QJsonObject jsonObject;
    jsonObject.insert(tr("message"), message);
    auto list = model->match(model->index(0, 0), ValidatorListModel::deviceRole::IPRole, ipString);
    for(auto& item : list)
    {
        model->setData(item, jsonObject, ValidatorListModel::deviceRole::UpdatePercentJobRole);
    }
}

void updater::commandChangeValidatorId()
{
    auto idString = ui.listView->currentIndex().data(ValidatorListModel::deviceRole::IdRole).toString();

    const auto ipString = ui.listView->currentIndex().data(ValidatorListModel::deviceRole::IPRole).toString();
    ChangeIdDialog* dialog = new ChangeIdDialog(this);
    dialog->setEditText(idString);
    if(dialog->exec() == QDialog::Accepted)
    {
        idString = dialog->text();
        QProcess* process_ = new QProcess();
        process_->setProgram(QString::fromLatin1("/bin/bash"));
        process_->start();

        /*QString sourceFile;

        const auto login = settings_->value(nameLogin, QString::fromLatin1("root")).toString();
        const auto paramsScp = QStringList() << QString::fromLatin1("-r") << QString::fromLatin1("%1").arg(sourceFile)
                                             <<QString::fromLatin1("%1@%2:%3").arg(login).arg(ipString).arg(DetectorValidator::client_id);
        std::cerr << logger() << "scp " <<  paramsScp.join(QString::fromLatin1(" ")).toStdString() << std::endl;
        const auto exitCode = process_->execute(QString::fromLatin1("scp"), paramsScp);*/
        boost::filesystem::path pathSettings = boost::filesystem::path(DetectorValidator::pathSettings.toStdString()).parent_path();
        const auto login = settings_->value(nameLogin, QString::fromLatin1("root")).toString();
        /*const auto changeIdCommandParams = QString::fromLatin1("%1@%2 \"mkdir -p %3 && echo %4 > %5\"")
                .arg(login)
                .arg(ipString)
                .arg(QString::fromStdString(pathSettings.string()))
                .arg(idString)
                .arg(DetectorValidator::pathClient_id)
                ;*/
        const auto changeIdCommandParams =
                QStringList()
                << QString::fromLatin1("%1@%2").arg(login).arg(ipString)
                   //<<QString::fromLatin1("exit")
                   <<QString::fromLatin1("mkdir -p %1 && echo %2 > %3")
                     .arg(QString::fromStdString(pathSettings.string()))
                     .arg(idString)
                     .arg(DetectorValidator::pathSettings)
                ;
        //int exitCode = process_->write(changeIdCommand.c_str());
        std::cerr << logger() << "ssh " << changeIdCommandParams.join(QString::fromLatin1(" ")).toStdString() << std::endl;
        int exitCode = process_->execute(QString::fromLatin1("ssh"), changeIdCommandParams);
        //int exitCode = process_->execute(QString::fromLatin1("ssh")+changeIdCommandParams.join(QString::fromLatin1(" ")));
        if (exitCode != 0)
        {
            std::cerr << logger() << "ERROR execute: ssh "<< changeIdCommandParams.join(QString::fromLatin1(" ")).toStdString()<< " status error code: "<< exitCode << std::endl;
            // indicate can not set id validator
            return ;
        }
        modelUpdateId(idString, ipString);
    }
}

void updater::modelUpdateId(const QString& idString, const QString ipString)
{
    auto model = ui.listView->model();
    if(model == nullptr)
    {
        return;
    }
    auto list = model->match(model->index(0, 0), ValidatorListModel::deviceRole::IPRole, ipString);
    for(auto& item : list)
    {
        model->setData(item, idString, ValidatorListModel::deviceRole::ChangeIdRole);
    }
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

void updater::modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    auto currentIndex = ui.listView->currentIndex();
    if(topLeft.row() != bottomRight.row()
            && topLeft.column() != bottomRight.column()
            && currentIndex.row() != topLeft.row()
            && roles.size() != 1
            /*&& roles.first() != ValidatorListModel::deviceRole::*/)
    {
        std::cerr << "I don't known updater" <<std::endl;
        return;
    }

    updateInfoValidator(currentIndex.data(ValidatorListModel::deviceRole::IdRole).toString());
}

void updater::modelReseted()
{
    updateInfoValidator(QString::fromLatin1(" "));
}

void updater::commandDownloadUpdates()
{
    const std::string filenameDownloadScript("download_updates.sh");
    const auto scriptFilename = folderAplication_/"scripts"/filenameDownloadScript;
    /*if(boost::filesystem::exists(scriptFilename) && boost::filesystem::is_regular_file(scriptFilename))
    {
        std::cerr << logger() << "Not found script: " << scriptFilename.string() << std::endl;
        return;
    }*/

    ui.pbProcessValidators->setEnabled(false);
    ui.pbDownloadUpdates->setEnabled(false);

    const QString sourcePathSW(QStringLiteral("./images/Application"));
    const QString destinationPath = QString::fromStdString((folderAplication_/localSubFolderUpdateSoftware_).string());
    DownloadUpdates *downloadUpdateProcess = new DownloadUpdates(QString::fromStdString(scriptFilename.string()), sourcePathSW, destinationPath);

    QThread* thread = new QThread;
    downloadUpdateProcess->moveToThread(thread);

    connect(thread, &QThread::started, downloadUpdateProcess, &DownloadUpdates::process, Qt::QueuedConnection);
    connect(downloadUpdateProcess, &DownloadUpdates::updateProcess, this, &updater::updateProcessDownloadUpdates, Qt::QueuedConnection);
    connect(downloadUpdateProcess, &DownloadUpdates::error, this, &updater::errorProcessDownloadUpdates, Qt::QueuedConnection);
    connect(downloadUpdateProcess, &DownloadUpdates::finished, thread, &QThread::quit, Qt::QueuedConnection);
    connect(downloadUpdateProcess, &DownloadUpdates::finished, this, &updater::finishedDownloadUpdates, Qt::QueuedConnection);
    connect(this, &updater::stopAll, downloadUpdateProcess, &DownloadUpdates::stop, Qt::QueuedConnection);
    connect(downloadUpdateProcess, &DownloadUpdates::finished, downloadUpdateProcess, &DownloadUpdates::deleteLater, Qt::QueuedConnection);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);

    thread->start();
}

void updater::updateProcessDownloadUpdates(int percent, const QString message)
{
    ui.labelStatusDetect->setText(tr("%1 %2%").arg(message).arg(percent));
}

void updater::finishedDownloadUpdates()
{
    ui.pbProcessValidators->setEnabled(true);
    ui.pbDownloadUpdates->setEnabled(true);
    ui.comboBoxUpdateSoftware->clear();
    ui.comboBoxUpdateSoftware->setCurrentIndex(-1);
    ui.comboBoxUpdateWhitelist->clear();
    ui.comboBoxUpdateWhitelist->setCurrentIndex(-1);

    fillListUpdateSoftware();
}

void updater::fillListUpdateSoftware()
{
    boost::filesystem::path path(folderAplication_/localSubFolderUpdateSoftware_/"Application");
    std::cerr << logger() << "Read updates from: " << path.string() << std::endl;

    if(!boost::filesystem::exists(path))
    {
        if(!boost::filesystem::create_directories(path))
        {
            std::cerr << logger() << "Can't create directories: " << path.string() << std::endl;
        }
    }

    for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(path), {}))
    {
        std::cerr <<  entry.path().string() << std::endl;

        if(boost::filesystem::is_regular_file(entry))
        {
            try
            {
                const auto zipFilename = entry.path().string();
                const auto tmpFolder = folderAplication_/localSubFolderUpdateSoftware_/"tmp";
                boost::filesystem::create_directories(tmpFolder);
                const auto tmpFile = (tmpFolder/"README.md");
                ZipFile::ExtractFile(zipFilename, "README.md", tmpFile.string());

                std::ifstream in(tmpFile.string().c_str());
                std::string contents;
                std::getline(in, contents);
                std::cerr << logger() << contents <<std::endl;
                ui.comboBoxUpdateSoftware->addItem(QString::fromStdString(contents), QVariant::fromValue<QString>(QString::fromStdString(entry.path().string())));
            }
            catch(const std::exception& ex)
            {
                std::cerr << logger() << ex.what() <<std::endl;
            }
        }
    }

    ui.comboBoxUpdateSoftware->setCurrentIndex(-1);
}

void updater::errorProcessDownloadUpdates( const QString message)
{
    finishedDownloadUpdates();
    ui.labelStatusDetect->setText(message);
}

void updater::updateInfoValidator(const QString& idValidator)
{
    //std::cerr << logger() << ui.labelIdValidatorValue->text().toStdString() << std::endl;
    ui.labelIdValidatorValue->setText(idValidator);
    //std::cerr << logger() << ui.labelIdValidatorValue->text().toStdString() << std::endl;
}

void updater::loadTranslate()
{
    //QTranslator* myTranslator = new QTranslator(QCoreApplication::instance());
    //myTranslator.load("validator_" + QLocale::system().name());

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

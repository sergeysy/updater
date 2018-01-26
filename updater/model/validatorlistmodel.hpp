#pragma once

#include <QJsonObject>
#include <QString>
#include <QAbstractListModel>
#include <QAbstractProxyModel>

class Validator
{
public:
    Validator(const QString& ipString, const QJsonObject);
    Validator(){};
    ~Validator();

    void setId(const QString& idValidator);
    void setIP(const QString& ipString);
    void setJsonObject(const QJsonObject& jsonObject);
    QString getId() const noexcept;
    QString getIP() const noexcept;
    QString getTimezone() const noexcept;
    QString getConfigVersion() const noexcept;
    void setPercentJob(const int percentJob) noexcept;
    int  getPercentJob() const noexcept;

    void setMessageJob(const QString message) noexcept;
    QString getMessageJob() const noexcept;
    QString getOSVersion() const noexcept;
    QString getTransactions() const noexcept;
    QString getMemFree() const noexcept;
    QString getMemTotal() const noexcept;
private:
    QString ipString_;
    //QString idValidator_;
    QJsonObject jsonObject_;
    int percentJob_ = 0;
    QString message_;
};

class ValidatorListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ValidatorListModel(QObject *parent = nullptr);

    enum deviceRole {
        DisplayRole = Qt::DisplayRole,
        IPRole = Qt::UserRole + 1,
        IdRole = Qt::UserRole + 2,
        UpdatePercentJobRole = Qt::UserRole + 3,
        PercentJobRole = Qt::UserRole + 4,
        ChangeIdRole  =  Qt::UserRole + 5,
        TimezoneRole = Qt::UserRole + 6,
        ConfigVersionRole = Qt::UserRole + 7,
        OSVersionRole =  Qt::UserRole + 8,
        TransactionsRole = Qt::UserRole + 9,
        MemFreeRole = Qt::UserRole + 10,
        MemTotalRole = Qt::UserRole + 11,

    };
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const; // функция доступа к данным
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent) const; // количество элементов в модели
    virtual bool removeRows(int row,
                            int count,
                            const QModelIndex &parent = QModelIndex()) override;
    virtual QModelIndexList	match(const QModelIndex &start,
                                  int role,
                                  const QVariant &value,
                                  int hits = 1,
                                  Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap )) const override;
    virtual QModelIndex	index(int row,
                              int column,
                              const QModelIndex &parent = QModelIndex()) const override;

    void clear();

private:
    QList<Validator> devices_;
};

class ValidatorProcessUpdateProxyModel : public QAbstractProxyModel {
Q_OBJECT
  public:
    ValidatorProcessUpdateProxyModel(QObject *parent = 0);

    virtual QModelIndex	mapFromSource(
            const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(
            const QModelIndex& proxyIndex) const;

    virtual QModelIndex index(int, int,
                              const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual int columnCount(const QModelIndex& parent) const;
    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;

    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
};

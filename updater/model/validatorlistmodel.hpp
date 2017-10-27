#pragma once
#include <QString>
#include <QAbstractListModel>
#include <QAbstractProxyModel>

class Validator
{
public:
    Validator(const QString& ipString, const QString& idValidator);
    ~Validator();

    void setId(const QString& idValidator);
    QString getId() const noexcept;
    QString getIP() const noexcept;
    void setPercentJob(const int percentJob) noexcept;
    int  getPercentJob() const noexcept;

private:
    QString ipString_;
    QString idValidator_;
    int percentJob_ = 0;
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
    };
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // функция доступа к данным
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent) const; // количество элементов в модели
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual QModelIndexList	match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap )) const override;

public slots:
    void addDevice(Validator device); // добавить контакт в модель

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

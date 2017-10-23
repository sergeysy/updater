#pragma once
#include <QString>
#include <QAbstractListModel>

class Validator
{
public:
    Validator(const QString& ipString, const QString& idValidator);
    ~Validator();

    void setId(const QString& idValidator);
    QString getId() const;
    QString getIP() const;

private:
    QString ipString_;
    QString idValidator_;
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
    };
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // функция доступа к данным
    int rowCount(const QModelIndex &parent) const; // количество элементов в модели
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
public slots:
    void addDevice(Validator device); // добавить контакт в модель

private:
    QList<Validator> devices_;
};

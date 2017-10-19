#include "validatorlistmodel.hpp"

Validator::Validator(const QString& ipString, const QString& idValidator)
    : ipString_(ipString)
    , idValidator_(idValidator)
{

}

Validator::~Validator()
{
}

QString Validator::getId() const
{
    return idValidator_;
}

QString Validator::getIP() const
{
    return ipString_;
}

void Validator::setId(const QString& idValidator)
{
    idValidator_ = idValidator;
}


ValidatorListModel::ValidatorListModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

QVariant ValidatorListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > devices_.count())
        return QVariant();
    const Validator& device = devices_[index.row()];

    switch (role) {
    case DisplayRole:
        return device.getIP() + QString::fromLatin1(" ") + device.getId();
        break;
    case IdRole:
        return device.getId();
        break;
    case IPRole:
        return device.getIP();
        break;
    default:
        break;
    }

    return QVariant();
}

void ValidatorListModel::addDevice(Validator device)
{
    beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));
    devices_ << device;
    endInsertRows();
}

int ValidatorListModel::rowCount(const QModelIndex &parent) const
{
    std::ignore = parent;

    return devices_.size();
}

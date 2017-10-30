#include "validatorlistmodel.hpp"

Validator::Validator(const QString& ipString, const QString& idValidator)
    : ipString_(ipString)
    , idValidator_(idValidator)
{

}

Validator::~Validator()
{
}

QString Validator::getId() const  noexcept
{
    return idValidator_;
}

QString Validator::getIP() const  noexcept
{
    return ipString_;
}

void Validator::setPercentJob(const int percentJob)  noexcept
{
    percentJob_ = percentJob;
}

int Validator::getPercentJob() const noexcept
{
    return percentJob_;
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
    case Qt::EditRole:
        return QString::fromLatin1("%1 %2").arg(device.getIP()).arg(device.getId());
        break;
    case IdRole:
        return device.getId();
        break;
    case IPRole:
        return device.getIP();
        break;
    case PercentJobRole:
        bool ok = false;
        device.getId().toInt(&ok);
        if(ok)
        {
            const auto percent = device.getPercentJob();
            auto result = tr("%1 %2%").arg((percent<100)?QString::fromLatin1("Recieve data "):QString::fromLatin1("Completed")).arg(percent);
            return QVariant::fromValue(result);
        }
        else
        {
            return QVariant::fromValue(tr("Nothing do"));
        }
        break;
    /*default:
        break;*/
    }

    return QVariant();
}

bool ValidatorListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const auto indexRow = index.row();
    if ( indexRow < 0 || indexRow > devices_.count())
        return false;

    switch (role) {
    case UpdatePercentJobRole:
    {
        Validator& device = devices_[indexRow];
        device.setPercentJob(value.toInt());
        emit dataChanged(index, index);
        return true;
    }
    break;
    case ChangeIdRole:
    {
        Validator& device = devices_[indexRow];
        device.setId(value.toString());
        emit dataChanged(index, index);
        return true;
    }
    break;
    /*default:
        break;*/
    }

    return false;
}
void ValidatorListModel::addDevice(Validator device)
{
    beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));
    devices_ << device;
    endInsertRows();
}

int ValidatorListModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
           return 0;

    return devices_.size();
}

bool ValidatorListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    std::ignore = parent;
    if(row + count > devices_.size())
    {
        return false;
    }

    beginRemoveRows(parent, row, row + count);

    QList<Validator>::iterator itFirst = devices_.begin();
    QList<Validator>::iterator itEnd = devices_.begin();
    std::advance(itFirst, row);
    std::advance(itEnd, row + count);

    devices_.erase(itFirst, itEnd);
    endRemoveRows();
    return true;
}

QModelIndexList ValidatorListModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    QModelIndexList list;
    if(start.row() > devices_.size() || hits == 0)
    {
        return list;
    }
    //ignore flags
    std::ignore = flags;

    switch (role) {
    case UpdatePercentJobRole:
        //QList<Validator>::const_iterator itStart = devices_.constBegin()+start.row();
        QList<Validator>::const_iterator itStart(devices_.cbegin()+start.row());
        QList<Validator>::const_iterator itEnd(devices_.cend());
        const auto ipString = value.toString();
        int i = start.row();
        for(auto it = itStart; it != itEnd; ++it, ++i)
        {
            if(hits != -1)
            {
                if( hits > 0)
                {
                    --hits;
                }
                else
                {
                    break;
                }
            }
            if((*it).getIP() == ipString)
            {
                list.push_back(QAbstractItemModel::createIndex(i,0));
            }
        };
        break;
    /*default:
        break;*/
    }

    return list;
}

void ValidatorListModel::clear()
{
    beginResetModel();
    devices_.clear();
    endResetModel();
}

ValidatorProcessUpdateProxyModel::ValidatorProcessUpdateProxyModel(QObject *parent)
  : QAbstractProxyModel(parent)
{
}
QModelIndex ValidatorProcessUpdateProxyModel::mapFromSource(
                const QModelIndex& sourceIndex) const
{
  return index(sourceIndex.column(), sourceIndex.row());
}

QModelIndex ValidatorProcessUpdateProxyModel::mapToSource(
                const QModelIndex& proxyIndex) const
{
  return sourceModel()->index(proxyIndex.row(), proxyIndex.column());
}
QModelIndex ValidatorProcessUpdateProxyModel::index(int row, int column,
                const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return createIndex(row, column);
}
QModelIndex ValidatorProcessUpdateProxyModel::parent(
                const QModelIndex& index) const
{
  Q_UNUSED(index);
  return QModelIndex();
}
int ValidatorProcessUpdateProxyModel::rowCount(const QModelIndex& parent) const
{
  return sourceModel()->rowCount(parent);
}

int ValidatorProcessUpdateProxyModel::columnCount(const QModelIndex& parent) const
{
  return sourceModel()->columnCount(parent);
}
QVariant ValidatorProcessUpdateProxyModel::data(const QModelIndex& index,
                    int role) const
{
  if (!index.isValid()) return QVariant();
  if(role == Qt::DisplayRole || role == Qt::EditRole)
  {
      return sourceModel()->data(mapToSource(index), ValidatorListModel::deviceRole::PercentJobRole);
  }
  return sourceModel()->data(mapToSource(index), role);
}
QVariant ValidatorProcessUpdateProxyModel::headerData(int section,
            Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal)
    return sourceModel()->headerData(section, Qt::Vertical, role);
  else
    return sourceModel()->headerData(section, Qt::Horizontal, role);
}


#include "relationmodel.h"
#include "qtsmalltools.h"
#include "mainwindow.h"
#include "commonfunction.h"
#include <QMessageBox>

using namespace std;
using namespace Jimmy;

static int IDCounter = 1;
RelationItem::RelationItem()
    :RID(IDCounter++)
{

}

RelationItem::RelationItem(const RelationItem& item)
    :RID(item.RID)
{
    name = item.name;
    checkStatus = item.checkStatus;
}

RelationModel::RelationModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

RelationModel::~RelationModel()
{
    release();
}

void RelationModel::release()
{
    qDeleteAll(RelationItems_);
    RelationItems_.clear();
}

int RelationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return RelationItems_.count();
}

QVariant RelationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole ||(role == Qt::EditRole))
    {
        if(RelationItem* item = RelationItems_.at(index.row()))
        {
            return item->name;
        }
        return QVariant();
    }

    if (role == Qt::CheckStateRole)
    {
        if(RelationItem* item = RelationItems_.at(index.row()))
        {
            return item->checkStatus;
        }
        return QVariant();
    }

    if (role == Qt::DecorationRole)
    {
        return QTSmalltools::svgIcon(":/image/image/relation.svg");
    }

    return QVariant();

    if (role == RelationRole)
    {
        return QVariant::fromValue((void*)RelationItems_.at(index.row()));
    }

    return QVariant();
}

bool RelationModel::setData(const QModelIndex &index, const QVariant &value,int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if(role == Qt::CheckStateRole)
    {
        if(RelationItem* item = RelationItems_.at(index.row()))
        {
            item->checkStatus = value.toInt();
            emit selectedRelationChanged(item);
            return true;
        }
    }

    if (index.isValid() && role == Qt::EditRole)
    {
        if(RelationItem* item = RelationItems_.at(index.row()))
        {
            item->name = value.toString();
            emit selectedRelationChanged(item);
            emit dataChanged(index, index);
            return true;
        }
    }

    return false;
}

Qt::ItemFlags RelationModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index)|Qt::ItemIsUserCheckable ;
}

RelationItem* RelationModel::itemFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }

    return RelationItems_.at(index.row());
}

QModelIndex RelationModel::getIndex(const RelationItem* item)
{
    if (!item)
    {
        return QModelIndex();
    }

    for(int i=0;i<RelationItems_.size();++i)
    {
        if(RelationItems_.at(i) == item)
        {
            return createIndex(i, 0, const_cast<RelationItem*>(item));
        }
    }

    return QModelIndex();
}

Jimmy::ErrorCode RelationModel::setRelation(const QJsonArray& ja)
{
    beginResetModel();
    release();
    auto ret = resetRelation_(ja);
    endResetModel();
    return ret;
}


Jimmy::ErrorCode RelationModel::resetRelation_(const QJsonArray& ja)
{
    RelationItems_.clear();
    for(auto itor = ja.constBegin();itor!= ja.constEnd();++itor)
    {
        if(!itor->isObject())
        {
            QMessageBox::critical(g_MainWindow_, QStringLiteral("错误"),QStringLiteral("%1 format is invalid").arg(itor->toVariant().toString()));
            return ErrorCode::ec_error;
        }

        RelationItem* item = new RelationItem();

        item->name = itor->toString();
        item->checkStatus = Qt::Unchecked;

        RelationItems_.push_back(item);
    }

    return ErrorCode::ec_ok;
}

QJsonArray RelationModel::getRelation()
{
    if (!verifyRelation())
    {
        return QJsonArray();
    }

    QJsonArray ja;
    foreach (RelationItem* item,RelationItems_)
    {
        ja.append(item->name);
    }

    return ja;
}

bool RelationModel::verifyRelation()
{
    QStringList items;
    foreach (RelationItem* item,RelationItems_)
    {
        if (items.contains(item->name))
        {
            QMessageBox::critical(g_MainWindow_, QStringLiteral("错误"),QStringLiteral("%1 重复的关系名称").arg(item->name));
            return false;
        }

        items.append(item->name);
    }

    return true;
}

QModelIndex RelationModel::appendRelation()
{
    RelationItem* item = new RelationItem();

    item->name = QStringLiteral("[新建关系]");
    item->checkStatus = Qt::Unchecked;

    beginInsertRows(QModelIndex(), RelationItems_.size(), RelationItems_.size());
    RelationItems_.append(item);
    endInsertRows();

    return getIndex(item);
}

void RelationModel::removeRelation(const QModelIndex& ind)
{
    RelationItem* item = itemFromIndex(ind);
    if (!item)
    {
        return;
    }

    int row = RelationItems_.indexOf(item);
    if (row == -1)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    RelationItems_.removeAt(row);
    endRemoveRows();

    delete item;
    item = nullptr;
}

void RelationModel::uncheckRelation(int RID)
{
     foreach(auto item,RelationItems_)
     {
         if(item->RID == RID)
         {
             QModelIndex modelIndex = getIndex(item);
             setData(modelIndex,QVariant(Qt::Unchecked),Qt::CheckStateRole);
             break;
         }
     }
}

QModelIndex RelationModel::moveUp(const QModelIndex& index)
{
    RelationItem* item = itemFromIndex(index);
    if (!item)
    {
        return QModelIndex();
    }

    int pos = RelationItems_.indexOf(item);

    if(pos > 0)
    {
        emit layoutAboutToBeChanged();
        RelationItems_.move(pos,pos-1);
        emit layoutChanged();
    }

    return getIndex(item);
}

QModelIndex RelationModel::moveDown(const QModelIndex& index)
{
    RelationItem* item = itemFromIndex(index);
    if (!item)
    {
        return QModelIndex();
    }

    int pos = RelationItems_.indexOf(item);

    if(pos < RelationItems_.size())
    {
        emit layoutAboutToBeChanged();
        RelationItems_.move(pos,pos+1);
        emit layoutChanged();
    }

    return getIndex(item);
}


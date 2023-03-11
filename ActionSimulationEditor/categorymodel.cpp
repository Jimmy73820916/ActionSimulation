/*******************************************************************************
EasyVsp System
Copyright (c) 2022 Jimmy Song

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#include <QIcon>
#include <QDebug>
#include <QMimeData>
#include <QJsonDocument>
#include <QJsonArray>

#include "categorymodel.h"
#include "qtsmalltools.h"

#include "commonfunction.h"
#include "mainwindow.h"
#include <QMessageBox>

using namespace std;
using namespace Jimmy;

const QString dragString = "category/item.list";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief CategoryItem::CategoryItem
///

CategoryItem* CategoryItem::clickSource_ = nullptr;
CategoryModel* CategoryItem::filterModel_ = nullptr;

CategoryItem::CategoryItem(const QString& name,CategoryItem* parentItem)
    :parentItem_(parentItem)
    ,name_(name)
    ,checkStatus_(false)
{

}

CategoryItem::~CategoryItem()
{
    removeChildren();
}

void CategoryItem::setCategoryModel(CategoryModel* filterModel)
{
    filterModel_ = filterModel;
}

CategoryItem* CategoryItem::find(const QString& name)
{
    if(name_ == name)
    {
        return this;
    }

    Q_FOREACH(auto item ,childItems_)
    {
        auto node = item->find(name);
        if(node)
        {
            return node;
        }
    }

    return nullptr;
}

int CategoryItem::find(const CategoryItem* item)
{
    for (int index = 0; index < childItems_.size(); ++index)
    {
        if (childItems_[index] == item)
        {
            return index;
        }
    }

	return -1;
}

bool CategoryItem::verifyCategory(QVector<QString>& names) const
{
    if(names.contains(name_))
    {
        return false;
    }

    names.push_back(name_);

    Q_FOREACH(auto item ,childItems_)
    {
        if(!item->verifyCategory(names))
        {
            return false;
        }
    }

    return true;
}

void CategoryItem::appendChild(CategoryItem* item)
{
    item->parentItem_ = this;
    childItems_.append(item);
}

void CategoryItem::removeChild(CategoryItem* item)
{
    auto itor = childItems_.begin();
    while(itor !=childItems_.end())
    {
        if(*itor == item)
        {
            delete *itor;
            childItems_.erase(itor);
            item = nullptr;
            break;
        }
        ++itor;
    }
}

void CategoryItem::move(CategoryItem* newParent)
{
    parentItem_->takeChild(this);
    newParent->appendChild(this);
    parentItem_ = newParent;
}

void CategoryItem::up()
{
    if(row() != 0)
    {
        parentItem_->childItems_.move(row(),row()-1);
    }
}

void CategoryItem::down()
{
    if((parentItem_->childItems_.count() != 1) && (row() != (parentItem_->childItems_.count()-1)))
    {
        parentItem_->childItems_.move(row(),row()+1);
    }
}

void CategoryItem::appendChildren(const QJsonObject& jo)
{
    auto elemItor = jo.find(name_);
    if(elemItor == jo.end())
    {
        return;
    }

    if(!elemItor->isArray() )
    {
        if(filterModel_)
        {
            emit filterModel_->errorMessage(QStringLiteral("filters 内容不是数组"));
        }

        return;
    }

    auto filters = elemItor->toArray();
    for(auto itor = filters.cbegin();itor !=filters.cend();++itor)
    {
        if(!itor->isString())
        {
            if(filterModel_)
            {
                emit filterModel_->errorMessage(QStringLiteral("筛选器包含无效内容"));
            }
            continue;
        }

        CategoryItem* item = new CategoryItem(itor->toString(),this);
        appendChild(item);
        item->appendChildren(jo);
    }
}

void CategoryItem::getChildren(QJsonObject& jo)
{
    if(childItems_.empty() && (parentItem_ != nullptr))
    {
        return;
    }

    QJsonArray ja;
    Q_FOREACH(auto item,childItems_)
    {
        ja.append(item->name_);
    }
    jo.insert(name_,ja);

    Q_FOREACH(auto item, childItems_)
	{
        item->getChildren(jo);
	}
}

void CategoryItem::removeChildren()
{
    qDeleteAll(childItems_);
    childItems_.clear();
}

void CategoryItem::takeChild(CategoryItem* item)
{
    foreach (CategoryItem* child,childItems_)
    {
        if(child == item)
        {
            childItems_.removeOne(item);
            break;
        }
    }
}

CategoryItem* CategoryItem::child(int row)
{
    if(row < 0 || row >= childItems_.size())
    {
        return nullptr;
    }

    return childItems_[row];
}

CategoryItem* CategoryItem::parentItem()
{
    return parentItem_;
}

int CategoryItem::childCount() const
{
    return childItems_.size();
}

QString CategoryItem::text()
{
    return name_;
}

void CategoryItem::setText(const QString& name)
{
    name_ = name;
}

QVariant CategoryItem::data(int role) const
{
    switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
    {
        return name_;
    }
    case Qt::CheckStateRole:
    {
        if (childItems_.count())
        {
            return childrenCheckState();
        }
        else
        {
            return checkStatus_;
        }
    }
    }
    return QVariant();
}

QVariant CategoryItem::childrenCheckState() const
{
    bool checkedChildren = false;
    bool uncheckedChildren = false;

    foreach (CategoryItem* child,childItems_)
    {
        QVariant value = child->data(Qt::CheckStateRole);
        if (!value.isValid())
        {
            return QVariant();
        }

        switch (static_cast<Qt::CheckState>(value.toInt()))
        {
        case Qt::Unchecked:
        {
            uncheckedChildren = true;
            break;
        }
        case Qt::Checked:
        {
            checkedChildren = true;
            break;
        }
        default:
            return Qt::PartiallyChecked;
        }

        if (uncheckedChildren && checkedChildren)
        {
            return Qt::PartiallyChecked;
        }
    }

    if (uncheckedChildren)
    {
        return Qt::Unchecked;
    }
    else if (checkedChildren)
    {
        return Qt::Checked;
    }

    return QVariant(); // value was not defined
}

QVariant CategoryItem::getIcon()
{
    return QTSmalltools::svgIcon(":/image/image/category.svg");
}

void CategoryItem::setData(int role, const QVariant& value)
{
    switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
    {
        name_ = value.toString();
        if (filterModel_)
        {
            filterModel_->emitDataChanged(this, { Qt::DisplayRole, Qt::EditRole });
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        int currentStatus = value.toInt();

        if (clickSource_ == nullptr)
        {
            clickSource_ = this;
        }

        if (value != Qt::PartiallyChecked)
        {
            if (checkStatus_ != currentStatus)
            {
                for (int i = 0; i < childItems_.count(); ++i)
                {
                    foreach (CategoryItem* item,childItems_)
                    {
                        item->setData(role, value);
                    }
                }
            }
        }

        checkStatus_ = currentStatus;

        if (clickSource_ == this)
        {
            if (filterModel_)
            {
                filterModel_->emitDataChanged(this, { role });
                if (role == Qt::CheckStateRole)
                {
                    for (CategoryItem* p = parentItem_; p; p = p->parentItem_)
                    {
                        if (filterModel_)
                        {
                            if(p->childrenCheckState().isValid())
                            {
                                p->checkStatus_ =p->childrenCheckState().toInt();
                                filterModel_->emitDataChanged(p, { role });
                            }

                        }
                    }
                }
            }
            clickSource_ = nullptr;
        }
        return;
    }
    }
}

int CategoryItem::row() const
{
    if (parentItem_)
    {
        return parentItem_->childItems_.indexOf(const_cast<CategoryItem*>(this));
    }

    return 0;
}

void CategoryItem::getAllSelected(QStringList& childrenName)
{
    Q_FOREACH(auto item,childItems_)
    {
        item->getAllSelected(childrenName);
    }

    if(checkStatus_ == Qt::Checked)
    {
        childrenName.push_back(name_);
    }
}

void CategoryItem::getAllDescendant(QStringList& descendantName)
{
    Q_FOREACH(auto item,childItems_)
    {
        item->getAllDescendant(descendantName);
    }

    descendantName.push_back(name_);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief CategoryModel::CategoryModel
/// \param parent
///


const QString CategoryModel::RootCategory = "_root";
const QString CategoryModel::UnloadProject = QStringLiteral("未加载项目");


CategoryModel::CategoryModel(QObject *parent)
    :QAbstractItemModel(parent)
{
    root_ = new CategoryItem("Project");

    project_ = new CategoryItem(UnloadProject,root_);
    root_->appendChild(project_);
}

CategoryModel::~CategoryModel()
{
    delete root_;
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if ((role == Qt::DisplayRole)||(role == Qt::CheckStateRole) || (role == Qt::EditRole))
    {
        CategoryItem* item = static_cast<CategoryItem*>(index.internalPointer());
        return item->data(role);
    }

    if (role == Qt::DecorationRole)
    {
        CategoryItem* item = static_cast<CategoryItem*>(index.internalPointer());
        return item->getIcon();
    }

    return QVariant();

}

QVariant CategoryModel::headerData(int /*section*/, Qt::Orientation orientation,int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return root_->data(role);
    }

    return QVariant();
}

QModelIndex CategoryModel::index(int row, int column,const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    CategoryItem* parentItem(nullptr);

    if (parent.isValid())
    {
        parentItem = static_cast<CategoryItem*>(parent.internalPointer());
    }
    else
    {
        parentItem = root_;
    }

    if (auto childItem = parentItem->child(row))
    {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex CategoryModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    if(auto childItem = static_cast<CategoryItem*>(index.internalPointer()))
    {
        CategoryItem *parentItem = childItem->parentItem();

        if (parentItem == root_)
        {
            return QModelIndex();
        }

        return createIndex(parentItem->row(), 0, parentItem);
    }

    return QModelIndex();
}

int CategoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    CategoryItem* parentItem(root_);
    if (parent.isValid())
    {
        parentItem = static_cast<CategoryItem*>(parent.internalPointer());
    }

    if(parentItem == nullptr)
    {
        return 0;
    }

    return parentItem->childCount();
}

int CategoryModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

Qt::ItemFlags CategoryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable |Qt::ItemIsAutoTristate|Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled ;
}

bool CategoryModel::setData(const QModelIndex &index, const QVariant &value,int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if((role == Qt::CheckStateRole)||(role == Qt::DisplayRole)||(role == Qt::EditRole))
    {
        if(CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer()))
        {
            item->setData(role,value);
            emit dataChanged(index,index,{role});
            return true;
        }
    }

    return false;
}

CategoryItem* CategoryModel::itemFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }

    return static_cast<CategoryItem*>(index.internalPointer());
}

void CategoryModel::emitDataChanged(CategoryItem* item, const QVector<int>& roles)
{
    QModelIndex left = index(item);
    emit dataChanged(left, left, roles);
}

QModelIndex CategoryModel::index(const CategoryItem* item)
{
    if (!item || (item == root_))
    {
        return QModelIndex();
    }

    return createIndex(item->row(), 0, const_cast<CategoryItem*>(item));
}

QModelIndex CategoryModel::appendCategory(const QModelIndex& ind)
{
    CategoryItem* parentItem = itemFromIndex(ind);
    if (!parentItem)
    {
        return QModelIndex();
    }

    int order = parentItem->childCount();
    beginInsertRows(index(parentItem), order, order);
    CategoryItem* item = new CategoryItem(QStringLiteral("[新建类别]"),parentItem);
    parentItem->appendChild(item);
    endInsertRows();

    g_MainWindow_->setProjectStatus(ProjectStatus::modified);
    return index(item);
}

void CategoryModel::appendCategory(const QString& category)
{
    QStringList list;
    getAll(list);

    if(list.contains(category))
    {
        return;
    }

    int order = project_->childCount();
    beginInsertRows(index(project_), order, order);
    CategoryItem* item = new CategoryItem(QStringLiteral("%1").arg(category),project_);
    project_->appendChild(item);
    endInsertRows();

    g_MainWindow_->setProjectStatus(ProjectStatus::modified);
}

void CategoryModel::eraseCategory(const QModelIndex &ind)
{
    CategoryItem* item = itemFromIndex(ind);
    if (!item)
    {
        return;
    }

    beginRemoveRows(index(item->parentItem()),item->row(), item->row());
    item->parentItem()->removeChild(item);
    endRemoveRows();

    g_MainWindow_->setProjectStatus(ProjectStatus::modified);
}

void CategoryModel::moveUp(const QModelIndex &ind)
{
    CategoryItem* item = itemFromIndex(ind);
    if (!item)
    {
        return;
    }

    auto row = item->row();
    if(item->row()!=0)
    {
        auto indexParent = index(item->parentItem());
        beginMoveRows(indexParent, row, row, indexParent, row -1);
        item->up();
        endMoveRows();

        g_MainWindow_->setProjectStatus(ProjectStatus::modified);
    }
}

void CategoryModel::moveDown(const QModelIndex &ind)
{
    CategoryItem* item = itemFromIndex(ind);
    if (!item)
    {
        return;
    }

    auto row = item->row();
    if(row!=(item->parentItem()->childCount()-1))
    {
        auto indexParent = index(item->parentItem());
        //向后移或许时移动后算一下索引，然后再删所以要+2
        beginMoveRows(indexParent, row, row, indexParent, row + 2);
        item->down();
        endMoveRows();

        g_MainWindow_->setProjectStatus(ProjectStatus::modified);
    }
}

void CategoryModel::setCategories(const QJsonObject& jo)
{
    auto elemItor = jo.find(RootCategory);
    if(elemItor == jo.end())
    {
        emit errorMessage(QStringLiteral("_root 键不存在"));
        return;
    }

    beginResetModel();
    project_->removeChildren();
    project_->setText(elemItor->toString());
    project_->appendChildren(jo);
    endResetModel();
}

QJsonObject CategoryModel::getCategories()
{
    if (!project_)
    {
        emit errorMessage(QStringLiteral("没有打开任何项目"));
        return QJsonObject();
    } 

    if (!verifyNode())
    {
        emit errorMessage(QStringLiteral("筛选器中有无效项"));
        return QJsonObject();
    }

    QJsonObject jo;
    project_->getChildren(jo);

    return jo;
}



bool CategoryModel::verifyNode()
{
    QVector<QString> filers;
    return project_->verifyCategory(filers);
}

void CategoryModel::clear()
{
    if (project_)
    {
        beginResetModel();
        project_->setText(UnloadProject);
        project_->removeChildren();
        endResetModel();
    }
}

void CategoryModel::getSelected(QStringList& selectedFilters)
{
    project_->getAllSelected(selectedFilters);
}

void CategoryModel::getAll(QStringList& categories)
{
    project_->getAllDescendant(categories);
}

const CategoryItem* CategoryModel::getProject() const
{
    return project_;
}

void CategoryModel::appendMissing(const QStringList& categories)
{
    foreach(auto item,categories)
    {
        if(project_->find(item)==nullptr)
        {
            int order = project_->childCount();
            beginInsertRows(index(project_), order, order);
            project_->appendChild(new CategoryItem(item,project_));
            endInsertRows();

            g_MainWindow_->setProjectStatus(ProjectStatus::modified);
        }
    }
}

Qt::DropActions CategoryModel::supportedDropActions() const
{
    return Qt::MoveAction ;
}

QStringList CategoryModel::mimeTypes() const
{
    QStringList types ;
    types << dragString ;
    return types ;
}

QMimeData* CategoryModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData* mimeData = new QMimeData();
	QByteArray encodeData;

	QDataStream stream(&encodeData, QIODevice::WriteOnly);
	foreach(const QModelIndex & index, indexes)
	{
		if (index.isValid())
		{
			stream << reinterpret_cast<quint64>(index.internalPointer());
		}
	}
	mimeData->setData(dragString, encodeData);
    return mimeData ;
}

bool CategoryModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if ( action == Qt::IgnoreAction ) { return true ; }
    if ( !data->hasFormat(dragString) ) { return false ; }
    if ( column > 0 ) { return false ; }

    QByteArray encodeData = data->data(dragString) ;
    QDataStream stream(&encodeData, QIODevice::ReadOnly) ;

    auto parentNode = itemFromIndex(parent);
    if(parentNode == nullptr)
    {
        return false;
    }

    while ( !stream.atEnd() ) {
        quint64 val ;
        stream >> val ;

        auto p = reinterpret_cast<CategoryItem*>(val);
        auto sourceParent = index(p->parentItem());
        int sourceIndex = p->parentItem()->find(p);
        if (sourceIndex == -1)
        {
            continue;
        }
        p->move(parentNode);

        beginMoveRows(sourceParent, sourceIndex, 1, parent, parentNode->childCount());
        endMoveRows();
    }

    g_MainWindow_->setProjectStatus(ProjectStatus::modified);

    return true ;
}

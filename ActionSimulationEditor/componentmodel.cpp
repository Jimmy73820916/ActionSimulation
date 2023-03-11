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

#include "componentmodel.h"
#include "qtsmalltools.h"
#include "commonfunction.h"
#include <QIcon>
#include <QPainter>
#include <QStyle>
#include <QPainterPath>
#include <QRectF>
#include <QJsonDocument>
#include <QMessageBox>
#include "mainwindow.h"
#include <QRegularExpression>

using namespace std;
using namespace Jimmy;

//////////////////////////////////////////////////////////////////////////////
/// \brief ComponentModel::ComponentModel
/// \param parent
///
///
ComponentModel::ComponentModel(QObject* parent)
    :QAbstractTableModel(parent)
{

}

ComponentModel::~ComponentModel()
{
    componentsShowed_.clear();
    release();
}

void ComponentModel::release()
{
    qDeleteAll(ComponentsWhole_);
    ComponentsWhole_.clear();
}

void ComponentModel::sort(int column,Qt::SortOrder order)
{
    if(componentsShowed_.isEmpty() || (column >= Column::Column_Uplimit))
    {
        return;
    }

    const auto is_asc = (order == Qt::AscendingOrder);

    std::sort(componentsShowed_.begin(),componentsShowed_.end(),[column,is_asc](const Jimmy::DesignComponent* lhs,const Jimmy::DesignComponent* rhs)
    {
        switch(column)
        {
        case ColumnCID: return is_asc?(lhs->getID()<rhs->getID()):(lhs->getID()>rhs->getID());
        case ColumnName: return is_asc?(lhs->getName()<rhs->getName()):(lhs->getName()>rhs->getName());
        case ColumnCategory: return is_asc?(lhs->getCategory()<rhs->getCategory()):(lhs->getCategory()>rhs->getCategory());
        case ColumnComponentType: return is_asc?(lhs->getType()<rhs->getType()):(lhs->getType()>rhs->getType());
        case ColumnTeam: return is_asc?(lhs->getTeam()<rhs->getTeam()):(lhs->getTeam()>rhs->getTeam());
        case ColumnRole: return is_asc?(lhs->getRole()<rhs->getRole()):(lhs->getRole()>rhs->getRole());
        case ColumnBehaviorType: return is_asc?(lhs->getBehavior()<rhs->getBehavior()):(lhs->getBehavior()>rhs->getBehavior());
        case ColumnActionKeep: return is_asc?(lhs->getActionKeep()<rhs->getActionKeep()):(lhs->getActionKeep()>rhs->getActionKeep());
        case ColumnSubscription:return is_asc?(lhs->getSubscription()<rhs->getSubscription()):(lhs->getSubscription()>rhs->getSubscription());
        case ColumnReference:return is_asc?(lhs->getReference()<rhs->getReference()):(lhs->getReference()>rhs->getReference());
        case ColumnRespondBoardcast:return is_asc?(lhs->getRespondBoardcast()<rhs->getRespondBoardcast()):(lhs->getRespondBoardcast()>rhs->getRespondBoardcast());
        case ColumnDefaultValue:return is_asc?(lhs->getDefaultValueString() < rhs->getDefaultValueString()):(lhs->getDefaultValueString()>rhs->getDefaultValueString());
        case ColumnDescription:return is_asc?(lhs->getDescription() < rhs->getDescription()):(lhs->getDescription()>rhs->getDescription());
        }

        return false;
    }
    );

    dataChanged(index(0,0),index(componentsShowed_.count()-1,Column::Column_Uplimit-1));
}

QVariant ComponentModel::data(const QModelIndex& index,int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole ||(role == Qt::EditRole))
    {
        auto item = componentsShowed_.at(index.row());
        switch (index.column())
        {
        case ColumnCID:return item->getID();
        case ColumnName:return item->getName();
        case ColumnCategory:return item->getCategory();
        case ColumnComponentType:return getTypeString(item->getType());
        case ColumnTeam:return item->getTeam();
        case ColumnRole:return item->getRole();
        case ColumnBehaviorType: return getBehaviorString(item->getBehavior());
        case ColumnActionKeep:return QStringLiteral("%1").arg(item->getActionKeep());
        case ColumnSubscription:return item->getSubscription().join(',');
        case ColumnReference:return item->getReference().join(',');
        case ColumnRespondBoardcast:return item->getRespondBoardcast().join(',');
        case ColumnDefaultValue:return item->getDefaultValueString();
        case ColumnDescription: return item->getDescription();

        default:break;
        }
        return QVariant();
    }

    if (role == ComponentRole)
    {
        return QVariant::fromValue((void*)componentsShowed_.at(index.row()));
    }

    if (role == CategoryRole)
    {
        auto* item = componentsShowed_.at(index.row());
        if(item)
        {
            return item->getCategory();
        }

        return QVariant();
    }

    return QVariant();
}

int ComponentModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return componentsShowed_.count();
}

int ComponentModel::columnCount(const QModelIndex& /*parent*/) const
{
    return Column::Column_Uplimit;
}

QVariant ComponentModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if(role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case ColumnCID: return QStringLiteral("ID");
            case ColumnName: return QStringLiteral("名称");
            case ColumnCategory: return QStringLiteral("类别");
            case ColumnComponentType: return QStringLiteral("类型");
            case ColumnTeam: return QStringLiteral("分组");
            case ColumnRole: return QStringLiteral("角色");
            case ColumnBehaviorType: return QStringLiteral("行为");
            case ColumnActionKeep: return QStringLiteral("置位信号\n保持时间(秒)");
            case ColumnSubscription: return QStringLiteral("订阅组件");
            case ColumnReference: return QStringLiteral("引用组件");
            case ColumnRespondBoardcast: return QStringLiteral("响应的\n广播码");
            case ColumnDefaultValue: return QStringLiteral("缺省值");
            case ColumnDescription: return QStringLiteral("描述");
            default:break;
            }
        }
        else if (orientation == Qt::Vertical)
        {
            return QString::number(section + 1);
        }
    }

    return QVariant();
}

Qt::ItemFlags ComponentModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return QAbstractTableModel::flags(index)|Qt::ItemIsEditable;
}

bool ComponentModel::appendComponent(const Jimmy::DesignComponent& component)
{
    if(!verifyComponent(component.getID(),component,true))
    {
        return false;
    }

    auto comp = new DesignComponent(component);
    ComponentsWhole_.append(comp);

    cidSet_.append(comp->getID());

    g_MainWindow_->setProjectStatus(ProjectStatus::modified);

    setFilter_();
    return true;
}

bool ComponentModel::modifyComponent(const QString& cid,const Jimmy::DesignComponent& component)
{
    foreach(auto& item, ComponentsWhole_)
    {
        if(item->getID() == cid)
        {
            if(!verifyComponent(cid,component,false))
            {
                return false;
            }

            if(cid != component.getID())
            {
                cidSet_.removeOne(cid);
                cidSet_.append(component.getID());
            }

            *item = component;
            g_MainWindow_->setProjectStatus(ProjectStatus::modified);
            setFilter_();
            return true;
        }
    }

    return false;
}

bool ComponentModel::eraseComponent(const QModelIndex &ind)
{
    auto item = itemFromIndex(ind);
    if (!item)
    {
        return false;
    }

    int row = componentsShowed_.indexOf(item);
    if(row == -1)
    {
        return false;
    }

    beginRemoveRows(QModelIndex(),row, row);
    componentsShowed_.removeAt(row);
    endRemoveRows();

    ComponentsWhole_.removeOne(item);
    cidSet_.removeOne(item->getID());

    delete item;
    item = nullptr;

    g_MainWindow_->setProjectStatus(ProjectStatus::modified);
    return true;
}

bool ComponentModel::hasComponents(const QString& category)
{
    foreach(auto item, ComponentsWhole_)
    {
        if(item->getCategory() == category)
        {
            return true;
        }
    }

    return false;
}

void ComponentModel::renameCategory(const QString& source,const QString& desc)
{
    bool bUpdate(false);
    foreach(auto item, ComponentsWhole_)
    {
        if(item->getCategory() == source)
        {
            item->setCategory(desc);
            bUpdate = true;
        }
    }

    if(bUpdate)
    {
        g_MainWindow_->setProjectStatus(ProjectStatus::modified);
    }
}

Jimmy::DesignComponent* ComponentModel::itemFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }

    return componentsShowed_.at(index.row());
}

Jimmy::DesignComponent* ComponentModel::getComponent(const QString& cid) const
{
    auto itor = std::find_if(ComponentsWhole_.begin(),ComponentsWhole_.end(),[&cid](const DesignComponent* core){
        return core->getID() == cid;
    });

    if(itor == ComponentsWhole_.end())
    {
        return nullptr;
    }

    return *itor;
}

const QVector<Jimmy::DesignComponent*>& ComponentModel::getAllComponents(bool showOnly) const
{
    return showOnly ? componentsShowed_:ComponentsWhole_;
}

bool ComponentModel::verifyComponent(const QString& cid,const Jimmy::DesignComponent& component,bool isAppend)
{
    if(!verifyCID(component.getID(),isAppend))
    {
        return false;
    }

    if(component.getType() == ComponentType::TeamMaster)
    {
        Q_FOREACH(auto& item, ComponentsWhole_)
        {
            if((item->getType() == ComponentType::TeamMaster) && (item->getID() != cid) && (item->getTeam() == component.getTeam()))
            {
                emit errorMessage(QStringLiteral("Team %1 is existed").arg(item->getTeam()));
                return false;
            }
        }
    }

    Q_FOREACH(auto& item, component.getSubscription())
    {
        if(!cidSet_.contains(item))
        {
            emit errorMessage(QStringLiteral("subscription %1 is not exist").arg(item));
            return false;
        }
    }

    Q_FOREACH(auto& item, component.getReference())
    {
        if(!cidSet_.contains(item))
        {
            emit errorMessage(QStringLiteral("Reference %1 is not exist").arg(item));
            return false;
        }
    }

    return true;
}

bool ComponentModel::verifyCID(const QString& cid,bool isAppend)
{
    if(isAppend)
    {
        if(cidSet_.contains(cid))
        {
            emit errorMessage(QStringLiteral("component %1 is existed").arg(cid));
            return false;
        }
    }

    bool ret = CommonFunction::validEngName(cid);
    if(!ret)
    {
        emit errorMessage(QStringLiteral("component %1 is invalid").arg(cid));
    }
    return true;
}


ErrorCode ComponentModel::resetComponents_(const QJsonObject& jo)
{
    cidSet_.clear();
    for(auto itor = jo.constBegin();itor!= jo.constEnd();++itor)
    {
        if(!itor->isObject())
        {
            QMessageBox::critical(g_MainWindow_, QStringLiteral("组件"), QStringLiteral("%1 format is invalid").arg(itor->toVariant().toString()));
            return ErrorCode::ec_error;
        }

        DesignComponent* core = new DesignComponent();
        core->setID(itor.key());

        QJsonObject jo = itor.value().toObject();

        auto elemItor = jo.find("name");
        if((elemItor == jo.end())||(!elemItor->isString()))
        {
            QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 name is failed").arg(core->getID()));
            return ErrorCode::ec_error;
        }
        core->setName(elemItor->toString());

        elemItor = jo.find("category");
        if((elemItor == jo.end())||(!elemItor->isString()))
        {
            QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 category is failed").arg(core->getID()));
            return ErrorCode::ec_error;
        }
        core->setCategory(elemItor->toString());

        elemItor = jo.find("component_type");
        if((elemItor == jo.end())||(!elemItor->isDouble()))
        {
            QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 component_type is failed").arg(core->getID()));
            return ErrorCode::ec_error;
        }
        core->setType(static_cast<ComponentType>(elemItor->toInt()));

        if ((core->getType() == Jimmy::ComponentType::TeamMaster) || (core->getType() == Jimmy::ComponentType::TeamSlave))
        {
			elemItor = jo.find("team");
			if ((elemItor == jo.end()) || (!elemItor->isString()))
			{
				QMessageBox::critical(g_MainWindow_, QStringLiteral("组件"), QStringLiteral("get %1 team is failed").arg(core->getID()));
				return ErrorCode::ec_error;
			}

            core->setTeam(elemItor->toString());
        }

        elemItor = jo.find("behavior_type");
        if((elemItor == jo.end())||(!elemItor->isDouble()))
        {
            QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 behavior_type is failed").arg(core->getID()));
            return ErrorCode::ec_error;
        }
        core->setBehavior(static_cast<BehaviorType>(elemItor->toInt()));

        if (core->getBehavior() == Jimmy::BehaviorType::Script)
        {
			elemItor = jo.find("role");
			if ((elemItor == jo.end()) || (!elemItor->isString()))
			{
				QMessageBox::critical(g_MainWindow_, QStringLiteral("组件"), QStringLiteral("get %1 role is failed").arg(core->getID()));
				return ErrorCode::ec_error;
			}
            core->setRole(elemItor->toString());
        }

        if(core->getType() == Jimmy::ComponentType::Input)
        {
            elemItor = jo.find("set_action_keep");
            if((elemItor == jo.end())||(!elemItor->isDouble()))
            {
                QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 set_action_keep is failed").arg(core->getID()));
                return ErrorCode::ec_error;
            }
            core->setActionKeep(elemItor->toInt());
        }

        elemItor = jo.find("default_value");
        if(elemItor == jo.end())
        {
            QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 default_value is failed").arg(core->getID()));
            return ErrorCode::ec_error;
        }
        core->setDefaultValue(*elemItor);

        elemItor = jo.find("description");
        if((elemItor == jo.end())||(!elemItor->isString()))
        {
            QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 description is failed").arg(core->getID()));
            return ErrorCode::ec_error;
        }
        core->setDescription(elemItor->toString());

        if(core->getType() != Jimmy::ComponentType::TeamSlave)
        {
            elemItor = jo.find("subscription");
            if((elemItor == jo.end())||(!elemItor->isArray()))
            {
                QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 subscription is failed").arg(core->getID()));
                return ErrorCode::ec_error;
            }

            auto subscription = CommonFunction::transform_jatosl(elemItor->toArray());
            if(!subscription)
            {
                QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 subscription member data type is invalid").arg(core->getID()));
                return ErrorCode::ec_error;
            }
            core->setSubscription(subscription.value());
        }

        if(!((core->getType() == Jimmy::ComponentType::Input) || (core->getType() == Jimmy::ComponentType::TeamSlave)))
        {
            elemItor = jo.find("reference");
            if((elemItor == jo.end())||(!elemItor->isArray()))
            {
                QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 reference is failed").arg(core->getID()));
                return ErrorCode::ec_error;
            }

            auto reference = CommonFunction::transform_jatosl(elemItor->toArray());
            if(!reference)
            {
                QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 reference member data type is invalid").arg(core->getID()));
                return ErrorCode::ec_error;
            }
            core->setReference(reference.value());

            elemItor = jo.find("respond_boardcast");
            if((elemItor == jo.end())||(!elemItor->isArray()))
            {
                QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 respond_boardcast is failed").arg(core->getID()));
                return ErrorCode::ec_error;
            }

            auto respondBoardcast = CommonFunction::transform_jatosl(elemItor->toArray());
            if(!respondBoardcast)
            {
                QMessageBox::critical(g_MainWindow_,QStringLiteral("组件"),QStringLiteral("get %1 respond_boardcast member data type is invalid").arg(core->getID()));
                return ErrorCode::ec_error;
            }
            core->setRespondBoardcast(respondBoardcast.value());

        }

        cidSet_.append(core->getID());
        ComponentsWhole_.push_back(core);
    }

    return ErrorCode::ec_ok;
}

Jimmy::ErrorCode  ComponentModel::setComponents(const QJsonObject& jo)
{
	beginResetModel();
	release();
    auto ret = resetComponents_(jo);
    if(ret != ErrorCode::ec_ok)
    {
        release();
    }
    endResetModel();

    return ret;
}

void ComponentModel::clearComponents()
{
    cidSet_.clear();

    beginResetModel();
    release();
    endResetModel();
}

QStringList ComponentModel::getAllCID()
{
    return cidSet_;
}

QStringList ComponentModel::getAllTeams()
{
    QStringList li;
    Q_FOREACH(auto item,ComponentsWhole_)
    {
        if(!item->getTeam().isEmpty())
        {
            if(li.contains(item->getTeam()))
            {
                continue;
            }
            li.push_back(item->getTeam());
        }
    }

    return li;

}

QStringList ComponentModel::getAllRoles()
{
    QStringList li;
    Q_FOREACH(auto item,ComponentsWhole_)
    {
        if(!item->getRole().isEmpty())
        {
            if(li.contains(item->getRole()))
            {
                continue;
            }
            li.push_back(item->getRole());
        }
    }

    return li;
}

QJsonObject ComponentModel::getComponents()
{
    QJsonObject jo;

    Q_FOREACH(auto item,ComponentsWhole_)
	{
        QJsonObject c;

        c.insert("name", item->getName());
        c.insert("category", item->getCategory());
        c.insert("component_type",static_cast<int>(item->getType()));

        switch(item->getType())
        {
        case Jimmy::ComponentType::Input:
        {
            c.insert("set_action_keep",item->getActionKeep());
            c.insert("subscription",QJsonArray::fromStringList(item->getSubscription()));
            break;
        }
        case Jimmy::ComponentType::Internal:
        case Jimmy::ComponentType::Output:
        {
            if(item->getBehavior() == Jimmy::BehaviorType::Script)
            {
                c.insert("role", item->getRole());
            }
            c.insert("subscription",QJsonArray::fromStringList(item->getSubscription()));
            c.insert("reference",QJsonArray::fromStringList(item->getReference()));
            c.insert("respond_boardcast",QJsonArray::fromStringList(item->getRespondBoardcast()));
            break;
        }
        case Jimmy::ComponentType::TeamMaster:
        {
            c.insert("team", item->getTeam());
            c.insert("role", item->getRole());
            c.insert("subscription",QJsonArray::fromStringList(item->getSubscription()));
            c.insert("reference",QJsonArray::fromStringList(item->getReference()));
            c.insert("respond_boardcast",QJsonArray::fromStringList(item->getRespondBoardcast()));
            break;
        }
        case Jimmy::ComponentType::TeamSlave:
        {
            c.insert("team", item->getTeam());
            break;
        }
        default:break;
        }

        c.insert("behavior_type",static_cast<int>(item->getBehavior()));
        c.insert("description",item->getDescription());
        c.insert("default_value",item->getDefaultValue());

        jo.insert(item->getID(),c);
	}
    return jo;
}

void ComponentModel::setFilter(const QStringList& filter)
{
    filterCategories_ = filter;
    setFilter_();
}

void ComponentModel::setFilter(ComponentFilterType cft,const QString& filter)
{
    cft_ = cft;
    filterComponents_ = filter;
    setFilter_();
}

void ComponentModel::setFilter_()
{
    QRegularExpression regexp(filterComponents_);
    if(cft_ == ComponentFilterType::ID)
    {
        regexp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }
    else
    {
        regexp.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
    }

    beginResetModel();
    componentsShowed_.clear();
    if(!filterCategories_.isEmpty())
    {
        Q_FOREACH(auto item,ComponentsWhole_)
        {
            if(filterCategories_.contains(item->getCategory()))
            {
                if(filterComponents_.isEmpty())
                {
                    componentsShowed_.push_back(item);
                }
                else
                {
                    QRegularExpressionMatch match;
                    if(cft_ == ComponentFilterType::ID)
                    {
                        match = regexp.match(item->getID());
                    }
                    else
                    {
                        match = regexp.match(item->getName());
                    }

                    if(match.hasMatch())
                    {
                        componentsShowed_.push_back(item);
                    }
                }
            }
        }
    }

    endResetModel();
}

QStringList ComponentModel::rescanCategory()
{
    QStringList li;
    Q_FOREACH(auto item,ComponentsWhole_)
    {
        if(!item->getCategory().isEmpty())
        {
            if(li.contains(item->getCategory()))
            {
                continue;
            }
            li.push_back(item->getCategory());
        }
    }

    return li;
}


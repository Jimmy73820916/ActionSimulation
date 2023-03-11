#pragma once

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

#include <QStyledItemDelegate>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QVariant>
#include <QString>
#include <QSet>
#include <qmetatype.h>
#include <QJsonObject>
#include "designcomponent.h"
#include "errorcode.h"

const int ComponentRole = Qt::UserRole + 1;
const int CategoryRole = ComponentRole + 1;

enum Column {
    ColumnCID = 0,
    ColumnName,
    ColumnCategory,
    ColumnComponentType,
    ColumnTeam,
    ColumnRole,
    ColumnBehaviorType,
    ColumnActionKeep,
    ColumnSubscription,
    ColumnReference,
    ColumnRespondBoardcast,
    ColumnDefaultValue,
    ColumnDescription,
    Column_Uplimit,
};

enum class ComponentFilterType
{
    ID,
    Name,
};


class ComponentModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ComponentModel(QObject* parent = nullptr);
    ~ComponentModel();

    void sort(int column,Qt::SortOrder order = Qt::AscendingOrder) override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;

    Jimmy::DesignComponent* itemFromIndex(const QModelIndex &index) const;

    Jimmy::DesignComponent* getComponent(const QString& id) const;
    const QVector<Jimmy::DesignComponent*>& getAllComponents(bool showOnly) const;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool appendComponent(const Jimmy::DesignComponent& component);
    bool modifyComponent(const QString& cid,const Jimmy::DesignComponent& component);
    bool eraseComponent(const QModelIndex &index);

    bool hasComponents(const QString& category);

    void renameCategory(const QString& source,const QString& desc);

    bool verifyComponent(const QString& cid,const Jimmy::DesignComponent& component,bool isAppend);

    Jimmy::ErrorCode setComponents(const QJsonObject& jo);
    QJsonObject getComponents();

    QStringList getAllCID();
    QStringList getAllTeams();
    QStringList getAllRoles();
	QStringList rescanCategory();
	
    void clearComponents();

    void setFilter(const QStringList& filter);
    void setFilter(ComponentFilterType cft,const QString& filter);
signals:
    void errorMessage(QString);
private:
    Jimmy::ErrorCode resetComponents_(const QJsonObject& jo);
    void release();

    bool verifyCID(const QString& cid,bool isAppend);
    void setFilter_();
private:
    QVector<Jimmy::DesignComponent*> ComponentsWhole_;
    QVector<Jimmy::DesignComponent*> componentsShowed_;
    QStringList filterCategories_;

    ComponentFilterType cft_;
    QString filterComponents_;

    QStringList cidSet_;
};



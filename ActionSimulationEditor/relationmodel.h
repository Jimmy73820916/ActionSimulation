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

#include "errorcode.h"
#include <QObject>
#include <QAbstractListModel>


const int RelationRole = Qt::UserRole;

class RelationItem:public QObject
{
    Q_OBJECT
public:
    RelationItem();
    RelationItem(const RelationItem&);
    ~RelationItem() = default;

    const int RID;
    QString name;
    int checkStatus; //Qt::Checked,Qt::Unchecked
};

class RelationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit RelationModel(QObject *parent = nullptr);
    ~RelationModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    RelationItem* itemFromIndex(const QModelIndex &index) const;
    QModelIndex getIndex(const RelationItem* item);

    Jimmy::ErrorCode setRelation(const QJsonArray& ja);
    QJsonArray getRelation();

    QModelIndex appendRelation();
    void removeRelation(const QModelIndex& index);

    void uncheckRelation(int RID);

    QModelIndex moveUp(const QModelIndex& index);
    QModelIndex moveDown(const QModelIndex& index);

    bool verifyRelation();
signals:
    void selectedRelationChanged(RelationItem*);
private:
    Jimmy::ErrorCode resetRelation_(const QJsonArray& jo);
    void release();


private:
    QList<RelationItem*> RelationItems_;
};


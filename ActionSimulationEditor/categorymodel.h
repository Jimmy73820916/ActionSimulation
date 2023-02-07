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

#include <QAbstractItemModel>
#include <QVariant>
#include <QString>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>

class CategoryModel;
class CategoryItem
{
    friend class CategoryModel;
public:
    CategoryItem(const QString& name,CategoryItem* parent = nullptr);
    ~CategoryItem();

    CategoryItem* find(const QString& name);
    int find(const CategoryItem* item);

    bool verifyCategory(QVector<QString>& names) const;

    void appendChild(CategoryItem* item);
    void removeChild(CategoryItem* item);
    void move(CategoryItem* newParent);

    void up();
    void down();

    void appendChildren(const QJsonObject& jo);
    void getChildren(QJsonObject& jo);
    void removeChildren();

    CategoryItem* child(int row);
    int childCount() const;

    CategoryItem* parentItem();

    QVariant getIcon();

    QString text();
    void setText(const QString& name);

    QVariant data(int role) const;
    void setData(int role, const QVariant& value);

    void getAllSelected(QStringList& childrenName);
    void getAllDescendant(QStringList& descendantName);

    int row() const;

    static void setCategoryModel(CategoryModel* filterModel);
private:
    static CategoryItem* clickSource_;
    static CategoryModel* filterModel_;
private:
    void takeChild(CategoryItem* item);
private:
    QVariant childrenCheckState() const;
private:
    QVector<CategoryItem*> childItems_;
    CategoryItem* parentItem_;
    QString name_;

    int checkStatus_; //Qt::Checked,Qt::Unchecked,Qt::PartiallyChecked
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CategoryModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    CategoryModel(QObject *parent = nullptr);
    ~CategoryModel();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,int role = Qt::EditRole) override;

    CategoryItem* itemFromIndex(const QModelIndex &index) const;
    QModelIndex index(const CategoryItem* item);


    void emitDataChanged(CategoryItem* item, const QVector<int>& roles);

    bool verifyNode();

    QModelIndex appendCategory(const QModelIndex& index);
    void appendCategory(const QString& category);
    void eraseCategory(const QModelIndex &index);

    void moveUp(const QModelIndex &index);
    void moveDown(const QModelIndex &index);

    void setCategories(const QJsonObject& jo);
    QJsonObject getCategories();

    void clear();

    void getSelected(QStringList& selectedFilters);
    void getAll(QStringList& categories);

    const CategoryItem* getProject() const;
    void appendMissing(const QStringList& categories);

    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
signals:
    void errorMessage(QString);
private:
    CategoryItem* root_;
    CategoryItem* project_;
private:
    static const QString RootCategory;
    static const QString UnloadProject;
};


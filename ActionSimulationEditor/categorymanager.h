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

#include <QWidget>
#include <QModelIndex>
#include <QJsonObject>
#include "categorymodel.h"

class QAction;
class QTreeView;
class QToolBar;

class CategoryManager:public QWidget
{
    Q_OBJECT
public:
    CategoryManager(QWidget *parent = nullptr);
    ~CategoryManager() = default;

    void setCategories(const QJsonObject& jo);
    QJsonObject getCategories();

    void clear();
    void getAll(QStringList& categories);

    void setToolBar(QToolBar* toolBar);

    const CategoryItem* getProject() const;

    void appendCategory(const QString& category);
signals:
    void selectedCategoriesChanged(QStringList);
    void errorMessage(QString);
public slots:
    void categoryTreeContextMenu(const QPoint& pos);
    void onSelectedCategoriesChanged(QModelIndex selected);
    void addCategory();
    void removeCategory();
    void renameCategory();
    void expandAll();
    void collapseAll();
    void actionUp();
    void actionDown();
    void closeCategoryTreeEditor();
private:
    QStringList recentSelected_;
    QTreeView*  categoryTree_;

    QString modifingCategory_;

    QAction* expandAll_;
    QAction* collapseAll_;
    QAction* actionUp_;
    QAction* actionDown_;
};


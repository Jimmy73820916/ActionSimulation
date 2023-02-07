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

#include "categorymanager.h"
#include "categorymodel.h"
#include "categorydelegate.h"
#include "mainwindow.h"
#include "qtsmalltools.h"
#include <QTreeView>
#include <QGridLayout>
#include <QDebug>
#include <QPoint>
#include <QMenu>
#include <QToolBar>
#include <QMessageBox>


using namespace std;
using namespace Jimmy;

CategoryManager::CategoryManager(QWidget *parent)
     : QWidget(parent)
{
    categoryTree_ = new QTreeView(this);
    categoryTree_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    categoryTree_->setHeaderHidden(true);
    categoryTree_->setSelectionBehavior(QAbstractItemView::SelectItems);
    categoryTree_->setRootIsDecorated(true);
    categoryTree_->setItemDelegateForColumn(0, new CategoryDelegate(this));
    categoryTree_->setContextMenuPolicy(Qt::CustomContextMenu);

    categoryTree_->setDragEnabled(true);
    categoryTree_->setDropIndicatorShown(true);
    categoryTree_->setDefaultDropAction(Qt::MoveAction);
    categoryTree_->setDragDropMode(QTreeView::DragDrop);
    categoryTree_->viewport()->setAcceptDrops(true);

    CategoryModel* categoryModel = new CategoryModel(categoryTree_);
    CategoryItem::setCategoryModel(categoryModel);
    categoryTree_->setModel(categoryModel);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(categoryTree_);
    layout->setContentsMargins(2,2,2,2);

    setLayout(layout);

    connect(categoryTree_,&QTreeView::customContextMenuRequested, this,&CategoryManager::categoryTreeContextMenu);
    connect(categoryTree_,SIGNAL(clicked(QModelIndex)), this,SLOT(onSelectedCategoriesChanged(QModelIndex)));

    connect(categoryModel, SIGNAL(errorMessage(QString)),this, SIGNAL(errorMessage(QString)));


    expandAll_ = new QAction(QStringLiteral("全部展开"), this);
    collapseAll_ = new QAction(QStringLiteral("全部折叠"), this);
    actionUp_ = new QAction(QStringLiteral("上移"), this);
    actionDown_ = new QAction(QStringLiteral("下移"), this);

    expandAll_->setIcon(QIcon(QTSmalltools::svgIcon(":/image/image/expand.svg")));
    collapseAll_->setIcon(QIcon(QTSmalltools::svgIcon(":/image/image/collapse.svg")));

    actionUp_->setIcon(QIcon(QTSmalltools::svgIcon(":/image/image/up.svg")));
    actionDown_->setIcon(QIcon(QTSmalltools::svgIcon(":/image/image/down.svg")));

    connect(expandAll_, SIGNAL(triggered()),this, SLOT(expandAll()));
    connect(collapseAll_, SIGNAL(triggered()),this, SLOT(collapseAll()));
    connect(actionUp_, SIGNAL(triggered()),this, SLOT(actionUp()));
    connect(actionDown_, SIGNAL(triggered()),this, SLOT(actionDown()));
}

void CategoryManager::setToolBar(QToolBar* toolBar)
{
    toolBar->addAction(expandAll_);
    toolBar->addAction(collapseAll_);
    toolBar->addSeparator();
    toolBar->addAction(actionUp_);
    toolBar->addAction(actionDown_);
}

void  CategoryManager::onSelectedCategoriesChanged(QModelIndex /*selected*/)
{
    QStringList currentSelected;
    static_cast<CategoryModel*>(categoryTree_->model())->getSelected(currentSelected);

    if(recentSelected_ != currentSelected)
    {
        recentSelected_ = currentSelected;
        emit selectedCategoriesChanged(recentSelected_);
    }
}

void CategoryManager::categoryTreeContextMenu(const QPoint& pos)
{
    if(g_MainWindow_->getProjectStatus() == ProjectStatus::invalid)
    {
        return;
    }

    QModelIndex curIndex = categoryTree_->indexAt(pos);
    if (!curIndex.isValid())
    {
        return;
    }

    QMenu menu;
    menu.addAction(QIcon(QTSmalltools::svgIcon(":/image/image/append.svg")),QStringLiteral("添加类别"), this, SLOT(addCategory()));
    menu.addAction(QIcon(QTSmalltools::svgIcon(":/image/image/rename.svg")),QStringLiteral("重命名类别"), this, SLOT(renameCategory()));
    menu.addAction(QIcon(QTSmalltools::svgIcon(":/image/image/remove.svg")), QStringLiteral("删除类别"), this, SLOT(removeCategory()));
    menu.exec(QCursor::pos());
}

void CategoryManager::appendCategory(const QString& category)
{
    static_cast<CategoryModel*>(categoryTree_->model())->appendCategory(category);
}

void CategoryManager::addCategory()
{
    QModelIndex curIndex = categoryTree_->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0);

    if(index.isValid())
    {
        QStringList categories;
        getAll(categories);

        auto newIndex = static_cast<CategoryModel*>(categoryTree_->model())->appendCategory(index);
        if (newIndex.isValid())
        {
            categoryTree_->expand(index);
            static_cast<CategoryDelegate*>(categoryTree_->itemDelegateForColumn(0))->setMemberSet(std::move(categories));
            categoryTree_->openPersistentEditor(newIndex);
            auto editor = categoryTree_->indexWidget(newIndex);
            editor->setFocus();
            if(editor)
            {
                connect(editor,SIGNAL(editingFinished()),this,SLOT(closeCategoryTreeEditor()));
            }
        }
    }
}

void CategoryManager::removeCategory()
{
    QModelIndex curIndex = categoryTree_->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0);

    if(index.isValid())
    {
        if(QMessageBox::Ok == QMessageBox::information(this,QStringLiteral("删除类别"),QStringLiteral("确认要删除吗?"),QMessageBox::Ok,QMessageBox::No))
        {
            CategoryItem* item = static_cast<CategoryModel*>(categoryTree_->model())->itemFromIndex(index);
            if(item)
            {
                if((item->childCount() > 0)||(g_MainWindow_->hasComponents(item->text())))
                {
                    QMessageBox::critical(this,QStringLiteral("删除类别"),QStringLiteral("不允许删除非空类别"));
                    return;
                }

                static_cast<CategoryModel*>(categoryTree_->model())->eraseCategory(curIndex);
            }
        }
    }
}

void CategoryManager::renameCategory()
{
    QModelIndex curIndex = categoryTree_->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0);

    if(index.isValid())
    {
        QStringList categories;
        getAll(categories);

        CategoryItem* item = static_cast<CategoryModel*>(categoryTree_->model())->itemFromIndex(index);
        if(item)
        {
            categories.removeOne(item->text());
        }
        static_cast<CategoryDelegate*>(categoryTree_->itemDelegateForColumn(0))->setMemberSet(std::move(categories));
        categoryTree_->openPersistentEditor(index);

        auto editor = categoryTree_->indexWidget(index);
        editor->setFocus();
        if(editor)
        {
            modifingCategory_ = item->text();
            connect(editor,SIGNAL(editingFinished()),this,SLOT(closeCategoryTreeEditor()));
        }
    }
}

void CategoryManager::closeCategoryTreeEditor()
{
    QModelIndex curIndex = categoryTree_->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0);
    categoryTree_->closePersistentEditor(index);

    if(!static_cast<CategoryModel*>(categoryTree_->model())->verifyNode())
    {
        static_cast<CategoryModel*>(categoryTree_->model())->eraseCategory(index);
    }

    if(!modifingCategory_.isEmpty())
    {
        CategoryItem* item = static_cast<CategoryModel*>(categoryTree_->model())->itemFromIndex(index);
        if(item)
        {
            if(item->text() != modifingCategory_)
            {
                g_MainWindow_->renameCategory(modifingCategory_,item->text());
                onSelectedCategoriesChanged(QModelIndex());
            }
        }

        modifingCategory_.clear();
    }

    g_MainWindow_->setProjectStatus(ProjectStatus::modified);

}

void CategoryManager::expandAll()
{
    if(g_MainWindow_->getProjectStatus() == ProjectStatus::invalid)
    {
        return;
    }

    categoryTree_->expandAll();
}

void CategoryManager::collapseAll()
{
    if(g_MainWindow_->getProjectStatus() == ProjectStatus::invalid)
    {
        return;
    }

    categoryTree_->collapseAll();
}

void CategoryManager::actionUp()
{
    if(g_MainWindow_->getProjectStatus() == ProjectStatus::invalid)
    {
        return;
    }

    QModelIndex curIndex = categoryTree_->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0);

    if(index.isValid())
    {
        static_cast<CategoryModel*>(categoryTree_->model())->moveUp(index);
    }
}

void CategoryManager::actionDown()
{
    if(g_MainWindow_->getProjectStatus() == ProjectStatus::invalid)
    {
        return;
    }

    QModelIndex curIndex = categoryTree_->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0);

    if(index.isValid())
    {
        static_cast<CategoryModel*>(categoryTree_->model())->moveDown(index);
    }
}

void CategoryManager::setCategories(const QJsonObject& ja)
{
    static_cast<CategoryModel*>(categoryTree_->model())->setCategories(ja);
}

QJsonObject CategoryManager::getCategories()
{
    return static_cast<CategoryModel*>(categoryTree_->model())->getCategories();
}

void CategoryManager::clear()
{
    static_cast<CategoryModel*>(categoryTree_->model())->clear();
}

void CategoryManager::getAll(QStringList& categories)
{
    static_cast<CategoryModel*>(categoryTree_->model())->getAll(categories);
}

const CategoryItem* CategoryManager::getProject() const
{
    return static_cast<CategoryModel*>(categoryTree_->model())->getProject();
}

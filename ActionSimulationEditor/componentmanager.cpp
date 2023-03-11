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

#include <QTableView>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QAction>
#include <QToolBar>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QHeaderView>
#include <QInputDialog>
#include "componentmanager.h"
#include "mainwindow.h"
#include "qtsmalltools.h"
#include "frmtransferexcel.h"
#include <QMessageBox>

using namespace std;
using namespace Jimmy;

ComponentManager::ComponentManager(QWidget *parent)
    : QWidget(parent)
    ,componentDialog_(nullptr)
{
    componentList_ = new QTableView();

    componentList_->setContextMenuPolicy(Qt::CustomContextMenu);
    componentList_->setAcceptDrops(false);
    componentList_->setSelectionMode(QAbstractItemView::SingleSelection);
    componentList_->setSelectionBehavior(QAbstractItemView::SelectRows);
    componentList_->setAlternatingRowColors(true);
    componentList_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    componentList_->setSortingEnabled(true);
    componentList_->horizontalHeader()->setStretchLastSection(true);

    QGridLayout* layout = new QGridLayout();
    layout->addWidget(componentList_);
    layout->setContentsMargins(2,2,2,2);

    setLayout(layout);

    ComponentModel* componentModel = new ComponentModel(componentList_);
    componentList_->setModel(componentModel);

    cbCFT_ = new QComboBox(this);
    cbCFT_->addItem(QStringLiteral("CID"));
    cbCFT_->addItem(QStringLiteral("Name"));
    cbCFT_->setFixedWidth(50);
    cbCFT_->setCurrentIndex(0);

    leSearchComponent_ = new QLineEdit(this);
    leSearchComponent_->setFixedWidth(200);
    leSearchComponent_->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    leSearchComponent_->setFocusPolicy(Qt::ClickFocus);

    actionSearch_ = new QAction(this);
    actionSearch_->setIcon(QIcon(QTSmalltools::svgIcon(":/image/image/refresh.svg")));

    import_ = new QAction(QStringLiteral("从 Excel 导入"), this);
    export_ = new QAction(QStringLiteral("导出到 Excel"), this);

    import_->setIcon(QIcon(QTSmalltools::svgIcon(":/image/image/import.svg")));
    export_->setIcon(QIcon(QTSmalltools::svgIcon(":/image/image/export.svg")));

    connect(import_, SIGNAL(triggered()),this, SLOT(importFromExcel()));
    connect(export_, SIGNAL(triggered()),this, SLOT(exportFromExcel()));

    connect(componentList_,&QTableView::customContextMenuRequested, this,&ComponentManager::componentListContextMenu);
    connect(componentList_,&QTableView::doubleClicked, this,&ComponentManager::componentListdoubleClicked);


    connect(leSearchComponent_, SIGNAL(editingFinished()), this, SLOT(findComponent()));
    connect(actionSearch_, SIGNAL(triggered()), this, SLOT(findComponent()));

    connect(componentModel, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));

    connect(componentList_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)));
}

void ComponentManager::selectedCategoriesChanged(QStringList selected)
{
    static_cast<ComponentModel*>(componentList_->model())->setFilter(selected);
}

void ComponentManager::setComponents(const QJsonObject& jo)
{
    static_cast<ComponentModel*>(componentList_->model())->setComponents(jo);
    selectedCategoriesChanged(QStringList());
}

QJsonObject ComponentManager::getComponents()
{
    return static_cast<ComponentModel*>(componentList_->model())->getComponents();
}

void ComponentManager::clearComponents()
{
    static_cast<ComponentModel*>(componentList_->model())->clearComponents();
}

void ComponentManager::setToolBar(QToolBar* toolBar)
{
    toolBar->addWidget(cbCFT_);
    toolBar->addWidget(leSearchComponent_);
    toolBar->addAction(actionSearch_);
    toolBar->addSeparator();
    toolBar->addAction(import_);
    toolBar->addAction(export_);
}

void ComponentManager::selectionChanged(const QItemSelection &selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
}

void ComponentManager::findComponent()
{
    static_cast<ComponentModel*>(componentList_->model())->setFilter((cbCFT_->currentIndex()==0)?ComponentFilterType::ID:ComponentFilterType::Name,leSearchComponent_->text());
}

void ComponentManager::componentListContextMenu(const QPoint& pos)
{
    if(g_MainWindow_->getProjectStatus() == ProjectStatus::invalid)
    {
        return;
    }

    QMenu menu;
    QModelIndex curIndex = componentList_->indexAt(pos);
    if (curIndex.isValid())
    {
        if(auto item = static_cast<ComponentModel*>(componentList_->model())->itemFromIndex(curIndex))
        {
            menu.addAction(QIcon(QTSmalltools::svgIcon(":/image/image/append.svg")),QStringLiteral("添加组件"), this, SLOT(appendComponent()));
            menu.addAction(QIcon(QTSmalltools::svgIcon(":/image/image/modify.svg")),QStringLiteral("编辑组件"), this, SLOT(modifyComponent()));
            menu.addAction(QIcon(QTSmalltools::svgIcon(":/image/image/remove.svg")), QStringLiteral("移除组件"), this, SLOT(removeComponent()));
        }
    }
    else
    {
        menu.addAction(QIcon(QTSmalltools::svgIcon(":/image/image/append.svg")),QStringLiteral("添加组件"), this, SLOT(appendComponent()));
    }

    menu.exec(QCursor::pos());
}

void ComponentManager::componentListdoubleClicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        if(auto item = static_cast<ComponentModel*>(componentList_->model())->itemFromIndex(index))
        {
            if(!componentDialog_)
            {
                componentDialog_ = new ComponentDialog(this);
                componentDialog_->setWindowTitle(QStringLiteral("编辑设备"));
                componentDialog_->setModifiedComponent(item);

                connect(componentDialog_,&ComponentDialog::destroyed,[this]{componentDialog_ = nullptr;});

                componentDialog_->show();
            }
        }
    }
}

void ComponentManager::appendComponent()
{
    if(!componentDialog_)
    {
        componentDialog_ = new ComponentDialog(this);
        componentDialog_->setWindowTitle(QStringLiteral("添加设备"));
        connect(componentDialog_,&ComponentDialog::destroyed,[this]{componentDialog_ = nullptr;});
        componentDialog_->show();
    }
}

void ComponentManager::modifyComponent()
{
    QModelIndex curIndex = componentList_->currentIndex();
    if (!curIndex.isValid())
    {
        return;
    }

    auto componentModel = static_cast<ComponentModel*>(componentList_->model());
    auto item = componentModel->itemFromIndex(curIndex);
    if(item)
    {
        if(!componentDialog_)
        {
            componentDialog_ = new ComponentDialog(this);
            componentDialog_->setWindowTitle(QStringLiteral("编辑设备"));
            componentDialog_->setModifiedComponent(item);
            connect(componentDialog_,&ComponentDialog::destroyed,[this]{componentDialog_ = nullptr;});
            componentDialog_->show();
        }
    }
}

void ComponentManager::removeComponent()
{
    QModelIndex curIndex = componentList_->currentIndex();
    if (!curIndex.isValid())
    {
        return;
    }

    auto componentModel = static_cast<ComponentModel*>(componentList_->model());
    auto item = componentModel->itemFromIndex(curIndex);
    if(QMessageBox::Ok == QMessageBox::information(this,QStringLiteral("删除设备"),QStringLiteral("确认要删除设备 %1 吗?").arg(item->getID()),QMessageBox::Ok,QMessageBox::No))
    {
        componentModel->eraseComponent(curIndex);
    }
}

bool ComponentManager::appendComponent(const DesignComponent& component)
{
    return static_cast<ComponentModel*>(componentList_->model())->appendComponent(component);
}

bool ComponentManager::hasComponents(const QString& category)
{
    return static_cast<ComponentModel*>(componentList_->model())->hasComponents(category);
}

bool ComponentManager::modifyComponent(const QString& cid,const DesignComponent& component)
{
    return static_cast<ComponentModel*>(componentList_->model())->modifyComponent(cid,component);
}

void ComponentManager::renameCategory(const QString& source,const QString& desc)
{
    return static_cast<ComponentModel*>(componentList_->model())->renameCategory(source,desc);
}

DesignComponent* ComponentManager::getComponent(const QString& componentName)
{
    return static_cast<ComponentModel*>(componentList_->model())->getComponent(componentName);
}

const QVector<DesignComponent*>& ComponentManager::getAllComponents(bool showOnly) const
{
    return static_cast<ComponentModel*>(componentList_->model())->getAllComponents(showOnly);
}

QStringList ComponentManager::getAllCID()
{
    return static_cast<ComponentModel*>(componentList_->model())->getAllCID();
}

QStringList ComponentManager::getAllTeams()
{
    return static_cast<ComponentModel*>(componentList_->model())->getAllTeams();
}

QStringList ComponentManager::getAllRoles()
{
    return static_cast<ComponentModel*>(componentList_->model())->getAllRoles();
}

QStringList ComponentManager::rescanCategory()
{
    return static_cast<ComponentModel*>(componentList_->model())->rescanCategory();
}

void ComponentManager::importFromExcel()
{
    if(g_MainWindow_->getProjectStatus() == ProjectStatus::invalid)
    {
        return;
    }

    FrmTransferExcel frmTransferExcel(this);
    frmTransferExcel.setWindowTitle(QStringLiteral("从 Excel 导入"));
    frmTransferExcel.setTransferType(TransferType::Import);
    frmTransferExcel.exec();
}

void ComponentManager::exportFromExcel()
{
    if(g_MainWindow_->getProjectStatus() == ProjectStatus::invalid)
    {
        return;
    }

    FrmTransferExcel frmTransferExcel(this);
    frmTransferExcel.setWindowTitle(QStringLiteral("导出到 Excel"));
    frmTransferExcel.setTransferType(TransferType::Export);
    frmTransferExcel.exec();

}

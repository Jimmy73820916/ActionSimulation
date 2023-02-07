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
#include <QTableView>
#include <QItemSelection>
#include <QRegularExpression>
#include <QJsonObject>
#include "componentmodel.h"
#include "componentdialog.h"

class ComponentModel;
class QSortFilterProxyModel;
class QLineEdit;
class QComboBox;
class QToolBar;
class QLabel;

class ComponentManager:public QWidget
{
    Q_OBJECT
public:
    ComponentManager(QWidget *parent = nullptr);
    ~ComponentManager() = default;

    void setComponents(const QJsonObject& jo);
    QJsonObject getComponents();

    void clearComponents();
    void setToolBar(QToolBar* toolBar);

    bool appendComponent(const Jimmy::DesignComponent& component);
    bool modifyComponent(const QString& cid,const Jimmy::DesignComponent& component);

    void renameCategory(const QString& source,const QString& desc);

    Jimmy::DesignComponent* getComponent(const QString& cid);
    const QVector<Jimmy::DesignComponent*>& getAllComponents() const;

    QStringList getAllCID();
    QStringList getAllTeams();
    QStringList getAllRoles();

    bool hasComponents(const QString& category);
signals:
    void errorMessage(QString);
    void selectedComponentChanged(Jimmy::DesignComponent*);
    void sendInputChange(QString cid);
    void saveComponents();
public slots:
    void findComponent();
    void selectedCategoriesChanged(QStringList selected);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void componentListContextMenu(const QPoint& pos);
    void componentListdoubleClicked(const QModelIndex &index);

    void appendComponent();
    void modifyComponent();
    void removeComponent();

    void importFromExcel();
    void exportFromExcel();
private:
    QTableView*  componentList_;

    QAction* import_;
    QAction* export_;

    QComboBox* cbCFT_;
    QLineEdit* leSearchComponent_;
    QAction* actionSearch_;

    ComponentDialog* componentDialog_;

};


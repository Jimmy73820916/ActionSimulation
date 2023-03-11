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

#include <QMainWindow>
#include <QPointer>

#include <QJsonObject>
#include <QHash>

#include "DockManager.h"
#include "DockWidget.h"

#include "designcomponent.h"
#include "categorymanager.h"
#include "componentmanager.h"
#include "relationmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class ProjectStatus
{
    invalid,			//无效的状态
    editing,            //编辑中
    modified,           //已更改
};

class RelationItem;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setProjectStatus(ProjectStatus projectStatus);
    ProjectStatus getProjectStatus();

    void getAllCategories(QStringList& categories);
    const CategoryItem* getProject() const;

    void registerSlotsFunctions();

    void appendCategory(const QString& category);
    void renameCategory(const QString& source,const QString& desc);
    bool appendComponent(const Jimmy::DesignComponent& component);
    bool modifyComponent(const QString& cid,const Jimmy::DesignComponent& component);
    bool hasComponents(const QString& category);

    Jimmy::DesignComponent* getComponent(const QString& cid);
    const QVector<Jimmy::DesignComponent*>& getAllComponents() const;

    QStringList getAllCID();
    QStringList getAllTeams();
    QStringList getAllRoles();
protected:
    virtual void closeEvent(QCloseEvent* event) override;
public slots:
    //menu
    void onActionCreateProject();
    void onActionOpenProject();
    void onActionSaveProject();
    //message
    void errorMessage(QString message);

    void selectedRelationChanged(RelationItem* item);
    void onRelationViewerCloseRequested();
private:
    void saveLayout_();
    void restoreLayout_();
    QString loadConfigFile_();
    void saveConfigFile_();
private:
    void updateMenuStatus();
    Jimmy::ErrorCode openProject(const QString& fileFullPath);
    void setProjectName(const QString& fileFullPath);
private:
    CategoryManager* categoryManager_;
    ads::CDockWidget* createCategoryManagerDockWidget();

    ComponentManager* componentManager_;
    ads::CDockWidget* createComponentManagerDockWidget();

    //关系
    RelationManager* relationManager_;
    ads::CDockWidget* createRelationManagerDockWidget();
    ads::CDockWidget* createRelationViewerDockWidget(const RelationItem* item,const QJsonObject& jo);

    void createDockingBar();

    ads::CDockManager* DockManager_;
    QVector<ads::CDockWidget*> dockWidgets_;
private:
    void createErrorInfoMessageBar();
private:
    Ui::MainWindow *ui;

    QLabel* lblErrorInfo_;

    ProjectStatus projectStatus_;
    QString projectName_;

    QJsonObject     json_general_;
    QJsonObject     json_relation_;
};

extern MainWindow* g_MainWindow_;

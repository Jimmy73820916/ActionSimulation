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

#include <QMenu>
#include <QFile>
#include <QToolButton>
#include <QToolBar>
#include <QScreen>
#include <QSettings>
#include <QLabel>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QStyleFactory>
#include <QDateTime>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QFileDialog>
#include <QInputDialog>
#include <QRegularExpression>
#include "frmcreateproject.h"

#include "DockAreaWidget.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "categorymanager.h"
#include "relationmanager.h"
#include "relationmodel.h"
#include "relationviewer.h"
#include "relationviewer/nodestyle.h"
#include "relationviewer/relationviewstyle.h"
#include "relationviewer/connectionstyle.h"

using namespace ads;
using namespace std;
using namespace Jimmy;

MainWindow* g_MainWindow_ = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , projectStatus_(ProjectStatus::invalid)

{
    ui->setupUi(this);
    ui->actionSaveProject->setEnabled(projectStatus_ == ProjectStatus::modified);

    setProjectStatus(ProjectStatus::invalid);

    createErrorInfoMessageBar();

    loadConfigFile_();

    createDockingBar();

    setWindowState(Qt::WindowMaximized);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, frameSize(),QGuiApplication::primaryScreen()->availableGeometry()));

    registerSlotsFunctions();

    restoreLayout_();
}

void MainWindow::createErrorInfoMessageBar()
{
    lblErrorInfo_ = new QLabel(this);
    QSize size = lblErrorInfo_->sizeHint();
    size.setWidth(500);
    lblErrorInfo_->setMinimumSize(size);
    lblErrorInfo_->setAlignment(Qt::AlignHCenter);
    this->statusBar()->addWidget(lblErrorInfo_);
}

void MainWindow::errorMessage(QString message)
{
    lblErrorInfo_->setText(message);
}

void MainWindow::registerSlotsFunctions()
{
    //menu
    connect(ui->actionCreateProject, SIGNAL(triggered()),this, SLOT(onActionCreateProject()));
    connect(ui->actionOpenProject, SIGNAL(triggered()),this, SLOT(onActionOpenProject()));
    connect(ui->actionSaveProject, SIGNAL(triggered()),this, SLOT(onActionSaveProject()));

    //dock
    connect(categoryManager_,SIGNAL(selectedCategoriesChanged(QStringList)), componentManager_,SLOT(selectedCategoriesChanged(QStringList)));
    connect(relationManager_,SIGNAL(selectedRelationChanged(RelationItem*)), this,SLOT(selectedRelationChanged(RelationItem*)));
}


void MainWindow::updateMenuStatus()
{
    switch (projectStatus_)
    {
    case ProjectStatus::invalid:
    case ProjectStatus::editing:
    {
        ui->actionSaveProject->setEnabled(false);
        break;
    }
    case ProjectStatus::modified:
    {
        ui->actionSaveProject->setEnabled(true);
        break;
    }
    }
}

void MainWindow::setProjectStatus(ProjectStatus projectStatus)
{
    projectStatus_ = projectStatus;
    if(projectStatus_ == ProjectStatus::modified)
    {
        setWindowTitle(QStringLiteral("%1    *%2").arg(QApplication::applicationName(),projectName_));
    }
    else if(projectStatus_ == ProjectStatus::editing)
    {
        setWindowTitle(QStringLiteral("%1    %2").arg(QApplication::applicationName(),projectName_));
    }
    updateMenuStatus();
}

ProjectStatus  MainWindow::getProjectStatus()
{
    return projectStatus_;
}

void MainWindow::getAllCategories(QStringList& categories)
{
    categoryManager_->getAll(categories);
}

const CategoryItem* MainWindow::getProject() const
{
    return categoryManager_->getProject();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    componentManager_->clearComponents();
    categoryManager_->clear();

    saveLayout_();
    DockManager_->deleteLater();
    QMainWindow::closeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::appendCategory(const QString& category)
{
    return categoryManager_->appendCategory(category);
}

void MainWindow::renameCategory(const QString& source,const QString& desc)
{
    return componentManager_->renameCategory(source,desc);
}

bool MainWindow::appendComponent(const Jimmy::DesignComponent& component)
{
    return componentManager_->appendComponent(component);
}

bool MainWindow::modifyComponent(const QString& cid,const Jimmy::DesignComponent& component)
{
    return componentManager_->modifyComponent(cid,component);
}

bool MainWindow::hasComponents(const QString& category)
{
    return componentManager_->hasComponents(category);
}

Jimmy::DesignComponent* MainWindow::getComponent(const QString& componentName)
{
    if (componentManager_)
    {
        return componentManager_->getComponent(componentName);
    }

    return nullptr;
}

const QVector<Jimmy::DesignComponent*>& MainWindow::getAllComponents(bool showOnly) const
{
    if (componentManager_)
    {
        return componentManager_->getAllComponents(showOnly);
    }

    static QVector<Jimmy::DesignComponent*> temp;
    return temp;
}

QStringList MainWindow::getAllCID()
{
    if (componentManager_)
    {
        return componentManager_->getAllCID();
    }

    return QStringList();
}

QStringList MainWindow::getAllTeams()
{
    if (componentManager_)
    {
        return componentManager_->getAllTeams();
    }

    return QStringList();
}

QStringList MainWindow::getAllRoles()
{
    if (componentManager_)
    {
        return componentManager_->getAllRoles();
    }

    return QStringList();
}

QStringList MainWindow::rescanCategory()
{
    if (componentManager_)
    {
        return componentManager_->rescanCategory();
    }

    return QStringList();
}

void MainWindow::createDockingBar()
{
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    CDockManager::setConfigFlags(CDockManager::DefaultOpaqueConfig);

    DockManager_ = new CDockManager(this);

    auto componentDockWidget = createComponentManagerDockWidget();
    ui->menu_W->addAction(componentDockWidget->toggleViewAction());
    DockManager_->addDockWidget(ads::TopDockWidgetArea, componentDockWidget);

    auto categoryDockWidget = createCategoryManagerDockWidget();
    ui->menu_W->addAction(categoryDockWidget->toggleViewAction());
    DockManager_->addDockWidget(ads::LeftDockWidgetArea, categoryDockWidget);

    auto relationDockWidget = createRelationManagerDockWidget();
    ui->menu_W->addAction(relationDockWidget->toggleViewAction());
    DockManager_->addDockWidget(ads::LeftDockWidgetArea, relationDockWidget);
}

ads::CDockWidget* MainWindow::createCategoryManagerDockWidget()
{
    categoryManager_ = new CategoryManager();
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QStringLiteral("类别"));

    DockWidget->setWidget(categoryManager_);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, false);
    categoryManager_->setFocusPolicy(Qt::NoFocus);
    categoryManager_->setToolBar(DockWidget->createDefaultToolBar());

    return DockWidget;
}

ads::CDockWidget* MainWindow::createRelationManagerDockWidget()
{
    relationManager_ = new RelationManager();
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QStringLiteral("订阅关系"));

    DockWidget->setWidget(relationManager_);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, false);
    relationManager_->setFocusPolicy(Qt::NoFocus);
    relationManager_->setToolBar(DockWidget->createDefaultToolBar());

    return DockWidget;
}

ads::CDockWidget* MainWindow::createRelationViewerDockWidget(const RelationItem* item,const QJsonObject& jo)
{
    RelationViewer* relationViewer = new RelationViewer(item,jo);
    ads::CDockWidget* DockWidget = new ads::CDockWidget(item->name);
    DockWidget->setWidget(relationViewer);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    relationViewer->setFocusPolicy(Qt::NoFocus);
    relationViewer->setToolBar(DockWidget->createDefaultToolBar());

    connect(DockWidget, SIGNAL(closeRequested()),this,SLOT(onRelationViewerCloseRequested()));

    return DockWidget;
}

ads::CDockWidget* MainWindow::createComponentManagerDockWidget()
{
    componentManager_ = new ComponentManager();
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QStringLiteral("组件"));
    DockWidget->setWidget(componentManager_);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, false);
    componentManager_->setFocusPolicy(Qt::NoFocus);
    componentManager_->setToolBar(DockWidget->createDefaultToolBar());

    return DockWidget;
}

void MainWindow::saveLayout_()
{
    QSettings Settings(QString("%1/Settings.ini").arg(qApp->applicationDirPath()), QSettings::IniFormat);
    Settings.setValue("mainWindow/Geometry", saveGeometry());
    Settings.setValue("mainWindow/State", saveState());
    Settings.setValue("mainWindow/DockingState", DockManager_->saveState());
}

void MainWindow::restoreLayout_()
{
    QSettings Settings(QString("%1/Settings.ini").arg(qApp->applicationDirPath()), QSettings::IniFormat);
    restoreGeometry(Settings.value("mainWindow/Geometry").toByteArray());
    restoreState(Settings.value("mainWindow/State").toByteArray());
    DockManager_->restoreState(Settings.value("mainWindow/DockingState").toByteArray());
}

QString MainWindow::loadConfigFile_()
{
    QString configFile = qApp->applicationFilePath();
    configFile.replace(configFile.length()-3,3,"json");

    QFile file(configFile.toLocal8Bit());
    if(!file.open(QIODevice::ReadOnly))
    {
        return QStringLiteral("load %1 is failed").arg(configFile);
    }

    QJsonParseError error;
    QJsonDocument jd = QJsonDocument::fromJson(file.readAll(),&error);

    if(error.error!=QJsonParseError::NoError)
    {
        return QStringLiteral("%1 is not json file").arg(configFile);
    }

    QJsonObject jo = jd.object();

    auto elemIter = jo.find("relation_style");
    if ((elemIter == jo.end() || (!elemIter->isObject())))
    {
        return QStringLiteral("relationStyle key is invalid").arg(configFile);
    }

    auto elemRelationStyle = elemIter->toObject();
    if (elemRelationStyle.contains("view_style") && elemRelationStyle["view_style"].isObject())
    {
        Jimmy::Relation::RelationViewStyle::getInstance()->setRelationViewStyle(elemRelationStyle["view_style"].toObject());
    }
    else
    {
        return QStringLiteral("ViewStyle key is invalid").arg(configFile);
    }

    if (elemRelationStyle.contains("connection_style") && elemRelationStyle["connection_style"].isObject())
    {
        Jimmy::Relation::ConnectionStyle::getInstance()->setConnectionStyle(elemRelationStyle["connection_style"].toObject());
    }
    else
    {
        return QStringLiteral("ConnectionStyle key is invalid").arg(configFile);
    }

    if (elemRelationStyle.contains("node_style") && elemRelationStyle["node_style"].isObject())
    {
        Jimmy::Relation::NodeStyle::getInstance()->setNodeStyle(elemRelationStyle["node_style"].toObject());
    }
    else
    {
        return QStringLiteral("NodeStyle key is invalid").arg(configFile);
    }

    return "";
}


void MainWindow::onActionCreateProject()
{
    frmCreateProject createProject(this);
    createProject.exec();
    if(createProject.getProjectInfo().exists())
    {
        if(openProject(createProject.getProjectInfo().absoluteFilePath())==Jimmy::ec_ok)
        {
            setProjectName(createProject.getProjectInfo().absoluteFilePath());
            setProjectStatus(ProjectStatus::editing);
        }
    }
}

void MainWindow::onActionOpenProject()
{
    QString projectPath(QStringLiteral("%1\\Projects").arg(QDir::currentPath()));
    auto fileName = QFileDialog::getOpenFileName(this,QStringLiteral("打开项目"),projectPath,QStringLiteral("json格式(*.json)"));
    if(fileName.isEmpty())
    {
        return;
    }

    if(openProject(fileName)==Jimmy::ec_ok)
    {
        setProjectName(fileName);
        setProjectStatus(ProjectStatus::editing);
    }
}

Jimmy::ErrorCode MainWindow::openProject(const QString& fileFullPath)
{
    QFile file(fileFullPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,QStringLiteral("打开项目"),QStringLiteral("打开项目文件失败"));
        return ErrorCode::ec_error;
    }

    QJsonParseError error;
    QJsonDocument jd = QJsonDocument::fromJson(file.readAll(),&error);

    if(error.error!=QJsonParseError::NoError)
    {
        QMessageBox::critical(this,QStringLiteral("打开项目"),QStringLiteral("打开项目文件失败,项目文件不是合法的Json"));
        return ErrorCode::ec_error;
    }

    QJsonObject docObj = jd.object();
    auto memItor = docObj.find("project");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        QMessageBox::critical(this,QStringLiteral("打开项目"),QStringLiteral("打开项目文件失败,project 键不存在或无效"));
        return ErrorCode::ec_error;
    }

    json_general_ = memItor->toObject();

    memItor = docObj.find("categories");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        QMessageBox::critical(this,QStringLiteral("打开项目"),QStringLiteral("打开项目文件失败,categories 键不存在或无效"));
        return ErrorCode::ec_error;
    }
    QJsonObject jsonCategory = memItor->toObject();

    memItor = docObj.find("components");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        QMessageBox::critical(this,QStringLiteral("打开项目"),QStringLiteral("打开项目文件失败,components 键不存在或无效"));
        return ErrorCode::ec_error;
    }
    QJsonObject jsonComponents = memItor->toObject();

    memItor = docObj.find("relation");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        QMessageBox::critical(this,QStringLiteral("打开项目"),QStringLiteral("打开项目文件失败,relation 键不存在或无效"));
        return ErrorCode::ec_error;
    }
    json_relation_ = memItor->toObject();

    categoryManager_->setCategories(jsonCategory);
    componentManager_->setComponents(jsonComponents);

    auto val = json_relation_.value("_lists");
    if(val.isArray())
    {
        relationManager_->setRelation(val.toArray());
    }


    return ErrorCode::ec_ok;
}

void MainWindow::setProjectName(const QString& fileFullPath)
{
    projectName_ = fileFullPath;
    if(projectName_.isEmpty())
    {
        this->setWindowTitle(QStringLiteral("%1").arg(QApplication::applicationName()));
    }
    else
    {
        this->setWindowTitle(QStringLiteral("%1    -%2").arg(QApplication::applicationName(),projectName_));
    }
}

void MainWindow::onActionSaveProject()
{
    QFileInfo fileInfo(projectName_);
    QDir projectPath(fileInfo.dir());

    QJsonObject jo;
    jo.insert("project", json_general_);
    jo.insert("components", componentManager_->getComponents());

    QJsonObject categories = categoryManager_->getCategories();
    categories.insert("_root",fileInfo.baseName());
    jo.insert("categories", categories);

    jo.insert("relation", relationManager_->getRelation());

    time_t now_time = time(nullptr);
    struct tm ctm;
    localtime_s(&ctm, &now_time);

    QString bakfile = QStringLiteral("%1%2%3%4%5%6%7%8")
        .arg(fileInfo.baseName())
        .arg(ctm.tm_year + 1900)
        .arg(ctm.tm_mon + 1,2,10,QLatin1Char('0'))
        .arg(ctm.tm_mday,2,10,QLatin1Char('0'))
        .arg(ctm.tm_hour,2,10,QLatin1Char('0'))
        .arg(ctm.tm_min,2,10,QLatin1Char('0'))
        .arg(ctm.tm_sec, 2, 10, QLatin1Char('0'))
        .arg(CommonConst::ProjectSuffix);

    projectPath.rename(fileInfo.fileName(), bakfile);


    QFile file(projectName_);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this,QStringLiteral("保存项目"),QStringLiteral("保存项目失败,无法创建文件"));
        return;
    }

    file.write(QJsonDocument(jo).toJson());
    file.close();

    setProjectStatus(ProjectStatus::editing);
}

void MainWindow::selectedRelationChanged(RelationItem* relationItem)
{
    if(relationItem == nullptr)
    {
        return;
    }

    ads::CDockWidget* widget = nullptr;
    foreach(auto item,dockWidgets_)
    {
        if(static_cast<RelationViewer*>(item->widget())->getRID() == relationItem->RID)
        {
            widget = item;
            break;
        }
    }

    if(relationItem->checkStatus == Qt::Checked)
    {
        if(widget == nullptr)
        {
            QJsonObject jo;
            QJsonValue jv = json_relation_.value(relationItem->name);
            if(jv.isObject())
            {
                jo = jv.toObject();
            }
            auto relationDockWidget = createRelationViewerDockWidget(relationItem,jo);
            dockWidgets_.push_back(relationDockWidget);
            DockManager_->addDockWidgetFloating(relationDockWidget);
        }
        else
        {
            if(widget->windowTitle().compare(relationItem->name) != 0)
            {
                widget->setWindowTitle(relationItem->name);
            }
        }
    }
    else
    {
        if(widget)
        {
            widget->closeDockWidget();
            dockWidgets_.removeOne(widget);
        }
    }
}

void MainWindow::onRelationViewerCloseRequested()
{
    auto DockWidget = qobject_cast<ads::CDockWidget*>(sender());
    if(DockWidget)
    {
        dockWidgets_.removeOne(DockWidget);
        relationManager_->uncheckRelation(static_cast<RelationViewer*>(DockWidget->widget())->getRID());
    }
}

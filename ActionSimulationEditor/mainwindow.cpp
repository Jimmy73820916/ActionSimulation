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

const QVector<Jimmy::DesignComponent*>& MainWindow::getAllComponents() const
{
    if (componentManager_)
    {
        return componentManager_->getAllComponents();
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

    categoryManager_->setCategories(jsonCategory);
    componentManager_->setComponents(jsonComponents);

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

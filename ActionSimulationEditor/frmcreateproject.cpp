#include "frmcreateproject.h"
#include "qjsonobject.h"
#include "ui_frmcreateproject.h"
#include "commonconst.h"
#include "commonfunction.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

using namespace std;
using namespace Jimmy;

frmCreateProject::frmCreateProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmCreateProject)
{
    ui->setupUi(this);
    setFixedSize(370,230);

    ui->cbType->addItem(QStringLiteral("单用户模式"));
    ui->cbType->addItem(QStringLiteral("多用户模式"));
    ui->cbType->setCurrentIndex(0);

    ui->cbType->setItemData(0,QStringLiteral("所有客户端都属于同一个用户"),Qt::ToolTipRole);
    ui->cbType->setItemData(1,QStringLiteral("客户端分别属于多个不同的用户"),Qt::ToolTipRole);

    ui->sbMinInterval->setSuffix(QStringLiteral("(毫秒)"));
    ui->sbDefaultInterval->setSuffix(QStringLiteral("(毫秒)"));

    connect(ui->tbSelect,SIGNAL(clicked()),this,SLOT(on_selectDir()));
    connect(ui->pbOK,SIGNAL(clicked()),this,SLOT(accept()));
    connect(ui->pbCancel,SIGNAL(clicked()),this,SLOT(cancel()));
}

frmCreateProject::~frmCreateProject()
{
    delete ui;
}

void frmCreateProject::accept()
{
    if(ui->leName->text().isEmpty() || !CommonFunction::validEngName(ui->leName->text()))
    {
        QMessageBox::critical(this,QStringLiteral("创建项目"),QStringLiteral("项目名称无效"));
        return;
    }

    QDir dir(QStringLiteral("%1\\%2").arg(ui->leName->toolTip(),ui->leName->text()));
    if(!dir.exists())
    {
        if(!dir.mkpath(dir.absolutePath()))
        {
            QMessageBox::critical(this,QStringLiteral("创建项目"),QStringLiteral("创建项目路径失败"));
            return;
        }
    }

    fi_.setFile(QStringLiteral("%1\\%2.json").arg(dir.absolutePath(),ui->leName->text()));
    if(createEasyVspProject(fi_.absoluteFilePath()) != ErrorCode::ec_ok)
    {
        QMessageBox::critical(this,QStringLiteral("创建项目"),QStringLiteral("创建项目文件失败"));
        return;
    }

    close();
}

void frmCreateProject::cancel()
{
    projectName_.clear();
    close();
}

void frmCreateProject::on_selectDir()
{
    QString file = QFileDialog::getSaveFileName(this,QStringLiteral("创建项目"),QDir::currentPath() + "\\Projects",QStringLiteral("无后缀(*)"));
    if(file.isEmpty())
    {
        return;
    }

    QFileInfo fi(file);
    ui->leName->setText(fi.baseName());
    ui->leName->setToolTip(fi.absolutePath());
}

QString frmCreateProject::projectTemplate(const QString& projectName)
{
    QJsonObject jo;

    QJsonObject project;
    project.insert("min_timer_interval",ui->sbMinInterval->value());
    project.insert("default_timer_interval",ui->sbDefaultInterval->value());
    project.insert("project_type",(ui->cbType->currentIndex()==0)?CommonConst::single_user:CommonConst::multi_users);

    jo.insert("project",project);
    jo.insert("components",QJsonObject());

    QJsonObject category;
    category.insert("_root",projectName);
    category.insert(projectName,QJsonArray());

    jo.insert("categories",category);

    QJsonObject relation;
    relation.insert("_lists",QJsonArray());
    jo.insert("relation",relation);

    return QJsonDocument(jo).toJson();
}

Jimmy::ErrorCode frmCreateProject::createEasyVspProject(const QString& projectFullName)
{
    QFile file(projectFullName);
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this,QStringLiteral("创建项目"),QStringLiteral("创建项目文件失败"));
        return Jimmy::ErrorCode::ec_error;
    }

    QFileInfo fi(projectFullName);
    file.write(projectTemplate(fi.baseName()).toLocal8Bit());
    file.close();

    return Jimmy::ErrorCode::ec_ok;
}

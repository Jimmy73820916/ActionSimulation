#pragma once
#include "errorcode.h"
#include <QDialog>
#include <QFileInfo>

namespace Ui {
class frmCreateProject;
}

class frmCreateProject : public QDialog
{
    Q_OBJECT

public:
    explicit frmCreateProject(QWidget *parent = nullptr);
    ~frmCreateProject();

    QFileInfo getProjectInfo() { return fi_;}
signals:
    void errorMessage(QString);
public slots:
    void accept();
    void cancel();
    void on_selectDir();
private:
    Jimmy::ErrorCode createEasyVspProject(const QString& projectFullName);
    QString projectTemplate(const QString& projectName);
private:
    Ui::frmCreateProject *ui;

    QString projectPath_;
    QString projectName_;

    QFileInfo fi_;

};


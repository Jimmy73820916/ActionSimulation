#pragma once

#include <QDialog>
#include "designcomponent.h"
#include "qautocompleteplaintextedit.h"

namespace Ui {
class ComponentDialog;
}

class ComponentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ComponentDialog(QWidget *parent = nullptr);
    ~ComponentDialog();

    void setModifiedComponent(const Jimmy::DesignComponent* component);
public slots:
    void cbTypeIndexChanged(int index);
    void cbBehaviorTextChanged(const QString& indexText);
    void saveAndAdd();
    void saveAndExit();
    void exit();
private:
    void resetCommonInputs();
    void behaviorInputDevice();
    void behaviorInput();
    void behaviorNormalScript();
    void behaviorTeamMaster();
    void behaviorTeamSlave();

    bool saveComponent();

    bool verifyComponent();
    Jimmy::DesignComponent collectComponentInfo();
private:
    Ui::ComponentDialog *ui;
    QString modifiedCID_;

    QStringList categories_;
    QStringList CIDS_;
    QStringList teams_;
    QStringList roles_;
};


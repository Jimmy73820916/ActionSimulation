#include "componentdialog.h"
#include "ui_componentdialog.h"
#include "qtsmalltools.h"
#include "commonconst.h"
#include "commonfunction.h"
#include "mainwindow.h"
#include <QJsonDocument>
#include <QMessageBox>

using namespace Jimmy;

ComponentDialog::ComponentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComponentDialog)
{
    ui->setupUi(this);
    setFixedSize(820,500);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->cbType->addItem(QTSmalltools::svgIcon(":/image/image/internaldevice.svg"),getTypeString(ComponentType::Internal));
    ui->cbType->addItem(QTSmalltools::svgIcon(":/image/image/inputdevice.svg"),getTypeString(ComponentType::Input));
    ui->cbType->addItem(QTSmalltools::svgIcon(":/image/image/outputdevice.svg"),getTypeString(ComponentType::Output));
    ui->cbType->addItem(QTSmalltools::svgIcon(":/image/image/teammaster.svg"),getTypeString(ComponentType::TeamMaster));
    ui->cbType->addItem(QTSmalltools::svgIcon(":/image/image/teamslave.svg"),getTypeString(ComponentType::TeamSlave));

    ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/script.svg"),getBehaviorString(BehaviorType::Script));
    ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/switch.svg"),getBehaviorString(BehaviorType::EqualInput));
    ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/keyboard.svg"),getBehaviorString(BehaviorType::EqualInputIgnoreReset));
    ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/btn.svg"),getBehaviorString(BehaviorType::ReversalIgnoreOther));

    ui->dsbActionKeep->setSuffix(QStringLiteral("秒"));

    ui->edtID->setMaxLength(CommonConst::MaxNameLength);
    ui->edtID->setToolTip(QStringLiteral("以英文字母开头,中间包括英文字符,数字,_。以英文字符或数字结束,最大长度64"));

    ui->edtName->setMaxLength(CommonConst::MaxNameLength);
    ui->edtName->setToolTip(QStringLiteral("中英文字符开头,加数字,_ 组成,最大长度64"));

    g_MainWindow_->getAllCategories(categories_);
    categories_.sort();
    foreach(auto item,categories_)
    {
        ui->cbcategory->addItem(item);
    }
    ui->cbcategory->setCurrentIndex(0);
    ui->cbcategory->setToolTip(QStringLiteral("中英文字符开头,加数字,_ 组成,最大长度64"));

    roles_ = g_MainWindow_->getAllRoles();
    roles_.sort();
    foreach(auto item,roles_)
    {
        ui->cbRole->addItem(item);
    }
    ui->cbRole->setCurrentIndex(0);
    ui->cbRole->setToolTip(QStringLiteral("以英文字母开头,中间包括英文字符,数字,_。以英文字符或数字结束,最大长度64"));

    teams_ = g_MainWindow_->getAllTeams();

    CIDS_ = g_MainWindow_->getAllCID();

    ui->txtSubscription->append(CIDS_);
    ui->txtReference->append(CIDS_);

    connect(ui->cbType,SIGNAL(currentIndexChanged(int)),this,SLOT(cbTypeIndexChanged(int)));
    connect(ui->cbBehavior,SIGNAL(currentTextChanged(QString)),this,SLOT(cbBehaviorTextChanged(QString)));

    connect(ui->pbSaveAdd,SIGNAL(clicked()),this,SLOT(saveAndAdd()));
    connect(ui->pbSaveExit,SIGNAL(clicked()),this,SLOT(saveAndExit()));
    connect(ui->pbExit,SIGNAL(clicked()),this,SLOT(exit()));

    ui->cbType->setCurrentIndex(0);
    ui->cbBehavior->setCurrentIndex(0);
    behaviorNormalScript();
}

ComponentDialog::~ComponentDialog()
{
    delete ui;
}

void ComponentDialog::setModifiedComponent(const Jimmy::DesignComponent* component)
{
    modifiedCID_ = component->getID();
    ui->edtID->setText(component->getID());
    ui->edtName->setText(component->getName());
    ui->cbcategory->setCurrentText(component->getCategory());
    ui->cbType->setCurrentIndex(component->getType());

    switch (component->getType())
    {
    case ComponentType::Input:
    {
        behaviorInputDevice();
        ui->cbBehavior->setCurrentIndex(component->getBehavior() - 1);
        break;
    }
    case ComponentType::TeamMaster:
    {
        behaviorTeamMaster();
        break;
    }
    case ComponentType::TeamSlave:
    {
        behaviorTeamSlave();
        break;
    }
    case ComponentType::Internal:
    case ComponentType::Output:
    {
        if(component->getBehavior() == BehaviorType::Script)
        {
            behaviorNormalScript();
        }
        else
        {
            behaviorInput();
        }

        ui->cbBehavior->setCurrentIndex(component->getBehavior());
    }
    default:break;
    }


    ui->cbTeam->setCurrentText(component->getTeam());
    ui->cbRole->setCurrentText(component->getRole());
    ui->dsbActionKeep->setValue(component->getActionKeep());

    QString val;
    if(component->getDefaultValue().isString())
    {
        val = QStringLiteral("\"%1\"").arg(component->getDefaultValueString());
    }
    else
    {
        val = QStringLiteral("%1").arg(component->getDefaultValueString());
    }
    ui->edtDefaultValue->setText(val);
    ui->txtSubscription->setPlainText(component->getSubscription().join(','));
    ui->txtReference->setPlainText(component->getReference().join(','));
    ui->edtRespondBoardcast->setText(component->getRespondBoardcast().join(','));
    ui->txtDescription->setPlainText(component->getDescription());
}

void ComponentDialog::resetCommonInputs()
{
    ui->edtID->setText("");
    ui->edtName->setText("");
    ui->txtDescription->setPlainText("");
}

void ComponentDialog::behaviorNormalScript()
{
    ui->labelSubcription->setText(QStringLiteral("订阅设备(多个设备用\",\"分割):"));

    ui->cbRole->setCurrentText("");
    ui->cbRole->setEnabled(true);

    ui->edtDefaultValue->setEnabled(true);

    ui->txtSubscription->setEnabled(true);
    ui->txtReference->setEnabled(true);
    ui->edtRespondBoardcast->setEnabled(true);

    ui->cbTeam->setCurrentIndex(0);
    ui->cbTeam->setEditable(false);
    ui->cbTeam->setEnabled(false);

    ui->dsbActionKeep->setValue(0.0);
    ui->dsbActionKeep->setEnabled(false);
}

void ComponentDialog::behaviorTeamMaster()
{
    ui->labelSubcription->setText(QStringLiteral("订阅设备(多个设备用\",\"分割):"));

    ui->cbTeam->clear();
    ui->cbTeam->setEditable(true);
    ui->cbTeam->setEnabled(true);

    ui->cbRole->setCurrentIndex(0);
    ui->cbRole->setEnabled(true);

    ui->edtDefaultValue->setText("0");
    ui->edtDefaultValue->setEnabled(false);

    ui->txtSubscription->setEnabled(true);
    ui->txtReference->setEnabled(true);
    ui->edtRespondBoardcast->setEnabled(true);

    ui->dsbActionKeep->setValue(0.0);
    ui->dsbActionKeep->setEnabled(false);
}

void ComponentDialog::behaviorTeamSlave()
{
    ui->labelSubcription->setText(QStringLiteral("订阅设备:"));

    ui->cbTeam->clear();
    foreach(auto item,teams_)
    {
        ui->cbTeam->addItem(item);
    }
    ui->cbTeam->setCurrentIndex(0);
    ui->cbTeam->setToolTip(QStringLiteral("中英文字符开头,加数字,_ 组成,最大长度64"));

    ui->cbTeam->setEditable(false);
    ui->cbTeam->setEnabled(true);

    ui->edtDefaultValue->setEnabled(true);

    ui->cbRole->setCurrentText("");
    ui->cbRole->setEnabled(false);

    ui->dsbActionKeep->setValue(0.0);
    ui->dsbActionKeep->setEnabled(false);

    ui->txtSubscription->setPlainText("");
    ui->txtSubscription->setEnabled(false);

    ui->txtReference->setPlainText("");
    ui->txtReference->setEnabled(false);

    ui->edtRespondBoardcast->setText("");
    ui->edtRespondBoardcast->setEnabled(false);
}

void ComponentDialog::behaviorInputDevice()
{
    ui->labelSubcription->setText(QStringLiteral("订阅设备(最多只能订阅一个设备):"));

    ui->dsbActionKeep->setEnabled(true);
    ui->edtDefaultValue->setEnabled(true);

    ui->txtSubscription->setEnabled(true);

    ui->cbTeam->clear();
    ui->cbTeam->setEditable(false);
    ui->cbTeam->setEnabled(false);

    ui->cbRole->setCurrentText("");
    ui->cbRole->setEnabled(false);

    ui->txtReference->setPlainText("");
    ui->txtReference->setEnabled(false);

    ui->edtRespondBoardcast->setText("");
    ui->edtRespondBoardcast->setEnabled(false);
}

void ComponentDialog::behaviorInput()
{
    ui->labelSubcription->setText(QStringLiteral("订阅设备(最多只能订阅一个设备):"));

    ui->dsbActionKeep->setValue(0);
    ui->dsbActionKeep->setEnabled(false);

    ui->edtDefaultValue->setEnabled(true);

    ui->txtSubscription->setEnabled(true);

    ui->cbTeam->clear();
    ui->cbTeam->setEditable(false);
    ui->cbTeam->setEnabled(false);

    ui->cbRole->setCurrentText("");
    ui->cbRole->setEnabled(false);

    ui->txtReference->setPlainText("");
    ui->txtReference->setEnabled(false);

    ui->edtRespondBoardcast->setText("");
    ui->edtRespondBoardcast->setEnabled(false);
}

void ComponentDialog::cbTypeIndexChanged(int index)
{
    ui->cbBehavior->clear();
    switch(index)
    {
    case ComponentType::Internal:
    case ComponentType::Output:
    {
        ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/script.svg"),getBehaviorString(BehaviorType::Script));
        ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/switch.svg"),getBehaviorString(BehaviorType::EqualInput));
        ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/keyboard.svg"),getBehaviorString(BehaviorType::EqualInputIgnoreReset));
        ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/btn.svg"),getBehaviorString(BehaviorType::ReversalIgnoreOther));
        behaviorNormalScript();
        break;
    }
    case ComponentType::Input:
    {
        ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/switch.svg"),getBehaviorString(BehaviorType::EqualInput));
        ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/keyboard.svg"),getBehaviorString(BehaviorType::EqualInputIgnoreReset));
        behaviorInputDevice();
        break;
    }
    case ComponentType::TeamMaster:
    {
        ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/script.svg"),getBehaviorString(BehaviorType::Script));
        ui->cbBehavior->setCurrentIndex(0);
        behaviorTeamMaster();
        break;
    }
    case ComponentType::TeamSlave:
    {
        ui->cbBehavior->addItem(QTSmalltools::svgIcon(":/image/image/masterset.svg"),getBehaviorString(BehaviorType::MasterSet));
        ui->cbBehavior->setCurrentIndex(0);
        behaviorTeamSlave();
        break;
    }
    default:break;
    }
}

void ComponentDialog::cbBehaviorTextChanged(const QString& indexText)
{
    if (indexText == getBehaviorString(BehaviorType::Script))
    {
        if (ui->cbType->currentIndex() != ComponentType::TeamMaster)
        {
            behaviorNormalScript();
        }
    }
    else
    {
        if (ui->cbType->currentIndex() == ComponentType::Input)
        {
            behaviorInputDevice();
        }
        else
        {
            behaviorInput();
        }
    }
}

bool ComponentDialog::verifyComponent()
{
    if (!CommonFunction::validEngName(ui->edtID->text()))
    {
        QMessageBox::critical(this, QStringLiteral("验证组件"), QStringLiteral("组件ID非法"));
        return false;
    }

    if (ui->edtName->text().trimmed().isEmpty())
    {
		QMessageBox::critical(this, QStringLiteral("验证组件"), QStringLiteral("组件Name非法"));
		return false;
    }

	if (ui->cbcategory->currentText().trimmed().isEmpty())
	{
		QMessageBox::critical(this, QStringLiteral("验证组件"), QStringLiteral("组件 Category 非法"));
		return false;
	}

    if(ui->cbBehavior->currentText() == getBehaviorString(BehaviorType::Script))
    {
        if(!CommonFunction::validEngName(ui->cbRole->currentText()))
        {
            QMessageBox::critical(this,QStringLiteral("验证组件"),QStringLiteral("角色名称非法"));
            return false;
        }
    }

    QJsonParseError error;
    QJsonDocument jd = QJsonDocument::fromJson(QStringLiteral("{\"value\":%1}").arg(ui->edtDefaultValue->text()).toLocal8Bit(),&error);

    if(error.error!=QJsonParseError::NoError)
    {
        QMessageBox::critical(this,QStringLiteral("验证组件"),QStringLiteral("默认值格式非法"));
        return false;
    }

    return true;
}

Jimmy::DesignComponent ComponentDialog::collectComponentInfo()
{
    DesignComponent designComponent;
    
    designComponent.setID(ui->edtID->text());
    designComponent.setName(ui->edtName->text());
    designComponent.setCategory(ui->cbcategory->currentText());
    designComponent.setType(static_cast<ComponentType>(ui->cbType->currentIndex()));

    switch(designComponent.getType())
    {
    case Jimmy::ComponentType::TeamSlave:
    {
        designComponent.setBehavior(BehaviorType::MasterSet);
        break;
    }
    case Jimmy::ComponentType::TeamMaster:
    {
        designComponent.setBehavior(BehaviorType::Script);
        break;
    }
    case Jimmy::ComponentType::Input:
    {
        designComponent.setBehavior(static_cast<BehaviorType>(ui->cbBehavior->currentIndex() + 1));
        break;
    }
    case Jimmy::ComponentType::Internal:
    case Jimmy::ComponentType::Output:
    {
        designComponent.setBehavior(static_cast<BehaviorType>(ui->cbBehavior->currentIndex()));
        break;
    }
    default:break;
    }


    designComponent.setTeam(ui->cbTeam->currentText());
    designComponent.setRole(ui->cbRole->currentText());
    designComponent.setActionKeep(ui->dsbActionKeep->value());

    QJsonDocument jd = QJsonDocument::fromJson(QStringLiteral("{\"value\":%1}").arg(ui->edtDefaultValue->text()).toLocal8Bit(),nullptr);

    designComponent.setDefaultValue(jd.object().value("value"));

    QString subscription = ui->txtSubscription->toPlainText();
    if(!subscription.isEmpty())
    {
        designComponent.setSubscription(subscription.split(',',Qt::SkipEmptyParts));
    }

    QString reference = ui->txtReference->toPlainText();
    if(!reference.isEmpty())
    {
        designComponent.setReference(reference.split(',',Qt::SkipEmptyParts));
    }

    QString boardcast = ui->edtRespondBoardcast->text();
    if(!boardcast.isEmpty())
    {
        designComponent.setRespondBoardcast(boardcast.split(',',Qt::SkipEmptyParts));
    }

    designComponent.setDescription(ui->txtDescription->toPlainText());

    return designComponent;
}

bool ComponentDialog::saveComponent()
{
    if(!verifyComponent())
    {
        return false;
    }

    if(!categories_.contains(ui->cbcategory->currentText()))
    {
        categories_.append(ui->cbcategory->currentText());
        ui->cbcategory->addItem(ui->cbcategory->currentText());
    }

    if(ui->cbType->currentIndex() == Jimmy::ComponentType::TeamMaster)
    {
        if(!teams_.contains(ui->cbTeam->currentText()))
        {
            teams_.append(ui->cbTeam->currentText());
        }
    }

    if(ui->cbBehavior->currentIndex() == Jimmy::BehaviorType::Script)
    {
        if(!roles_.contains(ui->cbRole->currentText()))
        {
            roles_.append(ui->cbRole->currentText());
            ui->cbRole->addItem(ui->cbRole->currentText());
        }
    }

    if(modifiedCID_.isEmpty())
    {
        ui->txtSubscription->append(ui->edtID->text());
        ui->txtReference->append(ui->edtID->text());

        return g_MainWindow_->appendComponent(collectComponentInfo());
    }
    else
    {
        return g_MainWindow_->modifyComponent(modifiedCID_,collectComponentInfo());
    }
}

void ComponentDialog::saveAndAdd()
{
    if(saveComponent())
    {
        if(!modifiedCID_.isEmpty())
        {
            modifiedCID_.clear();
        }
        resetCommonInputs();
    }
    else
    {
        QMessageBox::critical(this,QStringLiteral("保存组件"),QStringLiteral("保存组件失败"));
    }

}

void ComponentDialog::saveAndExit()
{
    if(saveComponent())
    {
        close();
    }
    else
    {
        QMessageBox::critical(this,QStringLiteral("保存组件"),QStringLiteral("保存组件失败"));
    }
}

void ComponentDialog::exit()
{
    close();
}

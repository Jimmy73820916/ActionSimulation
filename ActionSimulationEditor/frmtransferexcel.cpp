#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "frmtransferexcel.h"
#include "ui_frmtransferexcel.h"
#include "designcomponent.h"
#include "xlsxdocument.h"
#include "commonfunction.h"

QXLSX_USE_NAMESPACE

using namespace Jimmy;

FrmTransferExcel::FrmTransferExcel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrmTransferExcel)
{
    ui->setupUi(this);

    transferType_ = TransferType::Unknown;
    isTransfering_ = false;

    connect(ui->selectFile, SIGNAL(clicked()),this, SLOT(on_selectFile()));
    connect(ui->pbRun, SIGNAL(clicked()),this, SLOT(on_Transfer()));
    connect(ui->pbExit, SIGNAL(clicked()),this, SLOT(on_Exit()));

    ui->progressBar->hide();
}

FrmTransferExcel::~FrmTransferExcel()
{
    delete ui;
}

void FrmTransferExcel::setTransferType(TransferType tt)
{
    transferType_ = tt;
    if(transferType_ == TransferType::Export)
    {
        ui->label_2->hide();
        ui->cbType->hide();

        ui->pbRun->setText(QStringLiteral("导出"));
    }
    else if(transferType_ == TransferType::Import)
    {
        ui->cbType->addItem(QStringLiteral("覆盖原始数据"));
        ui->cbType->addItem(QStringLiteral("忽略导入数据"));

        ui->pbRun->setText(QStringLiteral("导入"));
    }
    else
    {
        ui->pbRun->hide();
    }
}

TransferType FrmTransferExcel::getTransferType()
{
    return transferType_;
}

void FrmTransferExcel::on_Exit()
{
    if(isTransfering_)
    {
        return;
    }

    close();
}

void FrmTransferExcel::on_Transfer()
{
    if(transferType_ == TransferType::Import)
    {
        if(ui->lineEdit->text().isEmpty())
        {
            QMessageBox::information(this,QStringLiteral("导入"),QStringLiteral("选择导入文件"));
            return;
        }

        importData();
    }
    else if(transferType_ == TransferType::Export)
    {
        if(ui->lineEdit->text().isEmpty())
        {
            QMessageBox::information(this,QStringLiteral("导出"),QStringLiteral("设置导出文件"));
            return;
        }

        exportData();
    }

    close();
}


void FrmTransferExcel::on_selectFile()
{
    if(transferType_ == TransferType::Export)
    {
        ui->lineEdit->setText(QFileDialog::getSaveFileName(this,QStringLiteral("导出到 Excel 文件"),"D:/",QStringLiteral("excel格式(*.xls *.xlsx)")));
    }
    else if(transferType_ == TransferType::Import)
    {
        ui->lineEdit->setText(QFileDialog::getOpenFileName(this,QStringLiteral("打开Excel文件"),"D:/",QStringLiteral("excel格式(*.xls *.xlsx)")));
    }
}

void FrmTransferExcel::importData()
{
    Document xlsx(ui->lineEdit->text());

    int pos(2);

    int total(0);
    while(true)
    {
        auto id = xlsx.read(QStringLiteral("A%1").arg(pos++));
        if(id.isNull())
        {
            break;
        }
        total++;
    }

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat(QStringLiteral("%1/%2").arg(0).arg(total));

    ui->progressBar->setVisible(true);

    pos =  2;
    auto index{0};
    while(true)
    {
        DesignComponent component;
        auto id = xlsx.read(QStringLiteral("A%1").arg(pos));
        if(id.isNull())
        {
            break;
        }
        component.setID(id.toString());

        auto name = xlsx.read(QStringLiteral("B%1").arg(pos));
        if(name.isNull())
        {
            emit errorMessage(QStringLiteral("component %1 item 名称 is invalid").arg(component.getID()));
            return;
        }
        component.setName(name.toString());

        auto category = xlsx.read(QStringLiteral("C%1").arg(pos));
        if(category.isNull())
        {
            emit errorMessage(QStringLiteral("component %1 item 类别 is invalid").arg(component.getID()));
            return;
        }
        component.setCategory(category.toString());


        auto componentType = xlsx.read(QStringLiteral("D%1").arg(pos));
        if(componentType.isNull())
        {
            emit errorMessage(QStringLiteral("component %1 item 设备类型 is invalid").arg(component.getID()));
            return;
        }

        auto bOk{false};
        auto componentVal = componentType.toInt(&bOk);
        if(!bOk || (componentVal < Jimmy::ComponentType::Internal) || (componentVal >= Jimmy::ComponentType::ComponentType_Uplimit))
        {
            emit errorMessage(QStringLiteral("component %1 item 设备类型 is invalid").arg(component.getID()));
            return;
        }
        component.setType(static_cast<Jimmy::ComponentType>(componentVal));

        auto team = xlsx.read(QStringLiteral("E%1").arg(pos));
        if(!team.isNull())
        {
            component.setTeam(team.toString());
        }

        auto role = xlsx.read(QStringLiteral("F%1").arg(pos));
        if(!role.isNull())
        {
            component.setRole(role.toString());
        }

        if(component.getType() == Jimmy::ComponentType::Input)
        {
            component.setBehavior(Jimmy::BehaviorType::EqualInput);
        }
        else if(component.getType() == Jimmy::ComponentType::TeamMaster)
        {
            component.setBehavior(Jimmy::BehaviorType::Script);
        }
        else
        {
            auto behaviorType = xlsx.read(QStringLiteral("G%1").arg(pos));

            auto behaviorVal = behaviorType.toInt(&bOk);
            if(!bOk || (behaviorVal < Jimmy::BehaviorType::Script) || (behaviorVal >= Jimmy::BehaviorType::BehaviorType_Uplimit))
            {
                emit errorMessage(QStringLiteral("component %1 item 行为 is invalid").arg(component.getID()));
                return;
            }
            component.setBehavior(static_cast<Jimmy::BehaviorType>(behaviorVal));
        }

        auto actionKeep = xlsx.read(QStringLiteral("H%1").arg(pos));
        component.setActionKeep(actionKeep.toInt());

        auto subscription = xlsx.read(QStringLiteral("I%1").arg(pos));
        component.setSubscription(subscription.toStringList());

        auto reference = xlsx.read(QStringLiteral("J%1").arg(pos));
        component.setReference(reference.toStringList());

        auto respondBoardcast = xlsx.read(QStringLiteral("K%1").arg(pos));
        component.setRespondBoardcast(respondBoardcast.toStringList());

        component.setDefaultValue(CommonFunction::transferType(xlsx.read(QStringLiteral("L%1").arg(pos))));

        auto description = xlsx.read(QStringLiteral("M%1").arg(pos));
        component.setDescription(description.toString());

        g_MainWindow_->appendCategory(component.getCategory());

        if(!g_MainWindow_->appendComponent(component))
        {
            if(ui->cbType->currentIndex() == 0)
            {
                g_MainWindow_->modifyComponent(component.getID(),component);
            }
        }

        pos++;
        index++;
        ui->progressBar->setValue(pos);
        ui->progressBar->setFormat(QStringLiteral("%1/%2").arg(index).arg(total));
    }

}

void FrmTransferExcel::exportData()
{
    Document xlsx;

    auto components = g_MainWindow_->getAllComponents(true);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(components.size());
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat(QStringLiteral("%1/%2").arg(0).arg(components.size()));

    ui->progressBar->setVisible(true);

    xlsx.write("A1", QStringLiteral("CID"));
    xlsx.write("B1", QStringLiteral("名称"));
    xlsx.write("C1", QStringLiteral("类别"));
    xlsx.write("D1", QStringLiteral("设备类型"));
    xlsx.write("E1", QStringLiteral("分组"));
    xlsx.write("F1", QStringLiteral("角色"));
    xlsx.write("G1", QStringLiteral("行为类型"));
    xlsx.write("H1", QStringLiteral("置位信号时间"));
    xlsx.write("I1", QStringLiteral("订阅组件"));
    xlsx.write("J1", QStringLiteral("引用组件"));
    xlsx.write("K1", QStringLiteral("响应故障码"));
    xlsx.write("L1", QStringLiteral("缺省值"));
    xlsx.write("M1", QStringLiteral("描述"));

    for(int index = 0;index < components.size();++index)
    {
        auto pos = index + 2;
        xlsx.write(QStringLiteral("A%1").arg(pos), components[index]->getID());
        xlsx.write(QStringLiteral("B%1").arg(pos), components[index]->getName());
        xlsx.write(QStringLiteral("C%1").arg(pos), components[index]->getCategory());
        xlsx.write(QStringLiteral("D%1").arg(pos), components[index]->getType());
        xlsx.write(QStringLiteral("E%1").arg(pos), components[index]->getTeam());
        xlsx.write(QStringLiteral("F%1").arg(pos), components[index]->getRole());
        xlsx.write(QStringLiteral("G%1").arg(pos), components[index]->getBehavior());
        xlsx.write(QStringLiteral("H%1").arg(pos), components[index]->getActionKeep());
        xlsx.write(QStringLiteral("I%1").arg(pos), components[index]->getSubscription().join(','));
        xlsx.write(QStringLiteral("J%1").arg(pos), components[index]->getReference().join(','));
        xlsx.write(QStringLiteral("K%1").arg(pos), components[index]->getRespondBoardcast().join(','));
        xlsx.write(QStringLiteral("L%1").arg(pos), components[index]->getDefaultValueString());
        xlsx.write(QStringLiteral("M%1").arg(pos), components[index]->getDescription());
        ui->progressBar->setValue(index);
        ui->progressBar->setFormat(QStringLiteral("%1/%2").arg(index + 1).arg(components.size()));
    }

   if (!xlsx.saveAs(ui->lineEdit->text()))
   {
       QMessageBox::critical(this,QStringLiteral("导出"),QStringLiteral("保存导出文件失败"));
       return;
   }
}


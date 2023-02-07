#pragma once

#include <QDialog>

namespace Ui {
class FrmTransferExcel;
}

namespace QXlsx {
class Document;
}

class CategoryItem;

enum class TransferType
{
    Unknown,
    Import,
    Export,
};

class FrmTransferExcel : public QDialog
{
    Q_OBJECT
public:
    explicit FrmTransferExcel(QWidget *parent = nullptr);
    ~FrmTransferExcel();

    void setTransferType(TransferType tt);
    TransferType getTransferType();

signals:
    void errorMessage(QString);
public slots:
    void on_Exit();
    void on_Transfer();
    void on_selectFile();
private:
    void importData();
    void exportData();
private:
    Ui::FrmTransferExcel *ui;

    TransferType transferType_;
    bool isTransfering_;
};


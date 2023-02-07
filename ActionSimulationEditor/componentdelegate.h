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

#include <QStyledItemDelegate>

class ReadOnlyDelegate:public QStyledItemDelegate
{
    Q_OBJECT
public:
    ReadOnlyDelegate(QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

class LineTextDelegate :public QStyledItemDelegate
{
    Q_OBJECT
public:
    LineTextDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* widget, const QModelIndex& index) const override;
    void setModelData(QWidget* widget,QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool eventFilter(QObject *object, QEvent *event) override;
};


class ComboboxDelegate :public QStyledItemDelegate
{
    Q_OBJECT
public:
    ComboboxDelegate(QObject* parent = nullptr);

    void setValidatorSet(const QStringList& list);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* widget, const QModelIndex& index) const override;
    void setModelData(QWidget* widget,QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
private:
    QStringList list_;
};

class DoubleSpinboxDelegate :public QStyledItemDelegate
{
    Q_OBJECT
public:
    DoubleSpinboxDelegate(QObject* parent = nullptr,double minval = 0,double maxVal = 10,double step = 1);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* widget, const QModelIndex& index) const override;
    void setModelData(QWidget* widget,QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
private:
    double minval_;
    double maxVal_;
    double step_;
};

class PlainTextDelegate :public QStyledItemDelegate
{
    Q_OBJECT
public:
    PlainTextDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* widget, const QModelIndex& index) const override;
    void setModelData(QWidget* widget,QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool eventFilter(QObject *object, QEvent *event) override;
};

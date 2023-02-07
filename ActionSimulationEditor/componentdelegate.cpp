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
#include <QSpinBox>
#include <QComboBox>
#include <QPlainTextEdit>
#include "QLineEdit"
#include "QEvent"
#include "QKeyEvent"
#include "QDebug"
#include "componentdelegate.h"

ReadOnlyDelegate::ReadOnlyDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{

}

QWidget* ReadOnlyDelegate::createEditor(QWidget* /*parent*/, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    return nullptr;
}

LineTextDelegate::LineTextDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{

}

QWidget* LineTextDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QWidget* editor = QStyledItemDelegate::createEditor(parent,option,index);
    editor->installEventFilter(const_cast<LineTextDelegate*>(this));
    return editor;
}

void LineTextDelegate::setEditorData(QWidget* widget, const QModelIndex& index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit* editor = static_cast<QLineEdit*>(widget);
    if (editor)
    {
        editor->setText(text);
    }
    else
    {
        QStyledItemDelegate::setEditorData(widget,index);
    }
}

void LineTextDelegate::setModelData(QWidget* widget, QAbstractItemModel* model, const QModelIndex& index) const
{
    QLineEdit* editor = static_cast<QLineEdit*>(widget);
    if (editor)
    {
        QString text = editor->text().trimmed();
        if(!text.isEmpty())
        {
            model->setData(index, text, Qt::EditRole);
        }
    }
    else
    {
        QStyledItemDelegate::setModelData(widget,model,index);
    }
}

void LineTextDelegate::updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)
    widget->setGeometry(option.rect);
}

bool LineTextDelegate::eventFilter(QObject *object, QEvent *event)
{
    QWidget* editor = qobject_cast<QWidget*>(object);
    if(editor && (event->type() == QEvent::KeyPress))
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if((key_event->key() == Qt::Key_Enter)||(key_event->key() == Qt::Key_Return))
        {
            QString text = static_cast<QLineEdit*>(editor)->text().trimmed();
            if(!text.isEmpty())
            {
                emit commitData(editor);
            }

            emit closeEditor(editor);
            return true;
        }
        else if (key_event->key() == Qt::Key_Escape)
        {
            emit closeEditor(editor);
            return true;
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}

ComboboxDelegate::ComboboxDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{

}

void ComboboxDelegate::setValidatorSet(const QStringList& list)
{
    list_ = list;
}

QWidget* ComboboxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItems(list_);
    editor->installEventFilter(const_cast<ComboboxDelegate*>(this));
    return editor;
}

void ComboboxDelegate::setEditorData(QWidget* widget, const QModelIndex& index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QComboBox* editor = static_cast<QComboBox*>(widget);
    auto tindex = editor->findText(text);
    if(tindex != -1)
    {
        editor->setCurrentIndex(tindex);
    }
}

void ComboboxDelegate::setModelData(QWidget* widget, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* editor = static_cast<QComboBox*>(widget);
    model->setData(index, editor->currentText(), Qt::EditRole);
}

void ComboboxDelegate::updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    widget->setGeometry(option.rect);
}

DoubleSpinboxDelegate::DoubleSpinboxDelegate(QObject* parent,double minval,double maxVal,double step)
    :QStyledItemDelegate(parent)
    ,minval_(minval)
    ,maxVal_(maxVal)
    ,step_(step)
{

}

QWidget* DoubleSpinboxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setMinimum(minval_);
    editor->setMaximum(maxVal_);
    editor->setSingleStep(step_);
    editor->setSuffix(QStringLiteral("秒"));
    return editor;
}

void DoubleSpinboxDelegate::setEditorData(QWidget* widget, const QModelIndex& index) const
{
    QDoubleSpinBox *doublespinBox = static_cast<QDoubleSpinBox*>(widget);
    doublespinBox->valueFromText(index.model()->data(index, Qt::EditRole).toString());
}

void DoubleSpinboxDelegate::setModelData(QWidget* widget, QAbstractItemModel* model, const QModelIndex& index) const
{
    QDoubleSpinBox *editor = static_cast<QDoubleSpinBox*>(widget);
    double value = editor->value();
    model->setData(index, value, Qt::EditRole);
}

void DoubleSpinboxDelegate::updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    widget->setGeometry(option.rect);
}

PlainTextDelegate::PlainTextDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{

}

QWidget* PlainTextDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    QPlainTextEdit *editor = new QPlainTextEdit(parent);
    editor->installEventFilter(const_cast<PlainTextDelegate*>(this));
    return editor;
}

void PlainTextDelegate::setEditorData(QWidget* widget, const QModelIndex& index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QPlainTextEdit* editor = static_cast<QPlainTextEdit*>(widget);
    if (editor)
    {
        editor->setPlainText(text);
    }
    else
    {
        QStyledItemDelegate::setEditorData(widget,index);
    }

}

void PlainTextDelegate::setModelData(QWidget* widget, QAbstractItemModel* model, const QModelIndex& index) const
{
    QPlainTextEdit* editor = static_cast<QPlainTextEdit*>(widget);
    if (editor)
    {
        QString text = editor->toPlainText().trimmed();
        if(!text.isEmpty())
        {
            model->setData(index, text, Qt::EditRole);
        }
    }
    else
    {
        QStyledItemDelegate::setModelData(widget,model,index);
    }
}

void PlainTextDelegate::updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    widget->setGeometry(option.rect.x(),option.rect.y(),option.rect.width() * 2,option.rect.height() * 5);
}

bool PlainTextDelegate::eventFilter(QObject *object, QEvent *event)
{
    QWidget* editor = qobject_cast<QWidget*>(object);
    if(editor && (event->type() == QEvent::KeyPress))
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if((key_event->key() == Qt::Key_Enter)||(key_event->key() == Qt::Key_Return)||(key_event->key() == Qt::Key_Tab))
        {
            QString text = static_cast<QPlainTextEdit*>(editor)->toPlainText().trimmed();
            if(!text.isEmpty())
            {
                emit commitData(editor);
            }

            emit closeEditor(editor);
            return true;
        }
        else if (key_event->key() == Qt::Key_Escape)
        {
            emit closeEditor(editor);
            return true;
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}

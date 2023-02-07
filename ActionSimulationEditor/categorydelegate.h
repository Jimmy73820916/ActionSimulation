#pragma once

#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QValidator>
#include <QStyledItemDelegate>
#include <commonfunction.h>

class CategoryValidator:public QValidator
{
public:
    CategoryValidator(QObject* parent,const QStringList& existedValues)
        :QValidator(parent)
        ,existedValues_(existedValues)
    {

    }

    virtual State validate(QString& input, int& pos) const
    {
        Q_UNUSED(pos)
        if (input.isEmpty())
        {
            return QValidator::Intermediate;
        }

        if(existedValues_.contains(input))
        {
            return QValidator::Intermediate;
        }

        return QValidator::Acceptable;
    }
private:
    QStringList existedValues_;
};


class CategoryDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    CategoryDelegate(QObject* parent = nullptr)
        :QStyledItemDelegate(parent)
    {

    }

    void setMemberSet(QStringList&& memberSet)
    {
        memberSet_ = std::move(memberSet);
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QWidget* editor = QStyledItemDelegate::createEditor(parent,option,index);
        editor->installEventFilter(const_cast<CategoryDelegate*>(this));

        if(!memberSet_.empty())
        {
            auto p = static_cast<QLineEdit*>(editor);
            p->setValidator(new CategoryValidator(p,std::move(memberSet_)));
        }

        return editor;
    }


    void setEditorData(QWidget* widget, const QModelIndex& index) const override
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


    void setModelData(QWidget* widget,QAbstractItemModel* model, const QModelIndex& index) const override
    {
        if (auto editor = static_cast<QLineEdit*>(widget))
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

    void updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        Q_UNUSED(index)
        widget->setGeometry(option.rect);
    }

    bool eventFilter(QObject *object, QEvent *event) override
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
                static_cast<QLineEdit*>(editor)->setValidator(nullptr);
                emit closeEditor(editor);
                return true;
            }
        }
        return QStyledItemDelegate::eventFilter(object, event);
    }
private:
    QStringList memberSet_;
};

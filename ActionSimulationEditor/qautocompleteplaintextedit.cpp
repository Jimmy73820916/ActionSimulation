#include "qautocompleteplaintextedit.h"

QAutoCompletePlainTextEdit::QAutoCompletePlainTextEdit(QWidget* parent)
    :QPlainTextEdit(parent)
{
    findTable_ = new Jimmy::MaxMatchTree::NodalPoint(NULL);
}

QAutoCompletePlainTextEdit::~QAutoCompletePlainTextEdit()
{
    delete findTable_;
    findTable_ = nullptr;
}
void QAutoCompletePlainTextEdit::append(const QString& word)
{
    findTable_->append(word);
}

void QAutoCompletePlainTextEdit::append(const QStringList& tbl)
{
    foreach(auto item,tbl)
    {
        findTable_->append(item);
    }
}

void QAutoCompletePlainTextEdit::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Tab)
    {
        QString text = toPlainText().trimmed();
        auto index = text.lastIndexOf(",");
        if(index == -1)
        {
            if(text.isEmpty())
            {
                return;
            }

            auto matchText = findTable_->findMatch(text);
            if(!matchText.isEmpty())
            {
                setPlainText(matchText);
            }
        }
        else
        {
            auto findText = text.right(text.length() - text.lastIndexOf(",") - 1);
            auto matchText = findTable_->findMatch(findText);
            if(!matchText.isEmpty())
            {
                setPlainText(text.replace(text.lastIndexOf(",") + 1,text.length() - text.lastIndexOf(","),matchText));
            }
        }

        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
        setTextCursor(cursor);
    }
    else
    {
        QPlainTextEdit::keyPressEvent(e);
    }
}

#include "nodalpoint.h"

namespace Jimmy
{
namespace MaxMatchTree
{

NodalPoint::NodalPoint(QChar id)
    :id_(id)
{

}

NodalPoint::~NodalPoint()
{
    foreach(auto item,children_)
    {
        delete item;
        item = nullptr;
    }

    children_.clear();
}

void NodalPoint::append(const QString& leaf)
{
    if(leaf.isEmpty())
    {
        return;
    }

    append(0,leaf);
}

void NodalPoint::appendorset(int level,const QString& leaf)
{
    if(leaf.length() == (level + 1))
    {
        setData(leaf);
    }
    else
    {
        append(level+1,leaf);
    }
}

void NodalPoint::append(int level,const QString& leaf)
{
    foreach(auto item,children_)
    {
        if(item->getID() == leaf[level])
        {
            item->appendorset(level,leaf);
            return;
        }
    }


    NodalPoint* node = new NodalPoint(leaf[level]);
    children_.append(node);
    node->appendorset(level,leaf);
}

QString NodalPoint::findMaxMatch(bool ishead,QString& match)
{
    if(!Data_.isEmpty())
    {
        return Data_;
    }

    if(!ishead)
    {
        match.append(getID());
    }

    if(children_.size() == 1)
    {
        return children_[0]->findMaxMatch(false,match);
    }
    else
    {
        return match;
    }
}

QString NodalPoint::findMatch(int level,const QString& heads)
{
    foreach(auto item,children_)
    {
        if(item->getID() == heads[level])
        {
            if(heads.length() <= (level + 1))
            {
                QString match(heads);
                return item->findMaxMatch(true,match);
            }
            else
            {
                return item->findMatch(level+1,heads);
            }
        }
    }

    return "";
}

QString NodalPoint::findMatch(const QString& heads)
{
    if(heads.isEmpty() || children_.empty())
    {
        return "";
    }

    return findMatch(0,heads);
}

}
}

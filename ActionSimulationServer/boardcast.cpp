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

#include "boardcast.h"

using namespace std;
using namespace Jimmy;

void Boardcast::appendBoardcast(User userid,const QString& boardcast)
{
    lock_guard<shared_mutex> lg(lockBoardcastCodes_);
    if(!boardcastCodes_[userid].contains(boardcast))
    {
        boardcastCodes_[userid].append(boardcast);
    }
}

void Boardcast::removeBoardcast(User userid,const QString& boardcast)
{
    lock_guard<shared_mutex> lg(lockBoardcastCodes_);
    boardcastCodes_[userid].removeOne(boardcast);

    if(boardcastCodes_[userid].empty())
    {
        boardcastCodes_.remove(userid);
    }
}

QStringList Boardcast::getBoardcast(User userid)
{
    shared_lock<shared_mutex> lock(lockBoardcastCodes_);
    if(boardcastCodes_.contains(userid))
    {
        return boardcastCodes_[userid];
    }

    return QStringList();
}

QStringList Boardcast::getIntersect(User userid,const QStringList& boardcast)
{
    QStringList t = getBoardcast(userid);

    QStringList ret;
    foreach(auto elem, boardcast)
    {
        if(t.contains(elem))
        {
            ret.append(elem);
        }
    }

    return ret;
}

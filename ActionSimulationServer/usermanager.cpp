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

#include "usermanager.h"
#include "actionsimulationserver.h"
#include "projectmanager.h"
#include <boost/bimap/support/lambda.hpp>
#include <functional>

using namespace std;
using namespace Jimmy;

User getVisitorID()
{
    if (gActionSimulationServer.getProjectManager()->getProjectType() == ProjectType::SingleUser)
    {
        return User{0};
    }

    static std::atomic<size_t> visitorID_ = static_cast<size_t>(INT_MAX) + 1 ;

    size_t ret = visitorID_++;

    if(visitorID_.load() < INT_MAX)
    {
        visitorID_.store(static_cast<size_t>(INT_MAX)+1);
    }
    return User{ret};
}

UserManager::UserManager()
{
    gActionSimulationServer.registerAppendConnnection(std::bind(&UserManager::registerConnection,this,placeholders::_1));
    gActionSimulationServer.registerRemoveConnnection(std::bind(&UserManager::disconnect,this,placeholders::_1));
}

User UserManager::registerConnection(size_t connection)
{
    lock_guard<shared_mutex> lg(lockUser_);
    auto uid = getVisitorID();

    UserInfo user(Connection{connection}, uid,static_cast<size_t>(UserRole::Normal));

    auto iter = userInfo_.find(user);
    if (iter != userInfo_.end())
    {
        return iter->userId;
    }

    userInfo_.insert(user);
    return uid;
}

void UserManager::login(Connection connection,User userID,size_t role)
{
    User oldID{0};
    {
        lock_guard<shared_mutex> lg(lockUser_);
        UserInfo user(connection,userID, role);

        auto& connIDView = userInfo_.get<ConnId>();
        auto iter = connIDView.find(user);
        if (iter != connIDView.end())
        {
            oldID = iter->userId;
            connIDView.modify(iter,UpdateUserInfo(userID, role));

            if(gActionSimulationServer.getProjectManager()->getProjectType() != ProjectType::SingleUser)
            {
                auto& userView = userInfo_.get<UserId>();
                auto iter = userView.find(oldID);
                if(iter == userView.end())
                {
                    gActionSimulationServer.getProjectManager()->removeUser(oldID);
                }
            }
        }
        else
        {
            userInfo_.insert(user);
        }
    }
}

void UserManager::disconnect(size_t connection)
{
    User userID(0);
    {
        lock_guard<shared_mutex> lg(lockUser_);
        {
            UserInfo user(Connection{connection});

            auto iter = userInfo_.find(user);
            if (iter != userInfo_.end())
            {
                userID = iter->userId;
                userInfo_.erase(iter);
            }
        }

        if(userID.userID > 0)
        {
            auto& userView = userInfo_.get<UserId>();
            auto iter = userView.find(userID);
            if(iter != userView.end())
            {
                return;
            }
        }
    }

    if(gActionSimulationServer.getProjectManager()->getProjectType() != ProjectType::SingleUser)
    {
        gActionSimulationServer.getProjectManager()->removeUser(userID);
    }
}

std::optional<UserInfo> UserManager::getUserInfo(Connection connection)
{
    UserInfo user(connection);
    shared_lock<shared_mutex> lg(lockUser_);
    {

        auto iter = userInfo_.find(user);
        if (iter != userInfo_.end())
        {
            return *iter;
        }
    }

    return std::nullopt;
}

QVector<UserInfo> UserManager::getConnectIdbyUser(User userID)
{
    QVector<UserInfo> vRet;
    shared_lock<shared_mutex> lg(lockUser_);
    {
        auto& userView = userInfo_.get<UserId>();
        auto p = userView.equal_range(userID);
        for (auto it = p.first; it != p.second; ++it)
        {
            vRet.push_back(*it);
        }
    }

    return vRet;
}

QVector<UserInfo> UserManager::getConnectIdbyUsers_(const QVector<User>& vUserID)
{
    QVector<UserInfo> vRet;
    shared_lock<shared_mutex> lg(lockUser_);
    {
        auto& userView = userInfo_.get<UserId>();
        Q_FOREACH(auto userID,vUserID)
        {
            auto p = userView.equal_range(userID);
            for (auto it = p.first; it != p.second; ++it)
            {
                vRet.push_back(*it);
            }
        }
    }

    return vRet;
}

void UserManager::answerMessage(Connection connection, const QString& message)
{
    gActionSimulationServer.sendNetMessage(connection.ConnectionID,message);
}

void UserManager::sendUserMessage(Jimmy::User userid, bool admin_Only,const QString& message)
{
    switch (gActionSimulationServer.getProjectManager()->getProjectType())
    {
    case ProjectType::SingleUser:
    {
        return sendMessage(admin_Only,message);
    }
    case ProjectType::MultiUser:
    {
        return sendUserMessage_(userid,admin_Only,message);
    }
    }
}

void UserManager::sendUserMessage(Jimmy::User userid,bool admin_Only,Jimmy::Connection excludeConnection,const QString& message)
{
    switch (gActionSimulationServer.getProjectManager()->getProjectType())
    {
    case ProjectType::SingleUser:
    {
        return sendMessage(admin_Only,excludeConnection,message);
    }
    case ProjectType::MultiUser:
    {
        return sendUserMessage_(userid,admin_Only,excludeConnection,message);
    }
    }
}

void UserManager::sendRoleMessage(size_t role, const QString& message)
{
    shared_lock<shared_mutex> lg(lockUser_);
    {
        auto& roleView = userInfo_.get<RoleInfo>();
        auto p = roleView.equal_range(role);
        for (auto it = p.first; it != p.second; ++it)
        {
            gActionSimulationServer.sendNetMessage(it->connectId.ConnectionID,message);
        }
    }
}

void UserManager::sendRoleMessage(size_t role,Connection excludeConnection, const QString& message)
{
    shared_lock<shared_mutex> lg(lockUser_);
    {
        auto& roleView = userInfo_.get<RoleInfo>();
        auto p = roleView.equal_range(role);
        for (auto it = p.first; it != p.second; ++it)
        {
            if(it->connectId == excludeConnection)
            {
                continue;
            }

            gActionSimulationServer.sendNetMessage(it->connectId.ConnectionID,message);
        }
    }
}


void UserManager::sendUserMessage_(User userid, bool admin_Only,const QString& message)
{
    shared_lock<shared_mutex> lg(lockUser_);
    {
        auto& userView = userInfo_.get<UserId>();
        auto p = userView.equal_range(userid);
        for (auto it = p.first; it != p.second; ++it)
        {
            if(admin_Only && (it->role != static_cast<int>(UserRole::Administrator)))
            {
                continue;
            }

            gActionSimulationServer.sendNetMessage(it->connectId.ConnectionID,message);
        }
    }
}

void UserManager::sendUserMessage_(User userid,bool admin_Only,Connection excludeConnection, const QString& message)
{
    shared_lock<shared_mutex> lg(lockUser_);
    {
        auto& userView = userInfo_.get<UserId>();
        auto p = userView.equal_range(userid);
        for (auto it = p.first; it != p.second; ++it)
        {
            if(it->connectId == excludeConnection)
            {
                continue;
            }

            if(admin_Only && (it->role != static_cast<int>(UserRole::Administrator)))
            {
                continue;
            }

            gActionSimulationServer.sendNetMessage(it->connectId.ConnectionID,message);
        }
    }
}

void UserManager::sendMessage(bool admin_Only,const QString& message)
{
    shared_lock<shared_mutex> lg(lockUser_);
    {
        for (auto it = userInfo_.cbegin(); it != userInfo_.cend(); ++it)
        {
            if(admin_Only && (it->role != static_cast<int>(UserRole::Administrator)))
            {
                continue;
            }

            gActionSimulationServer.sendNetMessage(it->connectId.ConnectionID,message);
        }
    }
}

void UserManager::sendMessage(bool admin_Only,Jimmy::Connection excludeConnection, const QString& message)
{
    shared_lock<shared_mutex> lg(lockUser_);
    {
        for (auto it = userInfo_.cbegin(); it != userInfo_.cend(); ++it)
        {
            if(it->connectId == excludeConnection)
            {
                continue;
            }

            if(admin_Only && (it->role != static_cast<int>(UserRole::Administrator)))
            {
                continue;
            }

            gActionSimulationServer.sendNetMessage(it->connectId.ConnectionID,message);
        }
    }
}

void UserManager::clear()
{
    lock_guard<shared_mutex> lg(lockUser_);
    userInfo_.clear();
}

bool UserManager::existUser(User userID)
{
    shared_lock<shared_mutex> lg(lockUser_);
    auto& userView = userInfo_.get<UserId>();
    return userView.find(userID)!= userView.end();
}

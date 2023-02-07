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

#include "commonstruct.h"
#include <QVector>
#include <QHash>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/lambda/lambda.hpp>
#include <shared_mutex>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

struct ConnId{};
struct UserId{};
struct RoleInfo{};

struct UserInfo
{
    Jimmy::Connection connectId;
    Jimmy::User userId;
    size_t role;

    UserInfo(Jimmy::Connection connId)
        :UserInfo(connId,Jimmy::User(),0)
    {}


    UserInfo(Jimmy::Connection connId, Jimmy::User uId, size_t uRole)
        :connectId(connId)
        , userId(uId)
        , role(uRole)
    {}

    bool operator<(const UserInfo& e)const{ return connectId.ConnectionID < e.connectId.ConnectionID; }
    bool operator<=(const UserInfo& e)const{ return connectId.ConnectionID <= e.connectId.ConnectionID; }
};

struct UpdateUserInfo
{
    UpdateUserInfo(Jimmy::User uId, size_t uRole)
        :newUserID_(uId)
        ,userRole_(uRole)
    {}

    void operator()(UserInfo& e)
    {
        e.userId = newUserID_;
        e.role = userRole_;
    }
private:
    Jimmy::User newUserID_;
    size_t userRole_;
};

typedef boost::multi_index::multi_index_container <
    UserInfo,
    boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<boost::multi_index::tag<ConnId>, boost::multi_index::identity<UserInfo> >,
    boost::multi_index::ordered_non_unique<boost::multi_index::tag<UserId>, boost::multi_index::member<UserInfo, Jimmy::User, &UserInfo::userId> >,
    boost::multi_index::ordered_non_unique<boost::multi_index::tag<RoleInfo>, boost::multi_index::member<UserInfo, size_t, &UserInfo::role> >
    >
> RegisteredUserInfo;


class UserManager
{
public:
    UserManager();
    ~UserManager() = default;

    Jimmy::User registerConnection(size_t connection);

    void disconnect(size_t connection);

    void login(Jimmy::Connection connection,Jimmy::User userID,size_t role = Jimmy::UserRole::Normal);

    bool existUser(Jimmy::User userID);

    std::optional<UserInfo> getUserInfo(Jimmy::Connection connection);

    QVector<UserInfo> getConnectIdbyUser(Jimmy::User userID);

    void answerMessage(Jimmy::Connection connection, const QString& message);
    void sendMessage(bool admin_Only,const QString& message);
    void sendMessage(bool admin_Only,Jimmy::Connection excludeConnection, const QString& message);
    void sendUserMessage(Jimmy::User userid,bool admin_Only,const QString& message);
    void sendUserMessage(Jimmy::User userid,bool admin_Only,Jimmy::Connection excludeConnection,const QString& message);
    void sendRoleMessage(size_t role,const QString& message);
    void sendRoleMessage(size_t role,Jimmy::Connection excludeConnection, const QString& message);
    void clear();
private:
    void sendUserMessage_(Jimmy::User userid,bool admin_Only, const QString& message);
    void sendUserMessage_(Jimmy::User userid,bool admin_Only,Jimmy::Connection excludeConnection, const QString& message);
private:
    QVector<UserInfo> getConnectIdbyUsers_(const QVector<Jimmy::User>& vUserID);

    void releaseComponentsValue_(Jimmy::User userid);
private:
    std::shared_mutex lockUser_;
    RegisteredUserInfo userInfo_;
};


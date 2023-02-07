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
#include <QString>
#include <QJsonArray>
#include <chrono>
#include <array>
#include <variant>
#include <QHash>

namespace Jimmy
{

struct Connection
{
    Connection() = default;
    Connection(size_t val) { ConnectionID = val; }
    Connection(const Connection&) = default;
    Connection& operator=(const Connection&) = default;

    Connection(Connection&&) = default;
    Connection& operator=(Connection&&) = default;

    size_t ConnectionID{0};
};

inline bool operator<(const Connection& lhs,const Connection& rhs)
{
    return lhs.ConnectionID < rhs.ConnectionID;
}

inline bool operator<=(const Connection& lhs,const Connection& rhs)
{
    return lhs.ConnectionID <= rhs.ConnectionID;
}

inline bool operator==(const Connection& lhs,const Connection& rhs)
{
    return lhs.ConnectionID == rhs.ConnectionID;
}

struct User
{
    User() = default;
    User(size_t val) { userID = val; }
    User(const User&) = default;
    User& operator=(const User&) = default;

    User(User&&) = default;
    User& operator=(User&&) = default;

    size_t userID{0};
};

inline uint qHash(const User& user, uint seed = 0)
{
    return uint(((user.userID >> (8 * sizeof(uint) - 1)) ^ user.userID) & (~0U)) ^ seed;
}

inline bool operator<(const User& lhs,const User& rhs)
{
    return lhs.userID < rhs.userID;
}

inline bool operator<=(const User& lhs,const User& rhs)
{
    return lhs.userID <= rhs.userID;
}

inline bool operator==(const User& lhs,const User& rhs)
{
    return lhs.userID == rhs.userID;
}


enum UserRole
{
    Normal = 0,
    Administrator = 1,
    UserDefined = 2,
};

enum class ScheduledType
{
    Timer,
    Loop,
    Order,
};

struct ScheduledTask
{
    Jimmy::User userid{0};
    QString cid;                                    //component cid
    int times{0};
    size_t interval{0};                             //millisecond
    size_t counter{0};
    ScheduledType scheduledType;
    QJsonArray loopValue;
    std::chrono::steady_clock::time_point next_tp;  //next invoke time
};

struct ComponentChangeEvent
{
    Jimmy::User userid{0};
    QString cid;
    QString trigger;
    QJsonValue value;
    size_t counter{0};
};

}

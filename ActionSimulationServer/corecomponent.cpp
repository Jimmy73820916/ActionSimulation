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

#include "actionsimulationserver.h"
#include "corecomponent.h"
#include "logger.h"
#include "projectmanager.h"
#include <chrono>

using namespace std;

namespace Jimmy
{

std::optional<ScheduledTask> CoreComponent::analysisLoopbase(User userid,const QJsonObject& jo)
{
    auto elem = jo.value("_loop");
    if(!elem.isArray())
    {
        LOGERROR(QStringLiteral("[%1:%2] %3:%4 failed,_order is not an array")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(userid.userID)
            .arg(getID()));

        return nullopt;
    }

    ScheduledTask st;
    st.scheduledType = ScheduledType::Loop;
    st.loopValue = elem.toArray();
    if(st.loopValue.size() <= 1)
    {
        LOGERROR(QStringLiteral("[%1:%2] %3:%4 failed,_order's value is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(userid.userID)
            .arg(getID()));

        return nullopt;
    }

    st.userid = userid;
    st.cid = getID();

    auto elemItor = jo.find("_times");
    if(elemItor != jo.end())
    {
        if(!elemItor.value().isDouble())
        {
            LOGERROR(QStringLiteral("[%1:%2] %3:%4 failed,_times's value is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(userid.userID)
                .arg(getID()));
        }

        st.times = elemItor.value().toInt(-1);
    }

    elemItor = jo.find("_interval");
    if(elemItor != jo.end())
    {
        if(!elemItor.value().isDouble())
        {
            LOGERROR(QStringLiteral("[%1:%2] %3:%4 failed,_interval's value is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(userid.userID)
                .arg(getID()));
        }

        st.interval = elemItor.value().toInt(500);
    }

    if(st.interval < gActionSimulationServer.getProjectManager()->getMinTimerInternal())
    {
        st.interval = gActionSimulationServer.getProjectManager()->getDefaultTimerInternal();
    }

    st.next_tp = chrono::steady_clock::now() + chrono::milliseconds(st.interval);

    return st;
}

std::optional<ScheduledTask> CoreComponent::analysisOrderbase(User userid,const QJsonObject& jo)
{
    auto elem = jo.value("_order");
    if(!elem.isArray())
    {
        LOGERROR(QStringLiteral("[%1:%2] %3:%4 failed,_order is not an array")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(userid.userID)
            .arg(getID()));

        return nullopt;
    }

    ScheduledTask st;
    st.scheduledType = ScheduledType::Order;
    st.loopValue = elem.toArray();
    if(st.loopValue.size() != 3)
    {
        LOGERROR(QStringLiteral("[%1:%2] %3:%4 failed,_order is not an array")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(userid.userID)
            .arg(getID()));

        return nullopt;
    }

    if(!(st.loopValue[0].isDouble() && st.loopValue[1].isDouble() && st.loopValue[2].isDouble()))
    {
        LOGERROR(QStringLiteral("[%1:%2] %3:%4 failed,order[2] is not a number")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(userid.userID)
            .arg(getID()));

        return nullopt;
    }

    st.userid = userid;
    st.cid = getID();

    auto elemItor = jo.find("_times");
    if(elemItor != jo.end())
    {
        if(!elemItor.value().isDouble())
        {
            LOGWARN(QStringLiteral("[%1:%2] %3:%4 failed,_times's value is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(userid.userID)
                .arg(getID()));
        }

        st.times = elemItor.value().toInt(-1);
    }

    elemItor = jo.find("_interval");
    if(elemItor != jo.end())
    {
        if(!elemItor.value().isDouble())
        {
            LOGWARN(QStringLiteral("[%1:%2] %3:%4 failed,_interval's value is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(userid.userID)
                .arg(getID()));
        }

        st.interval = elemItor.value().toInt(gActionSimulationServer.getProjectManager()->getDefaultTimerInternal());
    }

    if(st.interval < gActionSimulationServer.getProjectManager()->getMinTimerInternal())
    {
        st.interval = gActionSimulationServer.getProjectManager()->getDefaultTimerInternal();
    }

    st.next_tp = chrono::steady_clock::now() + chrono::milliseconds(st.interval);

    return st;
}

std::optional<ScheduledTask> CoreComponent::analysisTimerbase(User userid,const QJsonObject& jo)
{
    ScheduledTask st;
    st.userid =userid;
    st.cid = getID();
    auto elem = jo.value("_timer");
    if (elem.isBool())
    {
        st.times = (elem.toBool()?-1:0);
        st.interval = gActionSimulationServer.getProjectManager()->getDefaultTimerInternal();

    }
    else if (elem.isObject())
    {
        auto timerElem = elem.toObject();
        auto timerItor = timerElem.find("times");
        if(timerItor != timerElem.end())
        {
            st.times = timerItor->toInt(-1);
        }

        timerItor = timerElem.find("interval");
        if(timerItor != timerElem.end())
        {
            st.interval = timerItor->toInt(gActionSimulationServer.getProjectManager()->getDefaultTimerInternal());
        }

        if(st.interval < gActionSimulationServer.getProjectManager()->getMinTimerInternal())
        {
            st.interval = gActionSimulationServer.getProjectManager()->getDefaultTimerInternal();
        }
    }

    st.next_tp = chrono::steady_clock::now() + chrono::milliseconds(st.interval);
    return st;
}

QString CoreComponent::getAnswerValue(const QJsonValue& value)
{
    QJsonObject jo;
    jo.insert("cid", getID());
    jo.insert("value", value);
    return QJsonDocument(jo).toJson(QJsonDocument::Compact);
}


}

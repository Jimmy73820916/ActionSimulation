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

#include "inputcomponent.h"
#include "usermanager.h"
#include "projectmanager.h"
#include "actionsimulationserver.h"
#include "commonfunction.h"
#include "logger.h"

using namespace std;
namespace Jimmy
{

ErrorCode InputComponent::start()
{
    if(getID().isEmpty())
    {
        LOGERROR(QStringLiteral("[%1:%2] getID is empty")
            .arg(__FUNCTION__).arg(__LINE__));
        return ErrorCode::ec_invalid_datatype;
    }

    return ErrorCode::ec_ok;
}

void InputComponent::stop()
{
    removeAllUser();
}

ErrorCode InputComponent::load(const QString& id,const QJsonObject& jo)
{
    setID(id);

    auto elemItor = jo.find("set_action_keep");
    if((elemItor == jo.end())||(!elemItor->isDouble()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 set_action_keep data type is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(getID()));

        return ErrorCode::ec_invalid_datatype;
    }
    setActionKeep(elemItor->toDouble());

    elemItor = jo.find("behavior_type");
    if((elemItor == jo.end())||(!elemItor->isDouble()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 behavior_type data type is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }
    setBehavior(static_cast<BehaviorType>(elemItor->toInt()));
    if(!((getBehavior() == BehaviorType::EqualInput)||(getBehavior() == BehaviorType::EqualInputIgnoreReset)))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 behavior_type data type is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }

    elemItor = jo.find("subscription");
    if((elemItor == jo.end())||(!elemItor->isArray()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 subscription data type is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }

    auto subscription = CommonFunction::transform_jatosl(elemItor->toArray());
    if(!subscription)
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 subscription member data type is invalid")
                 .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }
    setSubscription(subscription.value());

    elemItor = jo.find("default_value");
    if(elemItor == jo.end())
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 default_value is not exist")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(getID()));

        return ErrorCode::ec_invalid_datatype;
    }
    setDefaultValue(*elemItor);

    return ErrorCode::ec_ok;
}

QJsonValue InputComponent::getValue(User userid)
{
    {
        std::shared_lock<std::shared_mutex> lg(lockValue_);
        auto itor = userValue_.find(userid);
        if(itor != userValue_.end())
        {
            return itor.value()->value;
        }
    }

    return getDefaultValue();
}

void InputComponent::setValue(Connection connection,const QJsonValue& value)
{
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    const double minActionKeep = 0.1 ; //最小置位信号保持时间(秒)
    if (getActionKeep() > minActionKeep)
    {
        ScheduledTask st;

        {
            lock_guard<shared_mutex> lg(lockValue_);
            if (auto userVal = getUserValue_(userInfo->userId,true))
            {
                st.userid = userInfo->userId;
                st.cid = getID();
                st.times = (value == getDefaultValue()) ? 0 : 1;
                st.next_tp = chrono::steady_clock::now() + chrono::milliseconds(static_cast<int>(getActionKeep() * 1000));
                gActionSimulationServer.getProjectManager()->appendScheduledTask(st);
                userVal->value = value;
            }
        }

        if(st.times != 0)
        {
            gActionSimulationServer.getUserManager()->sendUserMessage(userInfo->userId,false,getAnswerValue(value));
        }

        return;
    }

    {
        lock_guard<shared_mutex> lg(lockValue_);
        auto userVal = getUserValue_(userInfo->userId,true);
        if (!userVal || userVal->value == value)
        {
            return;
        }

        userVal->value = value;

        if(getBehavior() == BehaviorType::EqualInputIgnoreReset)
        {
            if(userVal->value == getDefaultValue())
            {
                gActionSimulationServer.getUserManager()->sendUserMessage(userInfo->userId,false,connection,getAnswerValue(value));
                return;
            }
        }
    }

    gActionSimulationServer.getUserManager()->sendUserMessage(userInfo->userId,false,connection,getAnswerValue(value));
    gActionSimulationServer.getProjectManager()->notifyComponentChange(userInfo->userId,getID(),value);
}

void InputComponent::onTime(User userid,size_t counter)
{
    Q_UNUSED(counter)

    lock_guard<shared_mutex> lg(lockValue_);
    auto userVal = getUserValue_(userid,false);
    if (!userVal)
    {
        LOGERROR(QStringLiteral("[%1:%2] %3:%4 getUserValue_ is failed")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(userid.userID)
            .arg(getID()));

        return;
    }

    //延迟推送值改变信号
    gActionSimulationServer.getProjectManager()->notifyComponentChange(userid,getID(),userVal->value);
}

std::shared_ptr<UserValue> InputComponent::getUserValue_(User userID,bool create_on_not_exist)
{
    auto itor = userValue_.find(userID);
    if(itor != userValue_.end())
    {
        return itor.value();
    }

    if(create_on_not_exist)
    {
        if(gActionSimulationServer.getUserManager()->existUser(userID))
        {
            shared_ptr<UserValue> val = make_shared<UserValue>();
            val->value = getDefaultValue();
            userValue_.insert(userID,val);
            return val;
        }
    }

    return nullptr;
}

void InputComponent::removeAllUser()
{
    lock_guard<shared_mutex> lg(lockValue_);
    userValue_.clear();
}

void InputComponent::removeUser(User userid)
{
    int exist(0);
    {
        lock_guard<shared_mutex> lg(lockValue_);
        auto userValue = getUserValue_(userid,false);
        if(!userValue)
        {
            return;
        }

        if(userid == 0)
        {
            userValue->value = getDefaultValue();
        }
        else
        {
            userValue_.remove(userid);
        }
    }

    if(exist)
    {
        ScheduledTask st;
        st.userid = userid;
        st.cid = getID();
        gActionSimulationServer.getProjectManager()->removeScheduledTask(st);
    }
}

void InputComponent::onAction(User userid,const QString& /*trigger*/,const QJsonValue& value)
{
    {
        lock_guard<shared_mutex> lg(lockValue_);
        auto userValue = getUserValue_(userid,true);

        if(userValue->value == value)
        {
            return;
        }

        userValue->value = value;
    }

    gActionSimulationServer.getUserManager()->sendUserMessage(userid,false,getAnswerValue(value));
    return;
}


}


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

#include "normalcomponent.h"
#include "usermanager.h"
#include "projectmanager.h"
#include "actionsimulationserver.h"
#include "logger.h"
#include "commonfunction.h"
#include "actionscript.h"

using namespace std;
namespace Jimmy
{

ErrorCode NormalComponent::start()
{   
    if(getID().isEmpty())
    {
        LOGERROR(QStringLiteral("[%1:%2] getID is empty")
            .arg(__FUNCTION__).arg(__LINE__));
        return ErrorCode::ec_error;
    }

    ErrorCode ret(ErrorCode::ec_ok);
    if (getBehavior() == BehaviorType::Script)
    {
        if(getRole().isEmpty())
        {
            LOGERROR(QStringLiteral("[%1:%2]component:%3 role is invalid")
                .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
            return ErrorCode::ec_error;
        }

        actionScript_ = make_shared<ActionScript>(getRole());
        ret = actionScript_->start();
    }

    if (ret != ErrorCode::ec_ok)
    {
        return ret;
    }

	auto value = getDefaultValue();
	if (value.isString() && (value.toString().compare(CommonConst::CalculateDefaultValue) == 0))
	{
        ret = onInitialize_();
	}

    return ret;
}

void NormalComponent::stop()
{
    if (getBehavior() == BehaviorType::Script)
    {
        actionScript_->stop();
        actionScript_.reset();
    }

    removeAllUser();
}

ErrorCode NormalComponent::reloadRole(const QString& role)
{
    if(getBehavior() == BehaviorType::Script)
    {
        if(role_.compare(role) == 0)
        {
            return actionScript_->reload();
        }
    }

    return ErrorCode::ec_ok;
}

ErrorCode NormalComponent::load(const QString& id,const QJsonObject& jo)
{
    setID(id);

    auto elemItor = jo.find("behavior_type");
    if((elemItor == jo.end())||(!elemItor->isDouble()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 behavior_type data type is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }
    setBehavior(static_cast<BehaviorType>(elemItor->toInt()));

    elemItor = jo.find("component_type");
    if((elemItor == jo.end())||(!elemItor->isDouble()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 component_type data type is invalid")
                 .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
             return ErrorCode::ec_invalid_datatype;
    }
    setType(static_cast<ComponentType>(elemItor->toInt()));
    if(!((getType() == ComponentType::Output)||(getType() == ComponentType::Internal)))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 component_type data type is not Output or Internal")
                 .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
             return ErrorCode::ec_invalid_datatype;
    }

    elemItor = jo.find("role");
    if((elemItor != jo.end())&&(elemItor->isString()))
    {
        setRole(elemItor->toString());
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

    elemItor = jo.find("reference");
    if((elemItor == jo.end())||(!elemItor->isArray()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 reference data type is invalid")
                 .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }

    auto reference = CommonFunction::transform_jatosl(elemItor->toArray());
    if(!reference)
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 reference member data type is invalid")
                 .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }
    setReference(reference.value());


    elemItor = jo.find("respond_boardcast");
    if((elemItor == jo.end())||(!elemItor->isArray()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 respond_boardcast data type is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }

    auto respondBoardcast = CommonFunction::transform_jatosl(elemItor->toArray());
    if(!respondBoardcast)
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 respond_boardcast member data type is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_invalid_datatype;
    }
    setRespondBoardcast(respondBoardcast.value());


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

QJsonValue NormalComponent::getValue(User userid)
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

bool NormalComponent::sendAdminOnly()
{
    return getType() == ComponentType::Internal;
}

void NormalComponent::setValue(Connection connection,const QJsonValue& value)
{
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    setValue_(userInfo->userId,value,false);
}

void NormalComponent::onBoardcast(User userid)
{
    if(!actionScript_)
    {
        LOGERROR(QStringLiteral("[%1:%2] component:{%3} is not start")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(getID()));

        return;
    }
    auto results = actionScript_->onAction(collectInputs(userid,CommonConst::Boardcast,QJsonValue()));
    if(!results)
    {
        return;
    }
    analysisResult(userid,results.value());
}

ErrorCode NormalComponent::onInitialize_()
{
	auto results = actionScript_->onInitialize(collectInputs());
	if (!results)
	{
        ErrorCode::ec_error;
	}

    auto result = results.value();

	if (result.empty()|| (!result.contains("_value")))
	{
        ErrorCode::ec_error;
	}

    setDefaultValue(result.value("_value"));
    return ErrorCode::ec_ok;
}

void NormalComponent::onAction(User userid,const QString& trigger,const QJsonValue& value)
{
    switch(getBehavior())
    {
    case BehaviorType::EqualInput:
    case BehaviorType::EqualInputIgnoreReset:
    {
        setValue_(userid,value,false);
        return;
    }
    case BehaviorType::ReversalIgnoreOther:
    {
        if(!value.isBool())
        {
            LOGERROR(QStringLiteral("[%1:%2] component:{%3} is only received bool value. now received [%4]")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(getID())
                .arg(value.toString()));

            return;
        }

        if(value.toBool())
        {
            lock_guard<shared_mutex> lg(lockValue_);
            auto userVal = getUserValue_(userid,true);
            userVal->value = !userVal->value.toBool();
            gActionSimulationServer.getUserManager()->sendUserMessage(userid,sendAdminOnly(),getAnswerValue(value));
            gActionSimulationServer.getProjectManager()->notifyComponentChange(userid,getID(),userVal->value);
        }

        return;
    }
    default:
    {
        if(!actionScript_)
        {
            LOGERROR(QStringLiteral("[%1:%2] component:{%3} is not start")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(getID()));

            return;
        }
        auto results = actionScript_->onAction(collectInputs(userid,trigger,value));
        if(!results)
        {
            return;
        }
        analysisResult(userid,results.value());
    }
    }
}

void NormalComponent::onTime(User userid,size_t counter)
{
    if(!actionScript_)
    {
        LOGERROR(QStringLiteral("[%1:%2] component:{%3} is not start")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(getID()));

        return;
    }
    auto results = actionScript_->onTime(collectInputs(userid,counter));
    if(!results)
    {
        return;
    }
    analysisResult(userid,results.value());
}

void NormalComponent::onLoop(User userid,const QJsonValue& value)
{
    setValue_(userid,value,false);
}

QJsonObject NormalComponent::collectInputs()
{
	QJsonObject jo;

	auto items = getSubscription() + getReference();
	items.removeDuplicates();

	Q_FOREACH(const auto & item, items)
	{
		auto pComponent = gActionSimulationServer.getProjectManager()->getComponent(item);

		if (!pComponent)
		{
			LOGFATAL(QStringLiteral("[%1:%2] component:{%3} subscription {%4} is not exist")
				.arg(__FUNCTION__)
				.arg(__LINE__)
				.arg(getID())
				.arg(item));

			return QJsonObject();
		}

		jo.insert(item, pComponent->getDefaultValue());
	}

	return jo;
}

QJsonObject NormalComponent::collectInputs(User userid,size_t counter)
{
    QJsonObject jo;

    jo.insert("_userid", static_cast<qint64>(userid.userID));

    {
        jo.insert("_cid", getID());
        jo.insert("_default_Value", getDefaultValue());

        shared_lock<shared_mutex> lock_value(lockValue_);
        auto userVal = getUserValue_(userid,false);
        if(userVal)
        {
            jo.insert("_value", userVal->value);
            jo.insert("_cache", userVal->cache);
        }
        else
        {
            jo.insert("_value", getDefaultValue());
            jo.insert("_cache", QJsonValue());
        }
        jo.insert("_counter", static_cast<qint64>(counter));
    }

    getRelationParams(userid,"",jo);

    jo.insert(CommonConst::Boardcast, QJsonArray::fromStringList(gActionSimulationServer.getProjectManager()->getIntersectBoardcast(userid, getRespondBoardcast())));

    return jo;
}

QJsonObject NormalComponent::collectInputs(User userid,const QString& trigger,const QJsonValue& value)
{
    QJsonObject jo;

    jo.insert("_userid", static_cast<qint64>(userid.userID));
    jo.insert("_cid", getID());
    jo.insert("_trigger", trigger);

    if (trigger != CommonConst::CalculateDefaultValue)
    {
        {
            shared_lock<shared_mutex> lock_value(lockValue_);
            auto userVal = getUserValue_(userid,false);
            if(userVal)
            {
                jo.insert("_value", userVal->value);
                jo.insert("_cache", userVal->cache);
            }
            else
            {
                jo.insert("_value", getDefaultValue());
                jo.insert("_cache", QJsonValue());
            }
        }

        jo.insert("_default_Value", getDefaultValue());
    }

    if((trigger.compare(CommonConst::Boardcast)==0)||(trigger.compare(CommonConst::CalculateDefaultValue) == 0))
    {
        getRelationParams(userid,"",jo);
    }
    else
    {
        jo.insert(trigger, value);
        getRelationParams(userid,trigger,jo);
    }

    jo.insert(CommonConst::Boardcast, QJsonArray::fromStringList(gActionSimulationServer.getProjectManager()->getIntersectBoardcast(userid, getRespondBoardcast())));

    return jo;
}

void NormalComponent::getRelationParams(User userid,const QString& trigger,QJsonObject& jo)
{
    auto items = getSubscription() + getReference();
    items.removeDuplicates();

    Q_FOREACH(const auto& item, items)
    {
        if(item.compare(trigger)==0)
        {
            continue;
        }

        auto pComponent = gActionSimulationServer.getProjectManager()->getComponent(item);

        if (!pComponent)
        {
            LOGFATAL(QStringLiteral("[%1:%2] component:{%3} subscription {%4} is not exist")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(getID())
                .arg(item));

            return;
        }

        jo.insert(item, pComponent->getValue(userid));
    }
}

std::shared_ptr<UserValue> NormalComponent::getUserValue_(Jimmy::User userID,bool create_on_not_exist)
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

void NormalComponent::setValue_(User userid,const QJsonValue& value,bool enableSchedulePossible)
{
    {
        lock_guard<shared_mutex> lg(lockValue_);
        auto userValue = getUserValue_(userid,true);

        if(enableSchedulePossible)
        {
            userValue->schedulePossible = true;
            userValue->cache = QJsonValue();
        }

        if(userValue->value == value)
        {
            return;
        }

        userValue->value = value;
    }

    if(BehaviorType::EqualInputIgnoreReset == getBehavior())
    {
        if(value == getDefaultValue())
        {
            return;
        }
    }

    gActionSimulationServer.getUserManager()->sendUserMessage(userid,sendAdminOnly(),getAnswerValue(value));
    gActionSimulationServer.getProjectManager()->notifyComponentChange(userid,getID(),value);
    return;
}

void NormalComponent::analysisResult(User userid, const QJsonObject& result)
{
    if (result.empty())
    {
        return;
    }

    if (result.contains("_loop"))
    {
        return analysisLoop(userid, result);
    }
    else if (result.contains("_order"))
    {
        return analysisOrder(userid, result);
    }

    bool valueChange(false);
    auto valueItor = result.find("_value");
    if(valueItor != result.end())
    {
        lock_guard<shared_mutex> lock_value(lockValue_);
        auto userVal = getUserValue_(userid,true);
        if (!userVal)
        {
            return;
        }

        if(userVal->schedulePossible)
        {
            stopScheduledTask(userid);
            userVal->schedulePossible = false;
        }

        auto cacheItor = result.find("_cache");
        if (cacheItor != result.end())
        {
            userVal->cache = cacheItor.value();
        }

        if((valueItor != result.end()) && (userVal->value != valueItor.value()))
        {
            userVal->value = valueItor.value();
            valueChange = true;
        }
    }

    if (valueChange)
    {
        gActionSimulationServer.getUserManager()->sendUserMessage(userid,sendAdminOnly(),getAnswerValue(valueItor.value()));
        gActionSimulationServer.getProjectManager()->notifyComponentChange(userid, getID(),valueItor.value());
    }

    if (result.contains("_timer"))
    {
        auto st = analysisTimerbase(userid, result);
        if(st)
        {
            auto& stValue = st.value();
            if(stValue.times == 0)
            {
                gActionSimulationServer.getProjectManager()->removeScheduledTask(stValue);
            }
            else
            {
                gActionSimulationServer.getProjectManager()->appendScheduledTask(stValue);
            }
        }
    }
}

void NormalComponent::stopScheduledTask(User userid)
{
    //任何有返回值得调用都终止 Loop和order
    ScheduledTask st;
    st.userid = userid;
    st.cid = getID();
    gActionSimulationServer.getProjectManager()->removeScheduledTask(st);
}

void NormalComponent::analysisLoop(User userid,const QJsonObject& jo)
{
    auto loopTask = analysisLoopbase(userid,jo);
    if(!loopTask)
    {
        return;
    }

    setValue_(userid,loopTask.value().loopValue[0],true);

    gActionSimulationServer.getProjectManager()->appendScheduledTask(loopTask.value());
    return;
}

void NormalComponent::analysisOrder(User userid,const QJsonObject& jo)
{
    auto st = analysisOrderbase(userid,jo);
    if(!st)
    {
        return;
    }

    auto& ot = st.value();

    setValue_(userid,ot.loopValue[0],true);

    ot.loopValue[0] = ot.loopValue[0].toDouble() + ot.loopValue[2].toDouble();
    if (((ot.loopValue[2].toDouble() > 0) && (ot.loopValue[0].toDouble() <= ot.loopValue[1].toDouble())) ||
        ((ot.loopValue[2].toDouble() < 0) && (ot.loopValue[0].toDouble() >= ot.loopValue[1].toDouble())))
    {
        //保证至少循环一次
        gActionSimulationServer.getProjectManager()->appendScheduledTask(move(ot));
    }
}

void NormalComponent::removeUser(User userid)
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

void NormalComponent::removeAllUser()
{
    lock_guard<shared_mutex> lg(lockValue_);
    userValue_.clear();
}

}

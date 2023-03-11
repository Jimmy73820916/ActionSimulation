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

#include "teammastercomponent.h"
#include "usermanager.h"
#include "projectmanager.h"
#include "actionsimulationserver.h"
#include "logger.h"
#include "commonfunction.h"
#include "actionscript.h"
#include <QPair>


using namespace std;
namespace Jimmy
{

ErrorCode TeamMasterComponent::start()
{
    if(getID().isEmpty())
    {
        LOGERROR(QStringLiteral("[%1:%2] getID is empty")
            .arg(__FUNCTION__).arg(__LINE__));
        return ErrorCode::ec_error;
    }

    foreach(auto item,subscription_)
    {
        if(slaves_.contains(item))
        {
            LOGERROR(QStringLiteral("[%1:%2] can't subscripe oneself")
                .arg(__FUNCTION__).arg(__LINE__));
            return ErrorCode::ec_error;
        }
    }

    foreach(auto item, slaves_.keys())
    {
        if(reference_.contains(item))
        {
            reference_.removeOne(item);
        }
    }

    if(getRole().isEmpty())
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 role is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_error;
    }

    actionScript_ = make_shared<ActionScript>(getRole());
    ErrorCode ret = actionScript_->start();

	if (ret == ErrorCode::ec_ok)
	{
        foreach(auto& item, slaves_.values())
        {
            auto val = item->getDefaultValue();
			if (val.isString() && (val.toString().compare(CommonConst::CalculateDefaultValue) == 0))
			{
				ret = onInitialize_();
				break;
			}
        }
	}

	return ret;
}

void TeamMasterComponent::stop()
{
    if (getBehavior() == BehaviorType::Script)
    {
        actionScript_->stop();
        actionScript_.reset();
    }

    removeAllUser();
}


ErrorCode TeamMasterComponent::reloadRole(const QString& role)
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

ErrorCode TeamMasterComponent::load(const QString& id,const QJsonObject& jo)
{
    setID(id);

    auto elemItor = jo.find("role");
    if((elemItor == jo.end())||(!elemItor->isString()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 role is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_error;
    }
    setRole(elemItor->toString());

    elemItor = jo.find("team");
    if((elemItor == jo.end())&&(elemItor->isString()))
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 team is invalid")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_error;
    }
    setTeam(elemItor->toString());

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


    __super::setDefaultValue(QJsonValue{0});

    return ErrorCode::ec_ok;
}

QJsonValue TeamMasterComponent::getValue(User userid)
{
    {
        std::shared_lock<std::shared_mutex> lg(lockValue_);
        auto itor = userValue_.find(userid);
        if(itor != userValue_.end())
        {
            return itor.value()->value;
        }
    }

    return __super::getDefaultValue();
}

void TeamMasterComponent::setValue(Connection connection,const QJsonValue& value)
{
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    if(!value.isObject())
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 value is not a json object")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return;
    }

    setValue_(userInfo->userId,value.toObject());
}

void TeamMasterComponent::onBoardcast(User userid)
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

ErrorCode TeamMasterComponent::onInitialize_()
{
	auto results = actionScript_->onInitialize(collectInputs());
	if (!results)
	{
        return ErrorCode::ec_error;
	}

	auto result = results.value();
	if (result.empty())
	{
        return ErrorCode::ec_error;
	}

	foreach(auto item, slaves_)
	{
        auto val = item->getDefaultValue();
		if (val.isString() && (val.toString().compare(CommonConst::CalculateDefaultValue) == 0))
		{
            if (!result.contains(item->getID()))
            {
				LOGERROR(QStringLiteral("[%1:%2] component:{%3} is not initialize")
					.arg(__FUNCTION__)
					.arg(__LINE__)
					.arg(item->getID()));

                return ErrorCode::ec_error;
            }

            item->setDefaultValue(result.value(item->getID()));
		}
	}

    return ErrorCode::ec_ok;
}

void TeamMasterComponent::onAction(User userid,const QString& trigger,const QJsonValue& value)
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

void TeamMasterComponent::onTime(User userid,size_t counter, size_t interval)
{
    if(!actionScript_)
    {
        LOGERROR(QStringLiteral("[%1:%2] component:{%3} is not start")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(getID()));

        return;
    }
    auto results = actionScript_->onTime(collectInputs(userid,counter,interval));
    if(!results)
    {
        return;
    }
    analysisResult(userid,results.value());
}

QJsonObject TeamMasterComponent::collectInputs()
{
	QJsonObject jo;

	foreach (auto item, slaves_)
	{
		jo.insert(QStringLiteral("%1").arg(item->getID()), item->getDefaultValue());
	}

	auto items = getSubscription() + getReference();
	items.removeDuplicates();

	Q_FOREACH(const auto & item, items)
	{
		auto pComponent = gActionSimulationServer.getProjectManager()->getComponent(item);

		if (!pComponent)
		{
			LOGFATAL(QStringLiteral("[%1:%2] component:{%3} subscription {%4} is not exist")
                .arg(__FUNCTION__,__LINE__)
                .arg(getID(),item));

			return QJsonObject();;
		}

		jo.insert(item, pComponent->getDefaultValue());
	}

	return jo;
}

QJsonObject TeamMasterComponent::collectInputs(User userid,size_t counter,size_t interval)
{
    QJsonObject jo;

    jo.insert("_userid", static_cast<qint64>(userid.userID));

    {
        shared_lock<shared_mutex> lock_value(lockValue_);
        auto userVal = getUserValue_(userid,true);
        jo.insert("_cache", userVal->cache);
        jo.insert("_counter", static_cast<qint64>(counter));
        jo.insert("_interval", static_cast<qint64>(interval));

        auto values = slaveValue_[userid];

        foreach(auto item, slaves_)
        {
            auto slaveValue = values.find(item->getID());
            if(slaveValue != values.end())
            {
                jo.insert(item->getID(), slaveValue.value());
            }
            else
            {
                jo.insert(item->getID(), item->getDefaultValue());
            }
        }
    }

	foreach(auto item, slaves_)
	{
		jo.insert(QStringLiteral("_%1_default_Value").arg(item->getID()), item->getDefaultValue());
	}

    getRelationParams(userid,"",jo);

    jo.insert(CommonConst::Boardcast, QJsonArray::fromStringList(gActionSimulationServer.getProjectManager()->getIntersectBoardcast(userid, getRespondBoardcast())));

    return jo;
}

QJsonObject TeamMasterComponent::collectInputs(User userid,const QString& trigger,const QJsonValue& value)
{
    QJsonObject jo;

    jo.insert("_userid", static_cast<qint64>(userid.userID));
    jo.insert("_trigger", trigger);

    {
        shared_lock<shared_mutex> lock_value(lockValue_);
        auto userVal = getUserValue_(userid, true);
        jo.insert("_cache", userVal->cache);

        auto values = slaveValue_[userid];

        foreach(auto item, slaves_)
        {
            auto slaveValue = values.find(item->getID());
            if (slaveValue == values.end())
            {
                jo.insert(QStringLiteral("%1").arg(item->getID()), item->getDefaultValue());
            }
            else
            {
                jo.insert(item->getID(), slaveValue.value());
            }
        }
    }

    foreach(auto item, slaves_)
    {
        jo.insert(QStringLiteral("_%1_default_Value").arg(item->getID()), item->getDefaultValue());
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

void TeamMasterComponent::getRelationParams(User userid,const QString& trigger,QJsonObject& jo)
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
                .arg(__FUNCTION__,__LINE__)
                .arg(getID(),item));

            return;
        }

        jo.insert(item, pComponent->getValue(userid));
    }
}

void TeamMasterComponent::setValue_(User userid,const QJsonObject& value)
{
    QVector<QPair<QString,QJsonValue>> valueChanged;
    {
        lock_guard<shared_mutex> lock_value(lockValue_);
        auto slaveValueItor = slaveValue_.find(userid);
        if (slaveValueItor != slaveValue_.end())
        {
            for (auto itor = value.constBegin(); itor != value.constEnd(); ++itor)
            {
                if (slaves_.contains(itor.key()))
                {
                    auto val = slaveValueItor.value().find(itor.key());
                    if (val == slaveValueItor.value().end())
                    {
                        slaveValueItor.value().insert(itor.key(), itor.value());
                        if (slaves_[itor.key()]->getDefaultValue() != itor.value())
                        {
                            valueChanged.push_back(QPair(itor.key(), itor.value()));
                        }
                    }
                    else
                    {
                        if (val.value() != itor.value())
                        {
                            val.value() = itor.value();
                            valueChanged.push_back(QPair(itor.key(), itor.value()));
                        }
                    }
                }
            }
        }

        if(!valueChanged.empty())
        {
            auto userVal = getUserValue_(userid,true);
            userVal->value = userVal->value.toInt() + 1;
        }
    }

    foreach(auto item,valueChanged)
    {
        gActionSimulationServer.getUserManager()->sendUserMessage(userid,true,getAnswerValue(item.first,item.second));
        gActionSimulationServer.getProjectManager()->notifyComponentChange(userid,item.first,item.second);
    }
}

std::shared_ptr<UserValue> TeamMasterComponent::getUserValue_(Jimmy::User userID,bool create_on_not_exist)
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
            val->value = __super::getDefaultValue();
            userValue_.insert(userID,val);
            return val;
        }
    }

    return nullptr;
}

void TeamMasterComponent::analysisResult(User userid, const QJsonObject& result)
{
    if (result.empty())
    {
        return;
    }

    {
		QJsonDocument jd(result);
        LOGINFO(jd.toJson(QJsonDocument::Compact));
    }

    setValue_(userid,result);

	auto cacheItor = result.find("_cache");
	if (cacheItor != result.end())
	{
		lock_guard<shared_mutex> lock_value(lockValue_);
		auto userValue = userValue_[userid];
        userValue->cache = cacheItor.value();
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

QString TeamMasterComponent::getAnswerValue(const QString& slaveID,const QJsonValue& value)
{
    QJsonObject jo;
    jo.insert("cid", slaveID);
    jo.insert("value", value);
    return QJsonDocument(jo).toJson(QJsonDocument::Compact);
}

QJsonValue TeamMasterComponent::getValue(User userid,const QString& slaveID)
{
    std::shared_lock<std::shared_mutex> lg(lockValue_);
    auto uv = slaveValue_.find(userid);
    if(uv != slaveValue_.end())
    {
        auto value = uv.value().find(slaveID);
        if(value !=uv.value().end())
        {
            return value.value();
        }
    }

    return getDefaultValue(slaveID);
}

QJsonValue TeamMasterComponent::getDefaultValue(const QString& slaveID)
{
    return slaves_[slaveID]->getDefaultValue();
}

void TeamMasterComponent::setSlave(const QString& slaveID, CoreComponent* slave)
{
    slaves_.insert(slaveID, slave);
}

QHash<QString, QJsonValue> TeamMasterComponent::getSlavesDefaultValue()
{
    QHash<QString, QJsonValue> items;

	foreach(auto item, slaves_)
	{
        items.insert(item->getID(), item->getDefaultValue());
	}

    return items;
}

void TeamMasterComponent::removeUser(User userid)
{
    int exist(0);
    {
        lock_guard<shared_mutex> lg(lockValue_);

        if(userid == 0)
        {
            userValue_[userid]->value = QJsonValue();
            userValue_[userid]->schedulePossible = false;
            userValue_[userid]->cache = QJsonValue();

            slaveValue_[userid] = getSlavesDefaultValue();
        }
        else
        {
            userValue_.remove(userid);
            slaveValue_.remove(userid);
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

void TeamMasterComponent::removeAllUser()
{
    lock_guard<shared_mutex> lg(lockValue_);
    userValue_.clear();
    slaveValue_.clear();
}

}



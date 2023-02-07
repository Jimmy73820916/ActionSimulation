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
#include "projectmanager.h"
#include "appconfig.h"
#include "usermanager.h"
#include "logger.h"
#include <QJsonDocument>
#include "inputcomponent.h"
#include "normalcomponent.h"
#include "teammastercomponent.h"
#include "teamslavecomponent.h"

using namespace std;
using namespace Jimmy;

const char* const ProjectManager::Action = "action";
const char* const ProjectManager::Succeed = "succeed";
const char* const ProjectManager::Failed = "failed";
const char* const ProjectManager::Result = "result";
const char* const ProjectManager::Reason = "reason";

ProjectManager::ProjectManager()
{
    initializesDispatcher();
    commandTCPDataThread_ = std::thread(std::bind(&ProjectManager::commandTcpDataThread, this));

    projectStatus_ = ProjectStatus::invalid;
    loadProject_();
}

ProjectManager::~ProjectManager()
{
    isRun_ = false;

    if(getStatus() == ProjectStatus::running)
    {
        stopProject_();
    }
}

void ProjectManager::loadProject_()
{
    QFile file(gActionSimulationServer.getAppConfig()->getProjectFile().absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        LOGERROR(QStringLiteral("[%1:%2] load %3 failed error code:%4")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(gActionSimulationServer.getAppConfig()->getProjectFile().absoluteFilePath())
            .arg(errno));

        return;
    }

    QJsonParseError error;
    QJsonDocument jd = QJsonDocument::fromJson(file.readAll(),&error);

    if(error.error!=QJsonParseError::NoError)
    {
        LOGERROR(QStringLiteral("[%1:%2] load %3 is failed")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(gActionSimulationServer.getAppConfig()->getProjectFile().absoluteFilePath()));

        return;
    }

    QJsonObject docObj = jd.object();
    auto memItor = docObj.find("project");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        LOGERROR(QStringLiteral("[%1:%2] key project is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return;
    }

    auto json_general = memItor->toObject();

    memItor = docObj.find("categories");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        LOGERROR(QStringLiteral("[%1:%2] key filter is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return;
    }
    json_category_ = memItor->toObject();

    memItor = docObj.find("components");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        LOGERROR(QStringLiteral("[%1:%2] key components is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return;
    }
    json_components_ = memItor->toObject();

    if (loadGeneral(json_general)
        && loadComponent())
    {
        projectStatus_ = ProjectStatus::stopped;
        return;
    }

    return;
}

bool ProjectManager::loadGeneral(const QJsonObject& jo)
{
    auto elemItor = jo.find("project_type");
    if(elemItor == jo.end())
    {
        LOGERROR(QStringLiteral("[%1:%2]load project project_type is failed")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return false;
    }

    projectType_ = (elemItor->toString() == CommonConst::multi_users) ? ProjectType::MultiUser : ProjectType::SingleUser;

    min_timer_interval_ = 10;
    elemItor = jo.find("min_timer_interval");
    if(elemItor == jo.end())
    {
        LOGERROR(QStringLiteral("[%1:%2]load project min_timer_interval is failed")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return false;
    }
    min_timer_interval_ = elemItor->toInt();

    default_timer_interval_ = 100;
    elemItor = jo.find("default_timer_interval");
    if(elemItor == jo.end())
    {
        LOGERROR(QStringLiteral("[%1:%2]load project default_timer_interval is failed")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return false;
    }
    default_timer_interval_ = elemItor->toInt();
    return true;
}

 std::shared_ptr<CoreComponent> ProjectManager::createComponent(Jimmy::ComponentType type)
 {
     switch (type)
     {
     case ComponentType::Input: return make_shared<InputComponent>();
     case ComponentType::TeamMaster: return make_shared<TeamMasterComponent>();
     case ComponentType::TeamSlave: return make_shared<TeamSlaveComponent>();
     case ComponentType::Internal:return make_shared<NormalComponent>();
     case ComponentType::Output: return make_shared<NormalComponent>();
     default:
     {
         return nullptr;
     }
     }
 }

bool ProjectManager::loadComponent()
{
    components_.clear();
    subscriptionComponents_.clear();

    for(auto itor = json_components_.constBegin();itor!=json_components_.constEnd();++itor)
    {
       if(!itor->isObject())
       {
           LOGERROR(QStringLiteral("[%1:%2]components data type is invalid")
               .arg(__FUNCTION__)
               .arg(__LINE__));

           return false;
       }

       QJsonObject jo = itor.value().toObject();

       auto elemItor = jo.find("component_type");
       if((elemItor == jo.end())||(!elemItor->isDouble()))
       {
           LOGERROR(QStringLiteral("[%1:%2]component:%3 component_type data type is invalid")
               .arg(__FUNCTION__)
               .arg(__LINE__)
               .arg(itor.key()));

           return false;
       }

       auto component = createComponent(static_cast<Jimmy::ComponentType>(elemItor->toInt()));
       if(!component)
       {
           LOGERROR(QStringLiteral("[%1:%2]component:%3 component_type data type is invalid")
               .arg(__FUNCTION__)
               .arg(__LINE__)
               .arg(itor.key()));

           return false;
       }

       if(component->load(itor.key(),jo)!=ErrorCode::ec_ok)
       {
           return false;
       }

       components_.insert(component->getID(), component);
    }

    return true;
}

void ProjectManager::initializesDispatcher()
{
    commandDispatcher_.insert("set_log", std::bind(&ProjectManager::setLog, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("set_boardcast", std::bind(&ProjectManager::setBoardcastCode, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("cancel_boardcast", std::bind(&ProjectManager::cancelBoardcastCode, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("get_boardcast", std::bind(&ProjectManager::getBoardcastCode, this, placeholders::_1, placeholders::_2));

    commandDispatcher_.insert("load", std::bind(&ProjectManager::loadProject, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("run", std::bind(&ProjectManager::runProject, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("stop", std::bind(&ProjectManager::stopProject, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("reset", std::bind(&ProjectManager::resetProject, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("get_project_status", std::bind(&ProjectManager::getProjectStatus, this, placeholders::_1, placeholders::_2));

    commandDispatcher_.insert("notify", std::bind(&ProjectManager::notify, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("reload_script", std::bind(&ProjectManager::reloadScript, this, placeholders::_1, placeholders::_2));

    commandDispatcher_.insert("login", std::bind(&ProjectManager::setLogin, this, placeholders::_1, placeholders::_2));

    commandDispatcher_.insert("query_all_value", std::bind(&ProjectManager::queryAllValue, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("query_value", std::bind(&ProjectManager::queryValue, this, placeholders::_1, placeholders::_2));
    commandDispatcher_.insert("component_status_change", std::bind(&ProjectManager::componentStatusChange, this, placeholders::_1, placeholders::_2));
}

void ProjectManager::pushMessage(size_t connectionId, const std::string& message)
{
    {
        lock_guard<mutex> lg(lockMsgData_);
        msgData_.push_back(QPair(Connection{connectionId},QString::fromLocal8Bit(message.c_str())));
    }

    cvMsgData_.notify_one();
}

void ProjectManager::commandTcpDataThread()
{
    while (isRun_)
    {
        QQueue<QPair<Connection,QString>> TcpData;
        {
            unique_lock<mutex> lg(lockMsgData_);
            cvMsgData_.wait(lg, [this] {return (!isRun_) || (!msgData_.empty()); });
            if (!isRun_) { break; }
            TcpData.swap(msgData_);
        }

        while (!TcpData.empty())
        {
            auto pData = TcpData.dequeue();
            disposeCommand(pData.first, pData.second);
        }
    }
}

void ProjectManager::disposeCommand(Jimmy::Connection connection, const QString& message)
{
    QJsonParseError error;
    QJsonDocument jd = QJsonDocument::fromJson(message.toLocal8Bit(),&error);

    if(error.error!=QJsonParseError::NoError)
    {
        LOGERROR(QStringLiteral("[%1:%2] 解析 %3 失败")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(message));

        return;
    }

    QString action;

    QJsonObject msg = jd.object();
    auto elemItor = msg.find(Action);
    if(elemItor == msg.end())
    {
        action = "component_status_change";
    }
    else
    {
        if(!elemItor->isString())
        {
            LOGERROR(QStringLiteral("[%1:%2] %3  action is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(message));

            return;
        }
        action = elemItor->toString();
    }

    auto itor = commandDispatcher_.find(action);
    if (itor == commandDispatcher_.end())
    {
        msg.insert(Result, Failed);
        msg.insert(Reason, "unsupported action");

        gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(msg).toJson(QJsonDocument::Compact));
        return;
    }

    itor.value()(connection, msg);
}

void ProjectManager::run()
{
    if (projectStatus_ == ProjectStatus::stopped)
    {
        projectStatus_ = ProjectStatus::prepare;
        projectStatus_ = (runProject_() == ErrorCode::ec_ok)?ProjectStatus::running:ProjectStatus::stopped;
    }
}

void ProjectManager::stop()
{
    if (projectStatus_ == ProjectStatus::running)
    {
        projectStatus_ = ProjectStatus::prepare;
        stopProject_();
        projectStatus_ = ProjectStatus::stopped;
    }
}

void ProjectManager::appendScheduledTask(const Jimmy::ScheduledTask& tt)
{
    scheduledTaskPool_.appendScheduledTask(tt);
}

void ProjectManager::removeScheduledTask(const Jimmy::ScheduledTask& tt)
{
    scheduledTaskPool_.removeScheduledTask(tt);
}

void ProjectManager::notifyComponentChange(Jimmy::User userid,const QString& cid,const QJsonValue& value)
{
    auto itor = subscriptionComponents_.find(cid);
    if (itor == subscriptionComponents_.end())
    {
        return;
    }

    if (!itor.value()->empty())
    {
        foreach(const auto& item,*(itor.value()))
        {
            ComponentChangeEvent componentChangeEvent;
            componentChangeEvent.userid = userid;
            componentChangeEvent.cid = item;
            componentChangeEvent.trigger = cid;
            componentChangeEvent.value = value;

            threadPool.notifyComponentChange(componentChangeEvent);
        }
    }
}

void ProjectManager::triggerComponentChangeEvent(const Jimmy::ComponentChangeEvent& componentChangeEvent)
{
    threadPool.notifyComponentChange(componentChangeEvent);
}

std::shared_ptr<Jimmy::CoreComponent> ProjectManager::getComponent(const QString& cid)
{
    auto itor = components_.find(cid);
    if (itor == components_.end())
    {
        return nullptr;
    }

    return itor.value();
}

Jimmy::ErrorCode ProjectManager::runProject_()
{
    generateSubscriptionComponents();
    generateBoardcastRespondComponents();

    QHash<QString, std::shared_ptr<Jimmy::CoreComponent>> teamMasters_;
    foreach (auto& item ,components_.values())
    {
        if(item->getType()==ComponentType::TeamMaster)
        {
            teamMasters_.insert(static_cast<TeamMasterComponent*>(item.get())->getTeam(),item);
        }
    }

    foreach (auto& item ,components_.values())
    {
        if(item->getType()==ComponentType::TeamSlave)
        {
            auto p = static_cast<TeamSlaveComponent*>(item.get());
            p->setMaster(teamMasters_[p->getTeam()]);
        }

        if (item->getType() == ComponentType::TeamMaster)
        {
            continue;
        }

        if(item->start()!=ErrorCode::ec_ok)
        {
            return ErrorCode::ec_error;
        }
    }

	foreach(auto& item, teamMasters_.values())
	{
		if (item->start() != ErrorCode::ec_ok)
		{
			return ErrorCode::ec_error;
		}
	}

    scheduledTaskPool_.registerScheduledEvent(std::bind(&ProjectManager::triggerComponentChangeEvent, this, placeholders::_1));
    scheduledTaskPool_.start();
    threadPool.start();

    return ErrorCode::ec_ok;
}

void ProjectManager::stopProject_()
{
    scheduledTaskPool_.stop();
    threadPool.stop();

    foreach (auto& item ,components_.values())
    {
        item->stop();
    }
}

QStringList ProjectManager::getIntersectBoardcast(Jimmy::User userid,const QStringList& boardcast)
{
    return boardcast_.getIntersect(userid, boardcast);
}

void ProjectManager::setLog(Jimmy::Connection connection, QJsonObject& jo)
{
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    if(userInfo->role !=UserRole::Administrator)
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "insufficient privileges");
    }
    else
    {
        auto elemItor = jo.find("log_level");
        if((elemItor == jo.end())||(!elemItor->isDouble()))
        {
            jo.insert(Result, Failed);
            jo.insert(Reason, "log_level is invalid");
        }
        else
        {
            jo.insert(Result, Succeed);
            GlobalLogger::get_instance()->setLoglevel(static_cast<LogLevel>(elemItor->toInt()));
        }
    }

    gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
}

void ProjectManager::setBoardcastCode(Jimmy::Connection connection, QJsonObject& jo)
{
    if (projectStatus_ != ProjectStatus::running)
    {
        return;
    }

    auto elemItor = jo.find("value");
    if((elemItor == jo.end())||(!elemItor->isString()))
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "value is invalid");
        gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
        return;
    }

    QString boardcastCode = elemItor->toString();

    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    boardcast_.appendBoardcast(userInfo->userId, boardcastCode);

    auto itor = boardcastRespondComponent_.find(boardcastCode);
    if (itor != boardcastRespondComponent_.end())
    {
        Q_FOREACH(auto componentName, *itor.value())
        {
            triggerComponentChangeEvent({userInfo->userId,componentName,CommonConst::Boardcast});
        }
    }

    jo.insert(Result, Succeed);
    gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
}

void ProjectManager::cancelBoardcastCode(Jimmy::Connection connection, QJsonObject& jo)
{
    if (projectStatus_ != ProjectStatus::running)
    {
        return;
    }

    auto elemItor = jo.find("value");
    if((elemItor == jo.end())||(!elemItor->isString()))
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "value is invalid");
        gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
        return;
    }

    QString boardcastCode = elemItor->toString();

    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    boardcast_.removeBoardcast(userInfo->userId,elemItor->toString());

    auto itor = boardcastRespondComponent_.find(boardcastCode);
    if (itor != boardcastRespondComponent_.end())
    {
        Q_FOREACH(auto componentName, *itor.value())
        {
            triggerComponentChangeEvent({userInfo->userId,componentName,CommonConst::Boardcast});
        }
    }

    jo.insert(Result, Succeed);
    gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
}

void ProjectManager::getBoardcastCode(Jimmy::Connection connection, QJsonObject& jo)
{
    if (projectStatus_ != ProjectStatus::running)
    {
        return;
    }

    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    jo.insert("value",QJsonArray::fromStringList(boardcast_.getBoardcast(userInfo->userId)));

    gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
}

void ProjectManager::actionFailed(Jimmy::Connection connection,const QString& action,const QString& reason)
{
    QJsonObject joRet;
    joRet.insert("action", action);
    joRet.insert(Result, Failed);
    joRet.insert(Reason, reason);
    gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(joRet).toJson(QJsonDocument::Compact));
}

void ProjectManager::loadProject(Jimmy::Connection connection, QJsonObject& jo)
{
    Q_UNUSED(jo)
    if (projectStatus_== ProjectStatus::invalid)
    {
        actionFailed(connection,"load_project","current project is invalid");
        return;
    }

    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    QJsonObject jRe;
    jRe.insert("action", "load_project");
    jRe.insert(Result, "succeed");
    jRe.insert("name", gActionSimulationServer.getAppConfig()->getProjectFile().baseName());
    jRe.insert("status", static_cast<int>(getStatus()));

    jRe.insert("components",json_components_);
    jRe.insert("category",json_category_);

    gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jRe).toJson(QJsonDocument::Compact));
}

void ProjectManager::runProject(Jimmy::Connection connection, QJsonObject& jo)
{
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    if(userInfo->role != UserRole::Administrator)
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "insufficient privileges");
        gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
        return;
    }

    switch (projectStatus_)
    {
    case ProjectStatus::invalid:
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "can't load any project");
        break;
    }
    case ProjectStatus::stopped:
    {
        auto ret = runProject_();
        if (ret != ErrorCode::ec_ok)
        {
            jo.insert(Result, Failed);
            jo.insert(Reason, "please check log file");
            break;
        }
    }
    default:
    {
        jo.insert(Result, Succeed);
        break;
    }
    }

    gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
}

void ProjectManager::stopProject(Jimmy::Connection connection, QJsonObject& jo)
{
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    if(userInfo->role != UserRole::Administrator)
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "insufficient privileges");
        gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson());
        return;
    }

    switch (projectStatus_)
    {
    case ProjectStatus::invalid:
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "can't load any project");
        break;
    }
    case ProjectStatus::running:
    {
        stopProject_();
        //fall through
    }
    default:
    {
        jo.insert(Result, Succeed);
        break;
    }
    }

    gActionSimulationServer.getUserManager()->answerMessage(connection,QJsonDocument(jo).toJson(QJsonDocument::Compact));
}

void ProjectManager::resetProject(Jimmy::Connection connection, QJsonObject& jo)
{
    Q_UNUSED(jo)
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    Q_FOREACH(auto elem, components_)
    {
        elem->removeUser(userInfo->userId);
    }
}

void ProjectManager::getProjectStatus(Jimmy::Connection connection, QJsonObject& jo)
{
    Q_UNUSED(jo)

    QJsonObject joRet;
    joRet.insert("action", "get_project_status");
    joRet.insert("name", gActionSimulationServer.getAppConfig()->getProjectFile().baseName());
    joRet.insert("value", ::getProjectStatus(getStatus()));
    gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(joRet).toJson(QJsonDocument::Compact));
}

void ProjectManager::setLogin(Jimmy::Connection connection, QJsonObject& jo)
{
    User user;

    if(projectType_ != ProjectType::SingleUser)
    {
        auto userItor = jo.find("userid");
        if((userItor == jo.end())||(!userItor->isDouble()))
        {
            jo.insert(Result, Failed);
            jo.insert(Reason, "userid is not exist");
            gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(jo).toJson(QJsonDocument::Compact));
            return;
        }

        user.userID = userItor->toInt();
        if(user.userID <= 0)
        {
            jo.insert(Result, Failed);
            jo.insert(Reason, "userid is invalid");
            gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(jo).toJson(QJsonDocument::Compact));
            return;
        }
    }

    auto role{0};
    auto roleItor = jo.find("role");
    if((roleItor != jo.end())&&(roleItor->isDouble()))
    {
        role = roleItor->toInt();
    }

    gActionSimulationServer.getUserManager()->login(connection,user,role);

    jo.insert(Result, Succeed);
    gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(jo).toJson(QJsonDocument::Compact));
}

void ProjectManager::queryAllValue(Jimmy::Connection connection, QJsonObject& jo)
{
    Q_UNUSED(jo)
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    foreach (auto& component ,components_.values())
    {
        gActionSimulationServer.getUserManager()->answerMessage(connection,component->getAnswerValue(component->getValue(userInfo->userId)));
    }
}

void ProjectManager::queryValue(Jimmy::Connection connection, QJsonObject& jo)
{
    auto cidItor = jo.find("cid");
    if ((cidItor == jo.end())||(!cidItor->isString()))
    {
        LOGERROR(QStringLiteral("[%1:%2] cid is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return;
    }

    shared_ptr<CoreComponent> component = getComponent(cidItor->toString());
    if (!component)
    {
        LOGERROR(QStringLiteral("[%1:%2] cid is not exist")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return;
    }

    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    gActionSimulationServer.getUserManager()->answerMessage(connection,component->getAnswerValue(component->getValue(userInfo->userId)));
}

void ProjectManager::componentStatusChange(Jimmy::Connection connection, QJsonObject& jo)
{
    if (projectStatus_ != ProjectStatus::running)
    {
        return;
    }

    auto cidItor = jo.find("cid");
    if ((cidItor == jo.end())||(!cidItor->isString()))
    {
        LOGERROR(QStringLiteral("[%1:%2] cid is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return;
    }

    auto valueItor = jo.find("value");
    if (valueItor == jo.end())
    {
        LOGERROR(QStringLiteral("[%1:%2] value is not exist")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return;
    }

    shared_ptr<CoreComponent> component = getComponent(cidItor->toString());
    if (!component)
    {
        LOGERROR(QStringLiteral("[%1:%2] component %s is not exist")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(cidItor->toString()));

        return;
    }

    component->setValue(connection,valueItor.value());
}

void ProjectManager::notify(Jimmy::Connection connection, QJsonObject& jo)
{
    auto userInfo = gActionSimulationServer.getUserManager()->getUserInfo(connection);
    if(!userInfo)
    {
        return;
    }

    auto data = QJsonDocument(jo).toJson();
    auto itor = jo.find("userid");
    if (itor != jo.end())
    {
        if(itor->isDouble())
        {
            gActionSimulationServer.getUserManager()->sendUserMessage(itor->toInt(), false,connection, data);
        }
        else if(itor->isArray())
        {
            auto users = itor->toArray();
            for(auto iter = users.cbegin();iter!=users.cend();++iter)
            {
                gActionSimulationServer.getUserManager()->sendUserMessage(iter->toInt(),false,connection, data);
            }
        }

        return;
    }

    itor = jo.find("role");
    if (itor != jo.end())
    {
        if(itor->isDouble())
        {
            gActionSimulationServer.getUserManager()->sendRoleMessage(itor->toInt(),connection, data);
        }
        else if(itor->isArray())
        {
            auto roles = itor->toArray();
            for(auto iter=roles.cbegin();iter!=roles.cend();++iter)
            {
                gActionSimulationServer.getUserManager()->sendRoleMessage(iter->toInt(),connection, data);
            }
        }
        return;
    }

    gActionSimulationServer.getUserManager()->sendMessage(false,connection,data);
}

void ProjectManager::reloadScript(Jimmy::Connection connection, QJsonObject& jo)
{
    if (projectStatus_ != ProjectStatus::running)
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "project is not running");
        gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(jo).toJson(QJsonDocument::Compact));
        return;
    }

    auto roleItor = jo.find("role");
    if ((roleItor == jo.end())||(!roleItor->isString()))
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "role is invalid");
        gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(jo).toJson(QJsonDocument::Compact));
        return;
    }

    bool bSucceed{true};
    foreach (auto& component ,components_.values())
    {
        if(component->reloadRole(roleItor->toString())!=ErrorCode::ec_ok)
        {
            bSucceed = false;
        }
    }

    if(bSucceed)
    {
        jo.insert(Result, Succeed);
    }
    else
    {
        jo.insert(Result, Failed);
        jo.insert(Reason, "load script failed");
    }
    gActionSimulationServer.getUserManager()->answerMessage(connection, QJsonDocument(jo).toJson(QJsonDocument::Compact));
    return;
}

void ProjectManager::generateSubscriptionComponents()
{
    subscriptionComponents_.clear();

    foreach (auto& item,components_.values())
    {
        foreach (auto& cid, item->getSubscription())
        {
            auto itor = subscriptionComponents_.find(cid);
            if (itor == subscriptionComponents_.end())
            {
                subscriptionComponents_.insert(cid, make_shared<QStringList>(item->getID()));
            }
            else
            {
                subscriptionComponents_[cid]->push_back(item->getID());
            }
        }
    }
}

void ProjectManager::generateBoardcastRespondComponents()
{
    boardcastRespondComponent_.clear();

    foreach (auto& item,components_.values())
    {
        foreach (auto& code, item->getRespondBoardcast())
        {
            auto itor = boardcastRespondComponent_.find(code);
            if(itor == boardcastRespondComponent_.end())
            {
                boardcastRespondComponent_.insert(code, make_shared<QStringList>(item->getID()));
            }
            else
            {
                boardcastRespondComponent_[code]->push_back(item->getID());
            }
        }
    }
}

void ProjectManager::removeUser(Jimmy::User userid)
{
    if (projectStatus_ != ProjectStatus::running)
    {
        return;
    }

    Q_FOREACH(auto elem, components_)
    {
        elem->removeUser(userid);
    }
}

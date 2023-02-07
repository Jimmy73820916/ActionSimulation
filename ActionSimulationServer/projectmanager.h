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
#include <string>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include "corecomponent.h"
#include <variant>
#include <QQueue>
#include <QPair>
#include <mutex>
#include "boardcast.h"
#include "corecomponent.h"
#include "scheduledtaskpool.h"
#include "threadpool.h"


enum class ProjectStatus
{
    invalid,
    prepare,
    running,
    stopped,
};

inline const char* getProjectStatus(ProjectStatus projectStatus)
{
    switch(projectStatus)
    {
    case ProjectStatus::invalid: return "invalid";
    case ProjectStatus::running: return "running";
    case ProjectStatus::stopped: return "stopped";
    default: return "";
    }
}

enum class ProjectType
{
    SingleUser,                 //单用户
    MultiUser,                  //多用户
};

class ProjectManager
{
public:
    ProjectManager();
    ~ProjectManager();

    ProjectStatus getStatus() const { return projectStatus_; }
    ProjectType getProjectType() { return projectType_; }

    void pushMessage(size_t connection_id,const std::string& message);

    void run();
    void stop();

    uint32_t getMinTimerInternal() { return min_timer_interval_; }
    uint32_t getDefaultTimerInternal() { return default_timer_interval_; }

    void appendScheduledTask(const Jimmy::ScheduledTask& tt);
    void removeScheduledTask(const Jimmy::ScheduledTask& tt);

    void removeUser(Jimmy::User userid);

    void notifyComponentChange(Jimmy::User userid,const QString& cid,const QJsonValue& value);

    void triggerComponentChangeEvent(const Jimmy::ComponentChangeEvent& componentChangeEvent);

    std::shared_ptr<Jimmy::CoreComponent> getComponent(const QString& cid);

    QStringList getIntersectBoardcast(Jimmy::User userid,const QStringList& boardcast);
private:
    void setLog(Jimmy::Connection connection, QJsonObject& jo);
    void setBoardcastCode(Jimmy::Connection connection, QJsonObject& jo);
    void cancelBoardcastCode(Jimmy::Connection connection, QJsonObject& jo);
    void getBoardcastCode(Jimmy::Connection connection, QJsonObject& jo);

    void loadProject(Jimmy::Connection connection, QJsonObject& jo);

    void runProject(Jimmy::Connection connection, QJsonObject& jo);
    void stopProject(Jimmy::Connection connection, QJsonObject& jo);
    void resetProject(Jimmy::Connection connection, QJsonObject& jo);
    void getProjectStatus(Jimmy::Connection connection, QJsonObject& jo);

    void setLogin(Jimmy::Connection connection, QJsonObject& jo);

    void queryAllValue(Jimmy::Connection connection, QJsonObject& jo);
    void queryValue(Jimmy::Connection connection, QJsonObject& jo);
    void componentStatusChange(Jimmy::Connection connection, QJsonObject& jo);

    void notify(Jimmy::Connection connection, QJsonObject& jo);
    void reloadScript(Jimmy::Connection connection, QJsonObject& jo);
private:
    static const char* const Action;
    static const char* const Succeed;
    static const char* const Failed;
    static const char* const Result;
    static const char* const Reason;
private:
    void loadProject_();
    Jimmy::ErrorCode runProject_();
    void stopProject_();

    bool loadGeneral(const QJsonObject& jo);
    bool loadComponent();

    void generateSubscriptionComponents();
    void generateBoardcastRespondComponents();

    std::shared_ptr<Jimmy::CoreComponent> createComponent(Jimmy::ComponentType type);

    QHash<QString, std::shared_ptr<Jimmy::CoreComponent>> components_;
    QHash<QString, std::shared_ptr<QStringList>> subscriptionComponents_;
    QHash<QString, std::shared_ptr<QStringList>> boardcastRespondComponent_;
    Boardcast boardcast_;

    QJsonObject     json_components_;
    QJsonObject     json_category_;

    Jimmy::ScheduledTaskPool scheduledTaskPool_;
    Jimmy::ThreadPool threadPool;
private:
    void actionFailed(Jimmy::Connection connection,const QString& action,const QString& reason);

    void disposeCommand(Jimmy::Connection connection, const QString& message);

    void commandTcpDataThread();
    std::thread commandTCPDataThread_;

    void initializesDispatcher();
    QHash<QString, std::function<void(Jimmy::Connection,QJsonObject&)>> commandDispatcher_;

    bool isRun_;

    QQueue<QPair<Jimmy::Connection,QString>> msgData_;
    std::mutex lockMsgData_;
    std::condition_variable cvMsgData_;
private:
    uint32_t min_timer_interval_;
    uint32_t default_timer_interval_;
    ProjectStatus projectStatus_;
    ProjectType projectType_;
};


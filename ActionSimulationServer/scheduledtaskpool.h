#pragma once
#include <mutex>
#include <QList>
#include <QPair>
#include "commonstruct.h"

namespace Jimmy
{

class ScheduledTaskPool
{
public:
    ScheduledTaskPool();
    ~ScheduledTaskPool();

    void start();
    void stop();

    void appendScheduledTask(const Jimmy::ScheduledTask& tt);
    void removeScheduledTask(const Jimmy::ScheduledTask& tt);

    //计划时间到执行函数
    void registerScheduledEvent(std::function<void(Jimmy::ComponentChangeEvent&&)> scheduledEvent);
private:
    bool is_run_;

    std::condition_variable evScheduledTask_;
    std::mutex	lockscheduledTask_;
    std::thread scheduledTaskThread_;

    std::function<void(Jimmy::ComponentChangeEvent&&)> scheduledEvent_;

    // true->append; false->remove
    QList<QPair<bool,Jimmy::ScheduledTask>> changeTaskList_;

    void scheduledTask();
};

}




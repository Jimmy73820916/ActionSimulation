#include "scheduledtaskpool.h"
#include "commonconst.h"

using namespace std;

namespace Jimmy
{

ScheduledTaskPool::ScheduledTaskPool()
    :is_run_(false)
{

}

ScheduledTaskPool::~ScheduledTaskPool()
{
    stop();
}

void ScheduledTaskPool::start()
{
    is_run_ = true;
    scheduledTaskThread_ = std::thread(std::bind(&ScheduledTaskPool::scheduledTask, this));
}

void ScheduledTaskPool::stop()
{
    is_run_ = false;
    if (scheduledTaskThread_.joinable())
    {
        evScheduledTask_.notify_one();
        scheduledTaskThread_.join();
    }
}

void ScheduledTaskPool::appendScheduledTask(const ScheduledTask& tt)
{
    if(is_run_)
    {
        {
            lock_guard<mutex> lg(lockscheduledTask_);
            changeTaskList_.push_back({true,tt});
        }

        evScheduledTask_.notify_one();
    }
}

void ScheduledTaskPool::removeScheduledTask(const ScheduledTask& tt)
{
    if(is_run_)
    {
        {
            lock_guard<mutex> lg(lockscheduledTask_);
            changeTaskList_.push_back({false,tt});
        }

        evScheduledTask_.notify_one();
    }
}

void ScheduledTaskPool::registerScheduledEvent(std::function<void(Jimmy::ComponentChangeEvent&&)> scheduledEvent)
{
    scheduledEvent_ = scheduledEvent;
}

void ScheduledTaskPool::scheduledTask()
{
    const size_t timeInterval = 86400;
    chrono::steady_clock::time_point tp = chrono::steady_clock::now() + chrono::seconds(timeInterval);

    QList<QPair<bool,Jimmy::ScheduledTask>> scheduledTaskSwap;
    QHash<QString, ScheduledTask> scheduledTaskList;

    while (is_run_)
    {
        {
            unique_lock<mutex> lg(lockscheduledTask_);
            evScheduledTask_.wait_until(lg, tp, [this] {return (!is_run_) || (!changeTaskList_.empty()); });
            if (!is_run_)
            {
                return;
            }

            if (!changeTaskList_.empty())
            {
                scheduledTaskSwap.swap(changeTaskList_);
            }
        }

        while(!scheduledTaskSwap.empty())
        {
            auto st = scheduledTaskSwap.takeFirst();
            if(st.first) //append
            {
                QString key = QStringLiteral("%1-%2").arg(st.second.userid.userID).arg(st.second.cid);
                scheduledTaskList[key] = st.second;
            }
            else //remove
            {
                if(st.second.cid == "")
                {
                    auto itor = scheduledTaskList.begin();
                    while (itor != scheduledTaskList.end())
                    {
                        QString key = QStringLiteral("%1-").arg(st.second.userid.userID);
                        if(itor.key().startsWith(key))
                        {
                            itor = scheduledTaskList.erase(itor);
                            continue;
                        }

                        ++itor;
                    }
                }
                else
                {
                    QString key = QStringLiteral("%1-%2").arg(st.second.userid.userID).arg(st.second.cid);
                    scheduledTaskList.remove(key);
                }
            }
        }

        tp = chrono::steady_clock::now() + chrono::seconds(timeInterval);

        auto itor = scheduledTaskList.begin();
        while (itor != scheduledTaskList.end())
        {
            if (scheduledEvent_ && (itor->next_tp <= chrono::steady_clock::now()))
            {
                itor->counter++;

                if (itor->loopValue.empty())
                {
                    scheduledEvent_({itor->userid,itor->cid,CommonConst::TimerEvent,QJsonValue(),itor->counter});
                }
                else
                {
                    if(itor->scheduledType == ScheduledType::Loop)
                    {
                        scheduledEvent_({itor->userid,itor->cid,CommonConst::LoopEvent,itor->loopValue[itor->counter % itor->loopValue.size()],itor->counter});
                    }
                    else
                    {
                        auto& loopValue = itor->loopValue;
                        loopValue[0] = loopValue[0].toDouble() + loopValue[2].toDouble();
                        if(((loopValue[2].toDouble() > 0) && (loopValue[0].toDouble() <= loopValue[1].toDouble())) ||
                            ((loopValue[2].toDouble() < 0) && (loopValue[0].toDouble() >= loopValue[1].toDouble())))
                        {
                            scheduledEvent_({itor->userid,itor->cid,CommonConst::OrderEvent,loopValue[0],itor->counter});
                        }
                        else
                        {
                            itor = scheduledTaskList.erase(itor);
                            continue;
                        }
                    }
                }

                itor->next_tp = chrono::steady_clock::now() + chrono::milliseconds(itor->interval);
                if (itor->times > 0)
                {
                    --itor->times;
                    if (itor->times == 0)
                    {
                        itor = scheduledTaskList.erase(itor);
                        continue;
                    }
                }
            }

            if (itor->next_tp < tp)
            {
                tp = itor->next_tp;
            }

            ++itor;
        }
    }
}


}


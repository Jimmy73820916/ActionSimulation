#include "threadpool.h"
#include "corecomponent.h"
#include "actionsimulationserver.h"
#include "projectmanager.h"
#include "logger.h"

using namespace std;

namespace Jimmy
{

ThreadPool::ThreadPool()
    :is_run_(false)
{

}

ThreadPool::~ThreadPool()
{
    stop();

    invokeChainThread_.clear();
}

void ThreadPool::start()
{
    if(!is_run_)
    {
        is_run_ = true;
        for (size_t i = 0; i < thread::hardware_concurrency(); ++i)
        {
            invokeChainThread_.push_back(std::thread(std::bind(&ThreadPool::invokeChain, this)));
        }
    }
}

void ThreadPool::stop()
{
    if(is_run_)
    {
        is_run_ = false;

        for (size_t i = 0; i < thread::hardware_concurrency(); ++i)
        {
            if (invokeChainThread_[i].joinable())
            {
                evInvokeChain_.notify_all();
                invokeChainThread_[i].join();
            }
        }

        invokeChainThread_.clear();
    }
}

void ThreadPool::notifyComponentChange(const Jimmy::ComponentChangeEvent& componentChangeEvent)
{
    {
        lock_guard<mutex> lg(lockInvokeChain_);
        waitingInvokeList_.push_back(componentChangeEvent);
    }

    evInvokeChain_.notify_all();
}

void ThreadPool::invokeChain()
{
    ComponentChangeEvent componentChangeEvent;
    while (is_run_)
    {
        {
            unique_lock<mutex> lg(lockInvokeChain_);

            evInvokeChain_.wait(lg, [this] {return (!is_run_) || (!waitingInvokeList_.empty()); });
            if (!is_run_)break;
            if (waitingInvokeList_.empty())
            {
                continue;
            }

            componentChangeEvent = waitingInvokeList_.front();
            waitingInvokeList_.pop_front();
        }

        shared_ptr<CoreComponent> component = gActionSimulationServer.getProjectManager()->getComponent(componentChangeEvent.cid);
        if (!component)
        {
            LOGFATAL(QStringLiteral("[%1:%2] %3 is not exist")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(componentChangeEvent.cid));

            return;
        }

        if (componentChangeEvent.trigger.compare(CommonConst::TimerEvent)==0)
        {
            component->onTime(componentChangeEvent.userid,componentChangeEvent.counter);
        }
        else if ((componentChangeEvent.trigger.compare(CommonConst::LoopEvent)==0)
             ||(componentChangeEvent.trigger.compare(CommonConst::OrderEvent)==0))
        {
            component->onLoop(componentChangeEvent.userid, componentChangeEvent.value);
        }
        else if (componentChangeEvent.trigger.compare(CommonConst::Boardcast) == 0)
        {
            component->onBoardcast(componentChangeEvent.userid);
        }
        else
        {
            component->onAction(componentChangeEvent.userid,componentChangeEvent.trigger, componentChangeEvent.value);
        }
    }
}


}

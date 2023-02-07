#pragma once

#include <QJsonValue>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "commonstruct.h"

namespace Jimmy
{

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    void start();
    void stop();

    void notifyComponentChange(const Jimmy::ComponentChangeEvent& componentChangeEvent);
private:
    bool is_run_;

    std::condition_variable evInvokeChain_;
    std::mutex	lockInvokeChain_;
    QList<ComponentChangeEvent> waitingInvokeList_;

    std::vector<std::thread> invokeChainThread_;

    void invokeChain();
};

}


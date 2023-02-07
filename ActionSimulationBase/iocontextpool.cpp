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

#include "iocontextpool.h"
#include <stdexcept>
#include <boost/bind/bind.hpp>

using namespace std;

namespace Jimmy {

IoContextPool::IoContextPool(bool is_single)
    : nextIoContext_(0)
    , status_(status::stopped)
    , isSingle_(is_single)
{
    size_t num = isSingle_ ? 1 : thread::hardware_concurrency();
    for (auto i = 0; i < num; ++i)
    {
        io_context_ptr io_context(new boost::asio::io_context);
        ioContexts_.push_back(io_context);
        work_.push_back(boost::asio::make_work_guard(*io_context));
    }
}

IoContextPool::~IoContextPool()
{
    stop();

    work_.clear();
    ioContexts_.clear();
}

void IoContextPool::run()
{
    if (status_ == status::stopped)
    {
        status_ = status::running;
        for (int i = 0; i < ioContexts_.size(); ++i)
        {
            shared_ptr<thread> athread(new thread(boost::bind(&boost::asio::io_context::run, ioContexts_[i])));
            threads_.push_back(athread);
        }
    }
    else if (status_ == status::pause)
    {
        for (int i = 0; i < ioContexts_.size(); ++i)
        {
            ioContexts_[i]->run();
        }
    }
}

void IoContextPool::pause()
{
    if (status_ == status::running)
    {
        status_ = status::pause;
        for (int i = 0; i < ioContexts_.size(); ++i)
        {
            ioContexts_[i]->stop();
        }
    }
}

void IoContextPool::stop()
{
    if (status_ != status::stopped)
    {
        status_ = status::stopped;
        for (int i = 0; i < ioContexts_.size(); ++i)
        {
            ioContexts_[i]->stop();
        }

        for (int i = 0; i < threads_.size(); ++i)
        {
            if (threads_[i]->joinable())
            {
                threads_[i]->join();
            }
        }

        threads_.clear();
    }
}

boost::asio::io_context& IoContextPool::getIoContext()
{
    if (isSingle_)
    {
        return *ioContexts_[0];
    }

    boost::asio::io_context& io_context = *ioContexts_[nextIoContext_];
    ++nextIoContext_;
    if (nextIoContext_ == ioContexts_.size())
    {
        nextIoContext_ = 0;
    }

    return io_context;
}

}

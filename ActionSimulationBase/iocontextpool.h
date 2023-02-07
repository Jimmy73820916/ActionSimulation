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

#include <boost/asio.hpp>
#include <QVector>
#include <QList>

namespace Jimmy
{
class IoContextPool
{
    Q_DISABLE_COPY(IoContextPool)
public:
    IoContextPool(bool is_single = true);
    ~IoContextPool();
    void run();
    void pause();
    void stop();
    boost::asio::io_context& getIoContext();

private:
    using io_context_ptr = std::shared_ptr<boost::asio::io_context>;
    using io_context_work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    QVector<std::shared_ptr<std::thread>> threads_;
    QVector<io_context_ptr> ioContexts_;
    QList<io_context_work> work_;

    std::size_t nextIoContext_;

    enum class status
    {
        stopped,
        running,
        pause,
    };

    status status_;
    bool isSingle_;
};

}

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
#include <atomic>
#include <mutex>
#include <QHash>
#include <QVector>
#include <QSet>
#include "iocontextpool.h"
#include "commonstruct.h"

namespace Jimmy
{
class TcpConnection;
class DataPackage;

class TcpServer
{
    Q_DISABLE_COPY(TcpServer)
    friend class TcpConnection;
public:
    TcpServer();
    ~TcpServer();

    bool start(const boost::asio::ip::tcp::endpoint& endpoint);
    void stop();

    void registerAppendConnnection(std::function<Jimmy::User(size_t)> connection);
    void registerRemoveConnnection(std::function<void(size_t)> connection);

    void registerMessageProcessFunction(std::function<void(size_t,const std::string&)> messageProcess);

    void sendData(size_t connectionId, const QByteArray& data);

    
    void removeConnection(size_t connectionId);

private:
    void appendConnection(std::shared_ptr<TcpConnection> pCon);

    int setOption();
    void startAccept();
    void handleAccept(const boost::system::error_code& ec);
private:
    static const size_t MaxConcurrencyConnectionCount = 10000;			//最大并发连接数
    static const size_t MaxConnectionTime = 10;

    std::atomic_bool isRun_;

    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;

    boost::asio::ip::tcp::endpoint endPoints_;

    std::shared_ptr<IoContextPool>  ioContextPool_;

    std::shared_ptr<TcpConnection> newConnection_;

    QHash<size_t, std::shared_ptr<TcpConnection>> connections_;
    std::mutex lockConnections_;

    std::shared_ptr<DataPackage> dataPackage_;

    std::function<Jimmy::User(size_t)> appendConnect_;
    std::function<void(size_t)> removeConnect_;


};

}

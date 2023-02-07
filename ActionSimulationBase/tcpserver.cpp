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

#include "tcpserver.h"
#include "tcpconnection.h"
#include "logger.h"
#include <boost/asio/socket_base.hpp>
#include "datapackage.h"
#include "commonfunction.h"

using namespace std;

namespace Jimmy
{

TcpServer::TcpServer()
    :isRun_(false)
{
    dataPackage_ = make_shared<DataPackage>();
    ioContextPool_ = make_shared<IoContextPool>();
}

TcpServer::~TcpServer()
{
    isRun_ = false;
    acceptor_->close();
    ioContextPool_->stop();
    dataPackage_.reset();
}

void TcpServer::registerAppendConnnection(std::function<Jimmy::User(size_t)> connection)
{
    appendConnect_ = connection;
}

void TcpServer::registerRemoveConnnection(std::function<void(size_t)> connection)
{
    removeConnect_ = connection;
}

void TcpServer::registerMessageProcessFunction(std::function<void(size_t, const std::string&)> messageProcess)
{
    dataPackage_->registerMessageProcessFunction(messageProcess);
}

bool TcpServer::start(const boost::asio::ip::tcp::endpoint& endpoint)
{
    if (!isRun_)
    {
        endPoints_ = endpoint;

        acceptor_ = std::make_shared<boost::asio::ip::tcp::acceptor>(ioContextPool_->getIoContext());
        acceptor_->open(endPoints_.protocol());
        if (setOption() != ec_ok)
        {
            return isRun_;
        }

        boost::system::error_code ec;
        acceptor_->bind(endPoints_, ec);
        if (ec.failed())
        {
            LOGERROR(QStringLiteral("[%1:%2] bind endpoint error,value = %3, message=%4")
                     .arg(__FUNCTION__)
                     .arg(__LINE__)
                     .arg(ec.value())
                     .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));
            return isRun_;
        }

        acceptor_->listen(MaxConcurrencyConnectionCount, ec);
        if (ec.failed())
        {
            LOGERROR(QStringLiteral("[%1:%2] bind endpoint error,value = %3, message=%4")
                     .arg(__FUNCTION__)
                     .arg(__LINE__)
                     .arg(ec.value())
                     .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));
            return isRun_;
        }
        else
        {
            LOGINFO("TcpServer Start listening");
        }

        startAccept();
        isRun_ = true;
    }
    ioContextPool_->run();
    return isRun_;
}

void TcpServer::stop()
{
    ioContextPool_->pause();

    lock_guard<mutex> lg(lockConnections_);
    foreach (auto item,connections_.values())
    {
        item->disconnect();
    }
    connections_.clear();
}

void TcpServer::startAccept()
{
    newConnection_.reset(new TcpConnection(this, ioContextPool_->getIoContext()));
    acceptor_->async_accept(newConnection_->socket(), bind(&TcpServer::handleAccept, this, std::placeholders::_1));
}

void TcpServer::handleAccept(const boost::system::error_code& ec)
{
    if (ec)
    {
        LOGERROR(QStringLiteral("[%1:%2] bind endpoint error,value = %3, message=%4")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(ec.value())
                 .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));
        return;
    }

    LOGINFO(QStringLiteral("[%1:%2] connection %3 request from remote ip [%4] was Accepted")
             .arg(__FUNCTION__)
             .arg(__LINE__)
             .arg(newConnection_->connectionID())
             .arg(newConnection_->getClientInfo()));

    appendConnection(newConnection_);
    newConnection_->start();
    startAccept();
}

void TcpServer::appendConnection(std::shared_ptr<TcpConnection> pCon)
{
    lock_guard<mutex> lg(lockConnections_);
    connections_.insert(pCon->connectionID(), pCon);
    if (appendConnect_)
    {
        appendConnect_(pCon->connectionID());
    } 
}


void TcpServer::removeConnection(size_t connectionId)
{
    {
        lock_guard<mutex> lg(lockConnections_);
        auto conn = connections_.find(connectionId);
        if (conn != connections_.end())
        {
            connections_.remove(connectionId);
            if (removeConnect_)
            {
                removeConnect_(connectionId);
            }
        }
    }

    dataPackage_->pushData(connectionId, nullptr, 0);
}

void TcpServer::sendData(size_t connectionId, const QByteArray& data)
{
    lock_guard<mutex> lg(lockConnections_);
    auto pT = connections_.find(connectionId);
    if (pT != connections_.end())
    {
        pT.value()->writeData(data);
    }
}

int TcpServer::setOption()
{
    boost::system::error_code ec;
    acceptor_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    if (ec.failed())
    {
        LOGERROR(QStringLiteral("[%1:%2] set reuse_address error,value = %3, message=%4")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(ec.value())
                 .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));

        return ErrorCode::ec_error;
    }

    acceptor_->set_option(boost::asio::ip::tcp::no_delay(true), ec);
    if (ec.failed())
    {
        LOGERROR(QStringLiteral("[%1:%2] set no_delay error,value = %3, message=%4")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(ec.value())
                 .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));

        return ErrorCode::ec_error;
    }

    acceptor_->set_option(boost::asio::socket_base::keep_alive(true), ec);
    if (ec.failed())
    {
        LOGERROR(QStringLiteral("[%1:%2] set keep_alive error,value = %3, message=%4")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(ec.value())
                 .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));

        return ErrorCode::ec_error;
    }

    acceptor_->non_blocking(true, ec);
    if (ec.failed())
    {
        LOGERROR(QStringLiteral("[%1:%2] set non_blocking error,value = %3, message=%4")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(ec.value())
                 .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));

        return ErrorCode::ec_error;
    }

    boost::asio::socket_base::receive_buffer_size receive_buffer(CommonConst::TCPBufferLength);
    acceptor_->set_option(receive_buffer, ec);
    if (ec.failed())
    {
        LOGERROR(QStringLiteral("[%1:%2] set receive buffer error,value = %3, message=%4")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(ec.value())
                 .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));

        return ErrorCode::ec_error;
    }

    boost::asio::socket_base::send_buffer_size send_buffer(CommonConst::TCPBufferLength);
    acceptor_->set_option(send_buffer, ec);
    if (ec.failed())
    {
        LOGERROR(QStringLiteral("[%1:%2] set send buffer error,value = %3, message=%4")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(ec.value())
                 .arg(CommonFunction::GBKtoUTF8(ec.message()).c_str()));

        return ErrorCode::ec_error;
    }

    return ErrorCode::ec_ok;
}

}


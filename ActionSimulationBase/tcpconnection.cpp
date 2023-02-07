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

#include "tcpconnection.h"
#include "tcpserver.h"
#include "logger.h"
#include "datapackage.h"
#include "commonfunction.h"

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace Jimmy
{
static uint64_t connection_index = 0;

TcpConnection::TcpConnection(TcpServer* pserver, boost::asio::io_context& ic)
    :server_(pserver)
    , connectionID_(++connection_index)
{
    connection_ = std::make_shared<boost::asio::ip::tcp::socket>(ic);

    memset(read_buffer_, 0, CommonConst::TCPBufferLength);
}

TcpConnection::~TcpConnection()
{
    disconnect();
}

size_t TcpConnection::connectionID()
{
    return connectionID_;
}

void TcpConnection::disconnect()
{
    LOGINFO(QStringLiteral("[%1:%2][%3]connectionId[%4] disconnect. ")
             .arg(__FUNCTION__)
             .arg(__LINE__)
             .arg(clientInfo_)
             .arg(connectionID_));

    server_->removeConnection(connectionID_);

    boost::system::error_code ec;
    connection_->shutdown(socket_base::shutdown_both, ec);
    connection_->close(ec);
}

void TcpConnection::start()
{
	clientInfo_ = QStringLiteral("%1:%2")
		.arg(connection_->remote_endpoint().address().to_string().c_str())
		.arg(connection_->remote_endpoint().port());

    readData();
}

void TcpConnection::readData()
{
    if (!connection_->is_open())
    {
        LOGERROR(QStringLiteral("[%1:%2][%3]connectionId[%4] is not open.")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(clientInfo_)
                 .arg(connectionID_));
        return;
    }

    connection_->async_read_some(asio::buffer(read_buffer_, CommonConst::TCPBufferLength),
        bind(&TcpConnection::handleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void TcpConnection::handleRead(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (ec)
    {
        if (ec != boost::asio::error::eof)
        {
            LOGERROR(QStringLiteral("[%1:%2][%3]connectionId[%4] read error.error code = %5 message = %6")
                     .arg(__FUNCTION__)
                     .arg(__LINE__)
                     .arg(clientInfo_)
                     .arg(connectionID_)
                     .arg(ec.value())
                     .arg(QString::fromLocal8Bit(CommonFunction::GBKtoUTF8(ec.message()).c_str())));

            disconnect();
        }

        server_->removeConnection(connectionID_);
        return;
    }

    if (bytes_transferred > 0)
    {
        server_->dataPackage_->pushData(connectionID_, &read_buffer_[0], bytes_transferred);

        QString readData = QString::fromLocal8Bit((const char*)read_buffer_, bytes_transferred);

        LOGINFO(QStringLiteral("[%1:%2][%3]connectionId[%4] received (%5) message [%6]")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(clientInfo_)
                 .arg(connectionID_)
                 .arg(bytes_transferred)
                 .arg(readData));

        connection_->async_read_some(asio::buffer(read_buffer_, CommonConst::TCPBufferLength),
            bind(&TcpConnection::handleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
}

void TcpConnection::writeData(const QByteArray& data)
{
    if (!connection_->is_open())
    {
        LOGERROR(QStringLiteral("[%1:%2][%3]connectionId[%4] is not open.")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(clientInfo_)
                 .arg(connectionID_));
        return;
    }

    if (!data.isEmpty())
    {
        lock_guard<mutex> lg(lockWriteBuffer_);
        size_t write_queqe_length = writeBuffer_.size();
        writeBuffer_.enqueue(data);

        if (write_queqe_length == 0)
        {
            connection_->async_write_some(asio::buffer(writeBuffer_.front().data(), writeBuffer_.front().size()),
                bind(&TcpConnection::handleWrite, shared_from_this(), std::placeholders::_1));
        }
        else
        {
            if (write_queqe_length > WriteCacheMaxLength)
            {
                LOGWARN(QStringLiteral("[%1:%2][%3]connectionId[%4] has %5 message waiting to send !")
                    .arg(__FUNCTION__)
                    .arg(__LINE__)
                    .arg(clientInfo_)
                    .arg(connectionID_)
                    .arg(write_queqe_length));
            }
        }
    }
}

void TcpConnection::handleWrite(const boost::system::error_code& ec)
{
    if (ec)
    {
        LOGERROR(QStringLiteral("[%1:%2] write [%3]connectionId[%4] error code = %5 message = %6")
                 .arg(__FUNCTION__)
                 .arg(__LINE__)
                 .arg(clientInfo_)
                 .arg(connectionID_)
                 .arg(ec.value())
                 .arg(QString::fromLocal8Bit(CommonFunction::GBKtoUTF8(ec.message()).c_str())));

        disconnect();
        return;
    }

    lock_guard<mutex> lg(lockWriteBuffer_);

    QString writeData = QString::fromLocal8Bit(writeBuffer_.dequeue());

    LOGINFO(QStringLiteral("[%1:%2] write [%3]connectionId[%4] message = %5")
             .arg(__FUNCTION__)
             .arg(__LINE__)
             .arg(clientInfo_)
             .arg(connectionID_)
             .arg(writeData));


    if (!writeBuffer_.empty())
    {
        auto& data = writeBuffer_.head();
        connection_->async_write_some(asio::buffer(data, data.size()),
            bind(&TcpConnection::handleWrite, shared_from_this(), std::placeholders::_1));
    }
}

boost::asio::ip::tcp::socket& TcpConnection::socket()
{
    return *connection_;
}

QString TcpConnection::getClientInfo()
{
    return clientInfo_;
}

}

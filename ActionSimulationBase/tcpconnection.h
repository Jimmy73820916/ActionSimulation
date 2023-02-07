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
#include <QString>
#include <QByteArray>
#include <QQueue>
#include "commonconst.h"

namespace Jimmy
{

class TcpServer;

class TcpConnection final : public std::enable_shared_from_this<TcpConnection>
{
    Q_DISABLE_COPY(TcpConnection)
public:
    TcpConnection(TcpServer* pserver, boost::asio::io_context& ic);
    ~TcpConnection();

    size_t connectionID();
    void disconnect();

    void start();

    void readData();
    void writeData(const QByteArray& data);

    boost::asio::ip::tcp::socket& socket();
    QString getClientInfo();
private:
    void handleRead(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void handleWrite(const boost::system::error_code& ec);
private:
    TcpServer* server_;
    const size_t connectionID_;

    std::shared_ptr<boost::asio::ip::tcp::socket> connection_;

    uint8_t read_buffer_[CommonConst::TCPBufferLength];

    static const size_t WriteCacheMaxLength = 64;
    QQueue<QByteArray> writeBuffer_;
    std::mutex lockWriteBuffer_;

    QString clientInfo_;
};

};

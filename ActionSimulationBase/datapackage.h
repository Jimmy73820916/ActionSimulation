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

#include <thread>
#include <mutex>
#include <QQueue>
#include <memory>
#include <functional>
#include "databuffer.h"

namespace Jimmy
{

struct MessageData
{
	size_t		connection_id;
	size_t		DataLength;
	uint8_t		Data[1];
};

class TcpServer;

class DataPackage
{
public:
    DataPackage();
    ~DataPackage();

    //组包成功调用函数
    void registerMessageProcessFunction(std::function<void(size_t, const std::string&)> messageProcess);

    //黑名单调用函数
    void registerblacklistFunction(std::function<void(size_t)> blackConnect);

    void pushData(size_t connectionId, const uint8_t* pData, size_t dataLength);
private:
    bool AnalyzeCommand(std::shared_ptr<Jimmy::DataBuffer> dest, MessageData* pRD);
    void datagramOpThread();
    std::thread	datagramOpThread_;

    QQueue<MessageData*> datagramData_;
    std::mutex lockDatagramData_;
    std::condition_variable eventDatagramData_;

    bool isRun_;

    std::function<void(size_t, const std::string&)> messageProcess_;
    std::function<void(size_t)> blacklistFunc_;
};

};

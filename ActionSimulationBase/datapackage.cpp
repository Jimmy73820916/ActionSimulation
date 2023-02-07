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

#include "datapackage.h"
#include "commonfunction.h"

using namespace std;

namespace Jimmy
{

MessageData* constructMsgPacket(size_t connection_id, const uint8_t* pData, size_t data_length)
{
	size_t size = sizeof(MessageData) + data_length;

	MessageData* pMsgData = static_cast<MessageData*>(malloc(size));
	pMsgData->connection_id = connection_id;
	pMsgData->DataLength = data_length;
	if (data_length > 0)
	{
		memcpy(pMsgData->Data, pData, data_length);
	}

	return pMsgData;
}

void destructMsgPacket(MessageData* p)
{
	if (p)
	{
		free((void*)p);
		p = nullptr;
	}
}

DataPackage::DataPackage()
	:isRun_(true)
{
	datagramOpThread_ = std::thread(std::bind(&DataPackage::datagramOpThread, this));
}

DataPackage::~DataPackage()
{
    isRun_ = false;

    if (datagramOpThread_.joinable())
    {
        eventDatagramData_.notify_one();
        datagramOpThread_.join();
    }

}

void DataPackage::pushData(size_t connectionId, const uint8_t* pData, size_t dataLength)
{
    MessageData* pRecData = constructMsgPacket(connectionId, pData, dataLength);

    {
        lock_guard<mutex> lg(lockDatagramData_);
        datagramData_.push_back(pRecData);
    }

    eventDatagramData_.notify_one();
}

bool DataPackage::AnalyzeCommand(shared_ptr<DataBuffer> dest, MessageData* pRD)
{
    size_t pos(0);
    int start(-1);
    bool isQuotation(false), isBackslash(false);
    int unmatchedLeftBraceNumbers(0);

    if (dest->getDataLength() > 0)
    {
        start = 0;
        auto pT = dest->getData();
        for (pos = 0; pos < dest->getDataLength(); ++pos)
        {
            CommonFunction::checkJsonChar(pT[pos], isBackslash, isQuotation, unmatchedLeftBraceNumbers);
        }
    }

    for (pos = 0; pos < pRD->DataLength; ++pos)
    {
        CommonFunction::checkJsonChar(pRD->Data[pos], isBackslash, isQuotation, unmatchedLeftBraceNumbers);

        if ((start == -1) && (unmatchedLeftBraceNumbers != 0))
        {
            start = pos;
            continue;
        }

        if ((start != -1) && (unmatchedLeftBraceNumbers == 0))
        {
            std::string cacheData;
            if (dest->getDataLength() > 0)
            {
                cacheData.append((const char*)dest->getData(), dest->getDataLength());
                cacheData.append((char*)&pRD->Data, pos + 1);
                dest->clear();
            }
            else
            {
                cacheData.append((char*)&pRD->Data[start], pos - start + 1);
            }

            if (messageProcess_)
            {
                messageProcess_(pRD->connection_id, cacheData);
            }

            start = -1;
            unmatchedLeftBraceNumbers = 0;
        }
    }

    if (start != -1)
    {
        if (dest->getDataLength() > 0)
        {
            return dest->appendData(pRD->Data, pRD->DataLength);
        }
        else
        {
            return dest->appendData(&pRD->Data[start], pRD->DataLength - start);
        }
    }

    return true;
}

void DataPackage::datagramOpThread()
{
    unordered_map<size_t, shared_ptr<DataBuffer>> connectBuffer;
    while (isRun_)
    {
        QQueue<MessageData*> DataBag;
        {
            std::unique_lock<std::mutex> lg(lockDatagramData_);
            eventDatagramData_.wait(lg, [this] {return !isRun_ || (!datagramData_.empty()); });
            if (!isRun_) { break; }
            DataBag.swap(datagramData_);
        }

        while (!DataBag.empty())
        {
            MessageData* pData = DataBag.dequeue();

            if (pData->DataLength == 0)
            {
                connectBuffer.erase(pData->connection_id);
            }
            else
            {
                auto itor = connectBuffer.find(pData->connection_id);
                if (itor == connectBuffer.end())
                {
                    shared_ptr<DataBuffer> pDB = make_shared<DataBuffer>();
                    AnalyzeCommand(pDB, pData);
                    connectBuffer.insert(make_pair(pData->connection_id, pDB));
                }
                else
                {
                    if (!AnalyzeCommand(itor->second, pData))
                    {
                        //无效数据超出缓冲区最大值，踢掉该连接
                        if (blacklistFunc_)
                        {
                            blacklistFunc_(pData->connection_id);
                        }
                    }
                }
            }
            destructMsgPacket(pData);
        }
    }
}

}

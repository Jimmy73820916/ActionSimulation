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

#include "databuffer.h"
#include <string>
#include <cassert>
#include "logger.h"
#include "commonconst.h"

using namespace std;

namespace Jimmy {

DataBuffer::DataBuffer()
    :bufferSize_(1024 * 64)
    , dataEndMark_(0)
{
    buffer_ = new uint8_t[bufferSize_];
}

DataBuffer::~DataBuffer()
{
    if (buffer_)
    {
        delete[] buffer_;
        buffer_ = nullptr;
    }
}

bool DataBuffer::appendData(const uint8_t* data, size_t length)
{
    if ((dataEndMark_ + length) > bufferSize_)
    {
        if ((dataEndMark_ + length) > CommonConst::JsonBagMaxLength)
        {
            return false;
        }

        bufferSize_ += length;
        uint8_t* pT = new uint8_t[bufferSize_];

        memcpy_s(pT, bufferSize_, buffer_, dataEndMark_);
        swap(pT, buffer_);

        delete[] pT;
        pT = nullptr;
    }

    memcpy_s(&buffer_[dataEndMark_], bufferSize_ - dataEndMark_, data, length);
    dataEndMark_ += length;
    return true;
}

size_t DataBuffer::getDataLength()
{
    return dataEndMark_;
}

const uint8_t* DataBuffer::getData()
{
    return buffer_;
}

char DataBuffer::getData(uint32_t pos)
{
    assert(pos < dataEndMark_);
    return buffer_[pos];
}

const uint8_t* DataBuffer::getSubData(uint32_t start)
{
    assert(start < dataEndMark_);
    return &buffer_[start];
}

void DataBuffer::clear()
{
    dataEndMark_ = 0;
}

};

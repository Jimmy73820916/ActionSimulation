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

#include "logger.h"

namespace Jimmy
{
template<class T>
class PointerScopeAssistant
{
public:
    PointerScopeAssistant(T** pointeer, bool bArray)
    {
        association(pointeer, bArray);
    }

    ~PointerScopeAssistant()
    {
        if (*pointeer_ == nullptr)
        {
            return;
        }

        if (bArray_)
        {
            delete[] * pointeer_;
        }
        else
        {
            delete* pointeer_;
        }
        *pointeer_ = nullptr;
    }

    void association(T** pointeer, bool bArray)
    {
        pointeer_ = pointeer;
        bArray_ = bArray;
    }
private:
    T** pointeer_;
    bool bArray_;
};


class FileCloseAssistant
{
public:
    FileCloseAssistant(FILE* fileHandle)
    {
        fileHandle_ = fileHandle;
    }

    ~FileCloseAssistant()
    {
        if (fileHandle_ == nullptr)
        {
            return;
        }

        fclose(fileHandle_);
    }

private:
    FILE* fileHandle_;
};


struct InvokeSpendInfo
{
    InvokeSpendInfo(const QString& invoke_name)
        :invoke_name_(invoke_name)
        , tp_(std::chrono::steady_clock::now())
    {
    }

    ~InvokeSpendInfo()
    {
        LOGDEBUG(QString("invoke %1 spend time %2ms").arg(invoke_name_).arg(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp_).count()));
    }
private:
    QString invoke_name_;
    std::chrono::steady_clock::time_point tp_;
};


}

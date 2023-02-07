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

#include <filesystem>
#include "logger.h"
#include "commonconst.h"
#include <io.h>
#include <time.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

using namespace std;
using namespace std::filesystem;



namespace Jimmy
{

ErrorCode getModuleName(string& ExeName)
{
    const int MaxPathLength = 4096;
    char path[MaxPathLength] = { 0 };

    if (!GetModuleFileNameA(nullptr, path, MaxPathLength))
    {
        return ec_error;
    }

    ExeName = path;
    return ec_ok;
}

ErrorCode getFileNameWithExt(const string& ExtName, string& out_filename)
{
    string filename;
    ErrorCode ec = getModuleName(filename);
    if (ec != ec_ok)
    {
        return ec;
    }

    auto pos = filename.find_last_of('.');
    if (pos != string::npos)
    {
        out_filename = filename.substr(0, pos);
    }
    else
    {
        out_filename = filename;
    }

    out_filename += ExtName;
    return ec_ok;
}

const char* Logger::getLogLevelDesc(LogLevel ll)
{
    switch (ll)
    {
    case Jimmy::LogLevel::LL_DEBUG:
        return "DEBUG";
    case Jimmy::LogLevel::LL_HINT:
        return "HINT";
    case Jimmy::LogLevel::LL_INFO:
        return "INFO";
    case Jimmy::LogLevel::LL_WARN:
        return "WARN";
    case Jimmy::LogLevel::LL_ERROR:
        return "ERROR";
    case Jimmy::LogLevel::LL_FATAL:
        return "FATAL";
    default:
        return "-----";
    }
}

const char* Logger::getTimeStr(const SYSTEMTIME& st)
{
	static char t[24];
    sprintf_s(t, "%04d-%02d-%02d %02d:%02d:%02d.%03d",st.wYear,st.wMonth,st.wDay,st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return t;
}

const char* Logger::makeLogMsg(const TLogMsg* plm)
{
    const size_t headlength = 64;
    static std::vector<char> msg;
    size_t nlength = plm->size + headlength;
    if (msg.size() < nlength)
    {
        msg.resize(nlength);
    }

	sprintf_s(&msg[0], nlength - 1, "%s %6d  [%-5s]  %s\r\n", getTimeStr(plm->messageTime), plm->threadId, getLogLevelDesc(plm->logLevel), plm->message);
	return &msg[0];
}

Logger::Logger()
    :logLevel_(LogLevel::LL_INFO)
    , isRun_(false)
    , lastErrorCode_(0)
    , toConsole_(false)
{

}

Logger::~Logger()
{
    if (isRun_)
    {
        release();
    }
}

void Logger::sendConsole(bool to_console)
{
    toConsole_ = to_console;
}

bool Logger::initialize(LogLevel ll, bool toConsole)
{
    if (isRun_)
    {
        return false;
    }
    isRun_ = true;

    makeDefaultLogFileName();
    if (!checkLogFile())
    {
        isRun_ = false;
        return false;
    }

    setLoglevel(ll);
    toConsole_ = toConsole;

    logThread_ = std::thread(std::bind(&Logger::logThreadFunc, this));

    return true;
}


void Logger::release()
{
    if (isRun_)
    {
        waitForWriteLogThreadComplate();
    }
}

bool Logger::hasInited() const
{
    return isRun_;
}

LogLevel Logger::getLoglevel() const
{
    return logLevel_;
}

void Logger::setLoglevel(LogLevel ll)
{
    logLevel_ = ll;
}

QString Logger::getLogFile() const
{
    return QString::fromLocal8Bit(logFullName_.c_str());
}

void Logger::logDebug(const QString& logMessage)
{
    if (!isRun_ || logMessage.isEmpty() || (LogLevel::LL_DEBUG < getLoglevel()))
    {
        return;
    }

    logV(LogLevel::LL_DEBUG, logMessage);
}

void Logger::logHint(const QString& logMessage)
{
    if (!isRun_ || logMessage.isEmpty() || (LogLevel::LL_HINT < getLoglevel()))
    {
        return;
    }

    logV(LogLevel::LL_HINT, logMessage);
}

void Logger::logInfo(const QString& logMessage)
{
    if (!isRun_ || logMessage.isEmpty() || (LogLevel::LL_INFO < getLoglevel()))
    {
        return;
    }

    logV(LogLevel::LL_INFO, logMessage);
}

void Logger::logWarn(const QString& logMessage)
{
    if (!isRun_ || logMessage.isEmpty() || (LogLevel::LL_WARN < getLoglevel()))
    {
        return;
    }

    logV(LogLevel::LL_WARN, logMessage);
}

void Logger::logError(const QString& logMessage)
{
    if (!isRun_ || logMessage.isEmpty() || (LogLevel::LL_ERROR < getLoglevel()))
    {
        return;
    }

    logV(LogLevel::LL_ERROR, logMessage);
}

void Logger::logFatal(const QString& logMessage)
{
    if (!isRun_ || logMessage.isEmpty() || (LogLevel::LL_FATAL < getLoglevel()))
    {
        return;
    }

    logV(LogLevel::LL_FATAL, logMessage);
}

void Logger::log(LogLevel ll, const QString& logMessage)
{
    if (!isRun_ || logMessage.isEmpty() || (ll < getLoglevel()))
    {
        return;
    }

    logV(ll, logMessage);
}

Logger::TLogMsg* Logger::constructLogMsg(LogLevel ll, uint32_t tid, const QString& logMessage, const SYSTEMTIME* pMsgTime)
{
    QByteArray msg = logMessage.toLocal8Bit();
    size_t msglen = msg.size() + 1;
    size_t size = sizeof(Logger::TLogMsg) + msglen;

    Logger::TLogMsg* pLogMsg = static_cast<Logger::TLogMsg*>(malloc(size));
    pLogMsg->size = size;
    pLogMsg->logLevel = ll;
    pLogMsg->threadId = tid;

    if (pMsgTime)
    {
        memcpy(&pLogMsg->messageTime, pMsgTime, sizeof(SYSTEMTIME));
    }

    strncpy_s(pLogMsg->message, msglen, msg, msglen);
    pLogMsg->message[msglen] = 0;

    return pLogMsg;
}

void Logger::destructLogMsg(TLogMsg* p)
{
    if(p)
    {
        free((void*)p);
        p = nullptr;
    }
}

void Logger::logV(LogLevel ll, const QString& logMessage)
{
    TLogMsg* plm = constructLogMsg(ll, ::GetCurrentThreadId(), logMessage);
    {
        lock_guard<mutex> lg(lockLogList_);
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        memcpy(&plm->messageTime, &st, sizeof(SYSTEMTIME));
        logList_.push_back(plm);
        if (ll == LogLevel::LL_FATAL)
        {
            logList_.push_back(nullptr);
        }
    }

    evLogList_.notify_one();
}

int Logger::logThreadFunc()
{
    bool bFatal(false);

    while (true)
    {
        QList<TLogMsg*> log_list;
        {
            unique_lock<mutex> lg(lockLogList_);
            evLogList_.wait(lg, [this] {return (!isRun_)||(!logList_.empty()); });
            if (!isRun_)break;
            logList_.swap(log_list);
        }

        FILE* hFile = openLogFile();
        if (hFile == nullptr)
        {
            continue;
        }

        while (!log_list.empty())
        {
            TLogMsg* plm = log_list.front();

            if (plm == nullptr)
            {
                bFatal = true;
                break;
            }

            const char* msg = makeLogMsg(plm);
            fprintf_s(hFile, "%s", msg);

            if (toConsole_)
            {
                printf_s("%s", msg);
            }

            destructLogMsg(plm);
            log_list.pop_front();
        }

        fclose(hFile);

        if (bFatal)
        {
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

ErrorCode Logger::makeDefaultLogFileName()
{
	string moduleName;

    ErrorCode ec = getModuleName(moduleName);
	if (ec != ec_ok)
	{
		return ec;
	}

	path fp(moduleName);
	auto parent_path = fp.parent_path();
	parent_path.append("logs");

	if (!exists(parent_path))
	{
		if (!create_directory(parent_path))
		{
			return ec_error;
		}
	}

    logPath_ = parent_path.string().c_str();
    logFileStemName_ = fp.stem().string().c_str();

	return ec_ok;
}

void Logger::setFullLogFileName()
{
	time_t ctm = time(nullptr);

	if (logFullName_.empty() ||
		((ctm / CommonConst::SecondPerDay) != (lastTime_ / CommonConst::SecondPerDay)))
	{
		logFileIndex_ = 1;
		lastTime_ = ctm;
		makeFullLogFileNameWithIndex();
	}

	while (true)
	{
        path fileInfo(logFullName_);
		if (!exists(fileInfo))
		{
			break;
		}

		auto size = file_size(fileInfo);
		if (size < Logfile_max_length)
		{
			break;
		}

		logFileIndex_++;
		makeFullLogFileNameWithIndex();
	}
}

void Logger::makeFullLogFileNameWithIndex()
{
    const int MaxLogFileLength = 64;
    char strDate[MaxLogFileLength] = { 0 };
    struct tm ctm { 0 };
    localtime_s(&ctm, &lastTime_);
    sprintf_s(strDate, "_%04d%02d%02d_%d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday, logFileIndex_);

    logFullName_ = logPath_ + CommonConst::PathSplit + logFileStemName_;
    logFullName_.append(strDate);
    logFullName_.append(".log");
}

void Logger::waitForWriteLogThreadComplate()
{
    isRun_ = false;
    if (logThread_.joinable())
    {
        evLogList_.notify_one();
        logThread_.join();
    }
}

bool Logger::checkLogFile()
{
    errno = 0;
    FILE* file = openLogFile();

    if (file == nullptr)
    {
        lastErrorCode_ = static_cast<ErrorCode>(errno);
        return false;
    }

    fclose(file);
    return true;
}

FILE* Logger::openLogFile()
{
    setFullLogFileName();

    FILE* file(nullptr);
    fopen_s(&file, logFullName_.c_str(), "a");
    return file;
}

}

std::mutex GlobalLogger::lock_log;
Jimmy::Logger* GlobalLogger::logger_;

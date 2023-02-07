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

#include <QList>
#include <QString>
#include <windows.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <condition_variable>
#include "errorcode.h"

namespace Jimmy
{

enum class LogLevel
{
    LL_DEBUG,
    LL_HINT,
    LL_INFO,
    LL_WARN,
    LL_ERROR,
    LL_FATAL,
};

class Logger
{
    struct TLogMsg
    {
        size_t          size;
        LogLevel        logLevel;
        uint32_t        threadId;
        SYSTEMTIME      messageTime;
        char            message[1];
    };
public:
    Logger();
    ~Logger();

    bool initialize(LogLevel ll = DEFAULT_LOG_LEVEL, bool toConsole = false);
    void release();

    void sendConsole(bool toConsole);

    bool hasInited() const;
    LogLevel getLoglevel() const;
    void	setLoglevel(LogLevel ll);

    QString getLogFile() const;

    void logDebug(const QString& logMessage);
    void logHint(const QString& logMessage);
    void logInfo(const QString& logMessage);
    void logWarn(const QString& logMessage);
    void logError(const QString& logMessage);
    void logFatal(const QString& logMessage);
    void log(LogLevel ll, const QString& logMessage);
private:
    static const char* getLogLevelDesc(LogLevel ll);
    static const char* getTimeStr(const SYSTEMTIME& st);
    static const char* makeLogMsg(const TLogMsg* plm);

    TLogMsg* constructLogMsg(LogLevel ll, uint32_t tid, const QString& logMessage, const SYSTEMTIME* pMsgTime = nullptr);
    void destructLogMsg(TLogMsg* p);

    void logV(LogLevel ll, const QString& logMessage);

    int logThreadFunc();

    ErrorCode makeDefaultLogFileName();
    void setFullLogFileName();
    void makeFullLogFileNameWithIndex();
    void waitForWriteLogThreadComplate();

    bool checkLogFile ();
    FILE* openLogFile();
private:
    QList<TLogMsg*> logList_;
    volatile LogLevel logLevel_;

    static const int32_t Max_File_Path = 4096;
    static const int32_t Logfile_max_length = 50 * 1024 * 1024;
    int32_t logFileIndex_;

    /*
       由于对于服务来说全局日志的建立可能会早于 QCoreApplication 对象出现 QCoreApplication::applicationDirPath() 使用会出错
       所以使用C++ filesystem
    */
    std::string logFileStemName_;
    std::string logPath_;
    std::string logFullName_;

    std::condition_variable evLogList_;
    std::mutex	lockLogList_;
    std::thread logThread_;

    mutable bool isRun_;
    int lastErrorCode_;
    time_t lastTime_;
    bool toConsole_;

    static const LogLevel DEFAULT_LOG_LEVEL =
#ifdef _DEBUG
        LogLevel::LL_DEBUG;
#else
        LogLevel::LL_INFO;
#endif

};

}

class GlobalLogger
{
public:
    static Jimmy::Logger* get_instance()
    {
        if (logger_ == nullptr)
        {
            std::lock_guard<std::mutex> lg(lock_log);
            if (logger_ == nullptr)
            {
                logger_ = new Jimmy::Logger();
                logger_->initialize();
            }
        }

        return logger_;
    }
private:
    GlobalLogger() = delete;
    GlobalLogger(const GlobalLogger&) = delete;
    GlobalLogger& operator=(const GlobalLogger&) = delete;
private:
    static std::mutex lock_log;
    static Jimmy::Logger* logger_;
};

#define LOGDEBUG(x)		if(GlobalLogger::get_instance()->getLoglevel() == Jimmy::LogLevel::LL_DEBUG) { GlobalLogger::get_instance()->logDebug(x);}
#define LOGHINT(x)		GlobalLogger::get_instance()->logHint(x);
#define LOGINFO(x)		GlobalLogger::get_instance()->logInfo(x);
#define LOGWARN(x)		GlobalLogger::get_instance()->logWarn(x);
#define LOGERROR(x)		GlobalLogger::get_instance()->logError(x);
#define LOGFATAL(x)		GlobalLogger::get_instance()->logFatal(x);


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

#include <thread>
#include <mutex>
#include <condition_variable>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include "commonconst.h"
#include "logger.h"

class AppConfig
{
    Q_DISABLE_COPY(AppConfig)
public:
    AppConfig();
    ~AppConfig();

    bool loadSucceed() { return isRun_; }

    //获取服务名称
    const QString& getServiceName() { return serviceName_; }

    //获取服务描述
    const QString& getServiceDescription() { return serviceDescription_; }

    //获取项目名称
    const QFileInfo& getProjectFile() { return projectName_; }

    //获取监听地址
    const QString& getListenAddress() { return address_; }

    //获取监听端口
    uint16_t getListenPort() { return port_; }

    //获取日志保存天数
    uint32_t getLogRetainDays() { return logRetainDays_; }

    //获取日志级别
    Jimmy::LogLevel getLogLevel() { return logLevel_; }
private:
    bool loadConfiguration(const QString& fileName);
private:
    void clearLogs();
    std::thread clearLogThread_;
    std::mutex lockClearLogs_;
    std::condition_variable cvClearLogs_;
private:
    bool isRun_;

    QString serviceName_;
    QString serviceDescription_;

    QFileInfo projectName_;

    QString address_;
    uint16_t port_;

    uint32_t logRetainDays_;
    Jimmy::LogLevel logLevel_;
};

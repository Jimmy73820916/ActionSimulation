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

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include "actionsimulationserver.h"
#include "appconfig.h"
#include "commonfunction.h"
#include "errorcode.h"

using namespace std;
using namespace Jimmy;

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

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

void AppConfig::clearLogs()
{
    QFileInfo logFile(GlobalLogger::get_instance()->getLogFile());
    QDir logPath(logFile.dir());

    while (isRun_)
    {
        QFileInfoList list = logPath.entryInfoList();
        for (int i = 0; i < list.count(); i++)
        {
            const auto& file = list.at(i);
            if (file.isFile())
            {
                if (file.lastModified().addDays(logRetainDays_) < QDateTime::currentDateTime())
                {
                    logPath.remove(file.fileName());
                }
            }
        }

        {
            unique_lock<mutex> lg(lockClearLogs_);
            cvClearLogs_.wait_for(lg, chrono::hours(24));
        }
    }
}

AppConfig::AppConfig()
    :isRun_(false)
    , port_(0)
    , logRetainDays_(15)
    , logLevel_(LogLevel::LL_INFO)
{
    std::string configFilename;
    if (ErrorCode::ec_ok != getFileNameWithExt(".json", configFilename))
    {
        return;
    }

    isRun_ = loadConfiguration(QString::fromStdString(configFilename));
    if (isRun_)
    {
        clearLogThread_ = std::thread(std::bind(&AppConfig::clearLogs, this));
    }
}

AppConfig::~AppConfig()
{
    if (isRun_ && clearLogThread_.joinable())
    {
        isRun_ = false;
        cvClearLogs_.notify_one();
        clearLogThread_.join();
    }
}

bool AppConfig::loadConfiguration(const QString& fileName)
{
    QFile file(fileName.toLocal8Bit());
    if(!file.open(QIODevice::ReadOnly))
    {
        LOGERROR(QStringLiteral("[%1:%2] load %3 is failed")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(fileName));

        return false;
    }

    QJsonParseError error;
    QJsonDocument jd = QJsonDocument::fromJson(file.readAll(),&error);

    if(error.error!=QJsonParseError::NoError)
    {
        LOGERROR(QStringLiteral("[%1:%2] load %3 is failed")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(fileName));

        return false;
    }

    QJsonObject docObj = jd.object();
    auto memItor = docObj.find("service");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        LOGERROR(QStringLiteral("[%1:%2] key service is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return false;
    }

    auto memElem = memItor->toObject();
    {
        auto itor = memElem.find("name");
        if((itor == docObj.end())||(!itor->isString()))
        {
            LOGERROR(QStringLiteral("[%1:%2] key service name is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }
        serviceName_ = itor->toString();

        itor = memElem.find("description");
        if((itor == docObj.end())||(!itor->isString()))
        {
            LOGERROR(QStringLiteral("[%1:%2] key service description is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }
        serviceDescription_ = itor->toString();
    }

    memItor = docObj.find("projects");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        LOGERROR(QStringLiteral("[%1:%2] key projects is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return false;
    }

    memElem = memItor->toObject();
    {
        auto itor = memElem.find("project_name");
        if ((itor == docObj.end()) || (!itor->isString()))
        {
            LOGERROR(QStringLiteral("[%1:%2] key projects path is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }

        projectName_ = QFileInfo(itor->toString());
        if(!projectName_.exists())
        {
            LOGERROR(QStringLiteral("[%1:%2] project file is not exist")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }
    }

    memItor = docObj.find("listen");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        LOGERROR(QStringLiteral("[%1:%2] key listen is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return false;
    }

    memElem = memItor->toObject();
    {
        auto itor = memElem.find("address");
        if((itor == docObj.end())||(!itor->isString()))
        {
            LOGERROR(QStringLiteral("[%1:%2] listen address is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }
        address_ = itor->toString();

        itor = memElem.find("port");
        if((itor == docObj.end())||(!itor->isDouble()))
        {
            LOGERROR(QStringLiteral("[%1:%2] listen port is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }
        port_ = itor->toInt();
        if(port_ == 0)
        {
            LOGERROR(QStringLiteral("[%1:%2] listen port is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }
    }

    memItor = docObj.find("miscellaneous");
    if((memItor == docObj.end())||(!memItor->isObject()))
    {
        LOGERROR(QStringLiteral("[%1:%2] key miscellaneous is invalid")
            .arg(__FUNCTION__)
            .arg(__LINE__));

        return false;
    }

    memElem = memItor->toObject();
    {
        auto itor = memElem.find("logretaindays");
        if((itor == docObj.end())||(!itor->isDouble()))
        {
            LOGERROR(QStringLiteral("[%1:%2] key miscellaneous logretaindays is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }
        logRetainDays_ = itor->toInt();

        itor = memElem.find("loglevel");
        if((itor == docObj.end())||(!itor->isDouble()))
        {
            LOGERROR(QStringLiteral("[%1:%2] key miscellaneous loglevel is invalid")
                .arg(__FUNCTION__)
                .arg(__LINE__));

            return false;
        }
        logLevel_ = static_cast<LogLevel>(itor->toInt());
        GlobalLogger::get_instance()->setLoglevel(logLevel_);
    }

    return true;
}

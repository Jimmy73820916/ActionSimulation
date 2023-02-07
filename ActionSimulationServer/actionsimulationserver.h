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

#include <QCoreApplication>
#include "qtservice.h"
#include "tcpserver.h"


class ActionSimulationService: public QtService<QCoreApplication>
{
public:
    ActionSimulationService(int argc, char **argv);
protected:
    void start();
    void stop();
};

class AppConfig;
class ProjectManager;
class UserManager;

class ActionSimulationServer
{
public:
    ActionSimulationServer() = default;
    ~ActionSimulationServer() = default;

   bool loadConfiguration();

   void registerAppendConnnection(std::function<Jimmy::User(size_t)> connection);
   void registerRemoveConnnection(std::function<void(size_t)> connection);

   void registerMessageProcessFunction(std::function<void(size_t, const std::string&)> messageProcess);

   void sendNetMessage(size_t connectionid, const QString& message);

   std::shared_ptr<AppConfig> getAppConfig();
   std::shared_ptr<ProjectManager> getProjectManager();
   std::shared_ptr<UserManager> getUserManager();

   void start();
   void stop();
private:
    bool initializeSystemConfig();
    void releaseSystemConfig();
private:
    std::shared_ptr<AppConfig> appConfig_;
    std::shared_ptr<ProjectManager> projectManager_;
    std::shared_ptr<UserManager> userManager_;
    std::unique_ptr<Jimmy::TcpServer> tcpServer_;
};

extern ActionSimulationServer gActionSimulationServer;

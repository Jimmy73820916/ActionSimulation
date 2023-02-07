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

#include "actionsimulationserver.h"
#include "appconfig.h"
#include "projectmanager.h"
#include "usermanager.h"

using namespace std;
using namespace Jimmy;

ActionSimulationServer gActionSimulationServer;

ActionSimulationService::ActionSimulationService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, gActionSimulationServer.getAppConfig()->getServiceName())
{
    setServiceDescription(gActionSimulationServer.getAppConfig()->getServiceDescription());
    setServiceFlags(QtServiceBase::Default);
}

void ActionSimulationService::start()
{
    gActionSimulationServer.start();
}

void ActionSimulationService::stop()
{
    gActionSimulationServer.stop();
}

bool ActionSimulationServer::loadConfiguration()
{
    if (!appConfig_)
    {
        appConfig_ = make_shared<AppConfig>();
    }

    return appConfig_->loadSucceed();
}

bool ActionSimulationServer::initializeSystemConfig()
{
    if (!tcpServer_)
    {
        tcpServer_ = make_unique<TcpServer>();
    }

    if (!userManager_)
    {
        userManager_ = make_shared<UserManager>();
    }

    if (!projectManager_)
    {
        projectManager_ = make_shared<ProjectManager>();
    }

    boost::asio::ip::address addr;
    addr.from_string(appConfig_->getListenAddress().toStdString());
    boost::asio::ip::tcp::endpoint ep(addr, appConfig_->getListenPort());
    tcpServer_->start(ep);

    if(projectManager_->getStatus() == ProjectStatus::stopped)
    {
        this_thread::sleep_for(chrono::seconds(2));
        projectManager_->run();
    }

    tcpServer_->registerMessageProcessFunction(std::bind(&ProjectManager::pushMessage, projectManager_.get(), placeholders::_1, placeholders::_2));
    tcpServer_->registerAppendConnnection(std::bind(&UserManager::registerConnection, userManager_.get(), placeholders::_1));
    tcpServer_->registerRemoveConnnection(std::bind(&UserManager::disconnect, userManager_.get(), placeholders::_1));

    return true;
}

void ActionSimulationServer::releaseSystemConfig()
{
    userManager_->clear();

    if (projectManager_->getStatus() == ProjectStatus::running)
    {
        projectManager_->stop();
    }

    tcpServer_->stop();
}

void ActionSimulationServer::sendNetMessage(size_t connectionid, const QString& message)
{
    if (connectionid == 0)
    {
        LOGINFO(message);
        return;
    }

    tcpServer_->sendData(connectionid,message.toLocal8Bit());
}

void ActionSimulationServer::registerAppendConnnection(std::function<Jimmy::User(size_t)> connection)
{
    tcpServer_->registerAppendConnnection(connection);
}

void ActionSimulationServer::registerRemoveConnnection(std::function<void(size_t)> connection)
{
    tcpServer_->registerRemoveConnnection(connection);
}

void ActionSimulationServer::registerMessageProcessFunction(std::function<void(size_t, const std::string&)> messageProcess)
{
    tcpServer_->registerMessageProcessFunction(messageProcess);
}

std::shared_ptr<AppConfig> ActionSimulationServer::getAppConfig()
{
    return appConfig_;
}

std::shared_ptr<ProjectManager> ActionSimulationServer::getProjectManager()
{
    return projectManager_;
}

std::shared_ptr<UserManager> ActionSimulationServer::getUserManager()
{
    return userManager_;
}

void ActionSimulationServer::start()
{
    initializeSystemConfig();
}

void ActionSimulationServer::stop()
{
    releaseSystemConfig();
}

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

#include <QObject>
#include <lua.hpp>
#include <mutex>
#include <optional>
#include <QJsonObject>
#include "errorcode.h"

namespace Jimmy
{
class ActionScript
{
    Q_DISABLE_COPY(ActionScript)
public:
    ActionScript(const QString& role);
    ~ActionScript();

    ErrorCode start();
    ErrorCode reload();
    void stop();

    std::optional<QJsonObject> onInitialize(const QJsonObject& inputParams);
    std::optional<QJsonObject> onAction(const QJsonObject& inputParams);
    std::optional<QJsonObject> onTime(const QJsonObject& inputParams);
private:
    std::optional<QJsonObject> onScript_(const QString& action,const QJsonObject& inputParams);
    ErrorCode load_();
private:
    std::mutex  luaStateLock_;
    lua_State*  luaState_;

    QString luaScriptName_;
	QString functionOnInitialize_;
    QString functionOnAction_;
    QString functionOnTime_;

    bool is_run_;
};
}



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
#include "actionscript.h"
#include "appconfig.h"
#include <QJsonDocument>

using namespace std;

namespace Jimmy
{

QString getScriptPath()
{
    return QStringLiteral("%1\\componentscript").arg(gActionSimulationServer.getAppConfig()->getProjectFile().absolutePath());
}

QString getDllPath()
{
    return QStringLiteral("%1").arg(QCoreApplication::applicationDirPath());
}

void luaAddPath(lua_State* luaState, const char* name, const char* value)
{
    std::string v;

    lua_getglobal(luaState, "package");
    lua_getfield(luaState, -1, name);
    v.append(lua_tostring(luaState, -1));
    v.append(";");
    v.append(value);
    lua_pushstring(luaState, v.c_str());
    lua_setfield(luaState, -3, name);
    lua_pop(luaState, 2);
}

ActionScript::ActionScript(const QString& role)
    :is_run_(false)
{
    luaState_ = luaL_newstate();
    luaL_openlibs(luaState_);
    luaAddPath(luaState_, "path", QStringLiteral("%1\\?.lua;%1\?.out").arg(getScriptPath()).toLocal8Bit());
    luaAddPath(luaState_, "cpath",QStringLiteral("%1\\?.dll").arg(getDllPath()).toLocal8Bit());

    QString luaScriptName = QStringLiteral("%1\\%2").arg(getScriptPath(), role);

    QString luac_file = luaScriptName + ".out";
    QString lua_file = luaScriptName + ".lua";

    QFileInfo lua(lua_file);

    if (lua.exists())
    {
        luaScriptName_ = lua_file;
    }
    else
    {
        QFileInfo luac(luac_file);
        if (luac.exists())
        {
            luaScriptName_ = luac_file;
        }


        if (luaScriptName_.isEmpty())
        {
            LOGERROR(QStringLiteral("[%1:%2] load script %3 or %4 is failed")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(luac_file,lua_file));

            return;
        }
    }

    functionOnInitialize_ = QStringLiteral("on_initialize");
    functionOnAction_ = QStringLiteral("on_action");
    functionOnTime_ = QStringLiteral("on_time");
}

ActionScript::~ActionScript()
{
    if (luaState_)
    {
        lua_close(luaState_);
        luaState_ = nullptr;
    }
}

ErrorCode ActionScript::start()
{
    if(is_run_)
    {
        return ErrorCode::ec_error;
    }

    return load_();
}

ErrorCode ActionScript::reload()
{
    if(!is_run_)
    {
        return ErrorCode::ec_error;
    }

    is_run_ = false;

    return load_();
}

void ActionScript::stop()
{
    is_run_ = false;
}

Jimmy::ErrorCode ActionScript::load_()
{
    lock_guard<mutex> lg(luaStateLock_);

    try
    {
        int ret = luaL_loadfile(luaState_, luaScriptName_.toLocal8Bit().constData());
        if (ret != LUA_OK)
        {
            LOGERROR(QStringLiteral("[%1:%2] load %3 is failed")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(luaScriptName_));

            return ErrorCode::ec_error;
        }

        ret = lua_pcall(luaState_, 0, 0, 0);
        if (ret != LUA_OK)
        {
            LOGERROR(QStringLiteral("[%1:%2] load %3 lua_pcall failed. cause:%4")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(luaScriptName_,lua_tostring(luaState_,-1)));

            return ErrorCode::ec_error;
        }
    }
    catch (...)
    {
        LOGERROR(QStringLiteral("[%1:%2] load %3 is failed.")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(luaScriptName_));

        return ErrorCode::ec_error;
    }

    is_run_ = true;
    return ErrorCode::ec_ok;
}

std::optional<QJsonObject> ActionScript::onInitialize(const QJsonObject& inputParams)
{
	if (!is_run_)
	{
		return std::nullopt;
	}

	return onScript_(functionOnInitialize_, inputParams);
}

std::optional<QJsonObject> ActionScript::onAction(const QJsonObject& inputParams)
{
    if(!is_run_)
    {
        return std::nullopt;
    }

    return onScript_(functionOnAction_, inputParams);
}

std::optional<QJsonObject> ActionScript::onTime(const QJsonObject& inputParams)
{
    if(!is_run_)
    {
        return std::nullopt;
    }

    return onScript_(functionOnTime_, inputParams);
}

std::optional<QJsonObject> ActionScript::onScript_(const QString& action, const QJsonObject& inputParams)
{
    lock_guard<mutex> lg(luaStateLock_);

    try
    {
        lua_getglobal(luaState_, action.toLocal8Bit());
        QByteArray params = QJsonDocument(inputParams).toJson();
        lua_pushstring(luaState_, params);
        int ret = lua_pcall(luaState_, 1, 1, 0);
        if(ret != LUA_OK)
        {
            LOGERROR(QStringLiteral("[%1:%2] run %3 %4(\"%5\") failed,return %6,cause:%7")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(luaScriptName_,action,params)
                .arg(ret)
                .arg(lua_tostring(luaState_,-1)));

            return std::nullopt;
        }

        int lType = lua_type(luaState_, -1);
        if(lType == LUA_TNIL)
        {
            return std::nullopt;
        }

        if (lType == LUA_TNUMBER)
        {
			LOGERROR(QStringLiteral("[%1:%2] run %3 %4(\"%5\") failed,return %6")
				.arg(__FUNCTION__)
				.arg(__LINE__)
				.arg(luaScriptName_, action, params)
				.arg(ret)
				.arg(lua_tostring(luaState_, -1)));

            lua_pop(luaState_, -1);
            return std::nullopt;
        }

        if (lType != LUA_TSTRING)
        {
			LOGERROR(QStringLiteral("[%1:%2] run %3 %4(\"%5\") failed,return %6")
				.arg(__FUNCTION__)
				.arg(__LINE__)
				.arg(luaScriptName_, action, params)
				.arg(lua_tostring(luaState_, -1)));

            lua_pop(luaState_, -1);
            return std::nullopt;
        }

        QByteArray result = lua_tostring(luaState_, -1);

        QJsonParseError error;
        QJsonDocument jd = QJsonDocument::fromJson(result,&error);

        LOGDEBUG(QStringLiteral("[%1:%2] %3 %4(\"%5\") return %6")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(luaScriptName_,action, params, result));


        lua_pop(luaState_, -1);
        if(error.error!=QJsonParseError::NoError)
        {
            LOGERROR(QStringLiteral("[%1:%2] %3 is not a json string")
                .arg(__FUNCTION__)
                .arg(__LINE__)
                .arg(result.constData()));

            return std::nullopt;
        }

        return jd.object();
    }
    catch (...)
    {
        LOGERROR(QStringLiteral("[%1:%2] run %3 throw exception")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(action));
    }

    return std::nullopt;
}


}


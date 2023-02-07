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
#include <QString>
#include <cstdint>

namespace Jimmy
{
Q_NAMESPACE
enum ComponentType
{
    Internal,                   //内部设备
    Input,                      //输入设备
    Output,
    TeamMaster,                 //设备组中的主设备,主设备可以订阅其他组件但不应被订阅
    TeamSlave,                  //设备组中的从设备,从设备不能订阅其他组件但可以被订阅，从设备不能响应广播
    ComponentType_Uplimit,
};
Q_ENUM_NS(ComponentType)

inline bool validDevicetype(ComponentType dt)
{
    return dt >= ComponentType::Internal && dt < ComponentType::ComponentType_Uplimit;
}

inline QString getTypeString(ComponentType componentType)
{
    switch(componentType)
    {
    case ComponentType::Internal: return QStringLiteral("内部设备");
    case ComponentType::Input: return QStringLiteral("输入设备");
    case ComponentType::Output: return QStringLiteral("输出设备");
    case ComponentType::TeamMaster: return QStringLiteral("设备组(主)");
    case ComponentType::TeamSlave: return QStringLiteral("设备组(从)");
    default: return "";
    }
}

inline ComponentType getComponentTypeValue(const QString& value)
{
    for(int index = 0; index < ComponentType::ComponentType_Uplimit;++index)
    {
        if(value == getTypeString(static_cast<ComponentType>(index)))
        {
            return static_cast<ComponentType>(index);
        }
    }

    return ComponentType::ComponentType_Uplimit;
}

enum BehaviorType
{
    Script,
    EqualInput,					//等于输入(开关)。设备只有一个依赖设备时有效.输入设备唯一的行为方式
    EqualInputIgnoreReset,      //输出等于输入,reset不发信号(按键)。 设备只有一个订阅设备时有效
    ReversalIgnoreOther,        //反转(true<->false), reset不发信号(按钮)。 设备只有一个订阅设备切设备取值为true-false时有效
    MasterSet,                  //主设备设置
    BehaviorType_Uplimit,
};
Q_ENUM_NS(BehaviorType)

inline bool validBehaviortype(BehaviorType bt)
{
    return bt >= BehaviorType::Script && bt < BehaviorType::BehaviorType_Uplimit;
}

inline QString getBehaviorString(BehaviorType behaviorType)
{
    switch(behaviorType)
    {
    case BehaviorType::Script: return QStringLiteral("脚本");
    case BehaviorType::EqualInput: return QStringLiteral("等于输入");
    case BehaviorType::EqualInputIgnoreReset: return QStringLiteral("等于输入,复位不发信号");
    case BehaviorType::ReversalIgnoreOther: return QStringLiteral("置位反转,复位不发信号");
    case BehaviorType::MasterSet: return QStringLiteral("主设备设置");
    default: return "";
    }
}

inline BehaviorType getBehaviorValue(const QString& value)
{
    for(int index = 0; index < BehaviorType::BehaviorType_Uplimit;++index)
    {
        if(value == getBehaviorString(static_cast<BehaviorType>(index)))
        {
            return static_cast<BehaviorType>(index);
        }
    }

    return BehaviorType::BehaviorType_Uplimit;
}

namespace CommonConst
{
const int32_t MaxNameLength = 64;
const size_t TCPBufferLength = 8192;
const int32_t SecondPerDay = 86400;

const char JSONSTART = '{';
const char JSONSEND = '}';
const char Quotation = '"';
const char BackSlash = '\\';

const char* const PathSplit = "\\";

const char* const ProjectSuffix = ".json";

const int32_t JsonBagMaxLength = 1024 * 1024 * 100;

const char* const Boardcast = "_boardcast";
const char* const TimerEvent = "_Timer";
const char* const LoopEvent = "_Loop";
const char* const OrderEvent = "_Order";

const char* const single_user = "single_user";
const char* const multi_users = "multi_users";
const char* const ValueKey = "_value";
const char* const CalculateDefaultValue = "_calculate_default_value";
}

}

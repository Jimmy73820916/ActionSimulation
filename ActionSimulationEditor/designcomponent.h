
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
#pragma once

#include <QStringList>
#include <QSet>
#include <QVector>
#include "commonconst.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


namespace Jimmy
{

class DesignComponent
{
public:
    DesignComponent() = default;
    DesignComponent(const DesignComponent&) = default;
    DesignComponent& operator=(const DesignComponent&) = default;

    DesignComponent(DesignComponent&&) = default;
    DesignComponent& operator=(DesignComponent&&) = default;

    QString getID() const { return id_; }

    template<typename T>
    void setID(T&& id)
    {
        id_ = std::forward<T>(id);
    }
    QString getName() const { return name_; }

    template<typename T>
    void setName(T&& name)
    {
        name_ = std::forward<T>(name);
    }

    QString getCaption() const { return ShowID ? getID() :getName(); }

    QString getCategory() const { return category_; }

    template<typename T>
    void setCategory(T&& category)
    {
        category_ = std::forward<T>(category);
    }

    ComponentType getType() const { return componentType_; }

    void setType(ComponentType componentType)
    {
        componentType_ = componentType;
    }

    QString getTeam() const { return team_; }

    template<typename T>
    void setTeam(T&& team)
    {
        team_ = std::forward<T>(team);
    }

    QString getRole() const
    {
        return role_;
    }

    template<typename T>
    void setRole(T&& role)
    {
        role_ = std::forward<T>(role);
    }

    BehaviorType getBehavior() const
    {
        return behaviorType_;
    }

    void setBehavior(BehaviorType behaviorType)
    {
        behaviorType_ = behaviorType;
    }

    double getActionKeep() const
    {
        if (componentType_ == ComponentType::Input)
        {
            return actionKeep_;
        }

        return 0;
    }

    void setActionKeep(double val)
    {
        actionKeep_ = val;
    }

    QStringList getSubscription() const { return subscription_; }

    template<typename T>
    void setSubscription(T&& subscription)
    {
        subscription_ = std::forward<T>(subscription);
    }

    QStringList getReference() const { return reference_; }

    template<typename T>
    void setReference(T&& reference)
    {
        reference_ = std::forward<T>(reference);
    }

    QStringList getRespondBoardcast() const { return respondBoardcast_; }

    template<typename T>
    void setRespondBoardcast(T&& respondBoardcast)
    {
        respondBoardcast_ = std::forward<T>(respondBoardcast);
    }

    QString getDescription() const { return description_; }

    template<typename T>
    void setDescription(T&& description)
    {
        description_ = std::forward<T>(description);
    }

    QJsonValue getDefaultValue() const { return defaultValue_; }

    template<typename T>
    void setDefaultValue(T&& val)
    {
        defaultValue_ = std::forward<T>(val);
    }

    bool needInitialize()
    {
        if(!defaultValue_.isString())
        {
            return false;
        }

        return defaultValue_.toString() == CommonConst::CalculateDefaultValue;
    }

    QString getDefaultValueString() const
    {
        if(defaultValue_.isDouble())
        {
            return QStringLiteral("%1").arg(defaultValue_.toDouble());
        }
        else if(defaultValue_.isBool())
        {
            return  defaultValue_.toBool() ? "true" : "false";
        }
        else if(defaultValue_.isString())
        {
            QString val = defaultValue_.toString();
            if((val.compare("true",Qt::CaseInsensitive)==0)||(val.compare("false",Qt::CaseInsensitive)==0))
            {
                return QStringLiteral("\"%1\"").arg(defaultValue_.toString());
            }

            bool bok(false);
            auto doubleVal = val.toDouble(&bok);
            if(bok)
            {
                return QStringLiteral("\"%1\"").arg(doubleVal);
            }
            return QStringLiteral("%1").arg(defaultValue_.toString());
        }
        else if(defaultValue_.isObject())
        {
            return QJsonDocument(defaultValue_.toObject()).toJson(QJsonDocument::Compact);
        }
        else if(defaultValue_.isArray())
        {
            return QJsonDocument(defaultValue_.toArray()).toJson(QJsonDocument::Compact);
        }

        return "";
    }
public:
    static bool ShowID;
private:
    QString id_;                                                                    //ID
    QString name_;                                                                  //名称(中文名)
    QString category_;                                                              //分类
    ComponentType componentType_ {ComponentType::Input};                            //设备类型
    QString team_;                                                                  //工作组
    QString role_;                                                                  //角色(脚本名称)
    BehaviorType behaviorType_{BehaviorType::EqualInput};                           //行为类型
    double actionKeep_{0};                                                          //置位信号保持时间(秒)
    QStringList subscription_;                                                      //订阅组件(订阅组件值改变会收到通知,如果订阅或引用了相同role的多个组件，则脚本变量全部传递id)
    QStringList reference_;                                                         //引用组件(引用组件值改变不会收到通知,如果订阅或引用了相同role的多个组件，则脚本变量全部传递id)
    QStringList respondBoardcast_;                                                  //响应的广播
    QString description_;                                                           //描述
    QJsonValue defaultValue_;                                                       //缺省值 缺省值等于 _calculate_default_value 表示该组件缺省值由计算产生
};

}

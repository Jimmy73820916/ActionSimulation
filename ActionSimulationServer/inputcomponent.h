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

#include "corecomponent.h"
#include <shared_mutex>

namespace Jimmy
{
/*
    输入设备通过接受外部输入改变自己的值,也可以通过一个订阅设备改变自己得值,但通过订阅改变自己的值时不发送信号.其行为类型只能是等于输入,输入设备的值改变后会推送到客户端
*/
class InputComponent : public CoreComponent
{
    Q_DISABLE_COPY(InputComponent)
public:
    InputComponent() = default;
    virtual ~InputComponent() = default;

    ErrorCode start() override;
    void stop() override;

    ComponentType getType() const override {  return ComponentType::Input; }
    BehaviorType getBehavior() const { return behaviorType_; }

    ErrorCode load(const QString& id,const QJsonObject& jo) override;

    QJsonValue getValue(User userid) override;
    void setValue(Connection connection,const QJsonValue& value) override;
    void onTime(User userid,size_t counter) override;
    void onAction(User userid,const QString& trigger,const QJsonValue& value) override;
    void onBoardcast(User /*userid*/) override {};
    void onLoop(User /*userid*/,const QJsonValue& /*value*/) override {};
    ErrorCode reloadRole(const QString& /*role*/) override { return Jimmy::ErrorCode::ec_ok; }

    void removeUser(User userid) override;

    QStringList getSubscription() const override { return subscription_;}
    QStringList getRespondBoardcast() const override { return QStringList(); }

    double getActionKeep() const { return actionKeep_; }
    void setActionKeep(double actionKeep) { actionKeep_ = actionKeep; }
private: 
    void setBehavior(BehaviorType behavior) { behaviorType_ = behavior; }
    void removeAllUser();
    std::shared_ptr<UserValue> getUserValue_(User userID,bool create_on_not_exist);

    void setSubscription(const QStringList& subscription) { subscription_ = subscription; subscription_.removeDuplicates();}
private:
    BehaviorType behaviorType_;
    double actionKeep_{0};                                                          //置位信号保持时间(秒)

    QStringList subscription_;                                                          //订阅组件(订阅组件值改变会收到通知)

    std::shared_mutex lockValue_;
    QHash<User,std::shared_ptr<UserValue>> userValue_;
};

}


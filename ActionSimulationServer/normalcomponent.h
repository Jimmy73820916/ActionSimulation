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

#include "commonstruct.h"
#include "corecomponent.h"
#include <shared_mutex>
#include <optional>


namespace Jimmy
{
/*
    NormalComponent 包括 ComponentType::Output 和 ComponentType::Internal 的实现 区别是 ComponentType::Output 在
    值发生变化时会推送给该 Userid 所有客户端,ComponentType::Internal 只会推送给该 Userid 的 UserRole 为 Administrator 的客户端
*/

class ActionScript;

class NormalComponent : public CoreComponent
{
    Q_DISABLE_COPY(NormalComponent)
public:
    NormalComponent() = default;
    virtual ~NormalComponent() = default;

    ComponentType getType() const override {  return componentType_; }
    BehaviorType getBehavior() const override { return behaviorType_; }

    ErrorCode start() override;
    void stop() override;
    ErrorCode reloadRole(const QString& role) override;

    ErrorCode load(const QString& id,const QJsonObject& jo) override;

    QJsonValue getValue(User userid) override;

    //仅用于调试
    void setValue(Connection connection,const QJsonValue& value) override;

    void onBoardcast(User userid) override;
    
    void onAction(User userid,const QString& trigger,const QJsonValue& value) override;
    void onTime(User userid,size_t counter,size_t interval) override;
    void onLoop(User userid,const QJsonValue& value) override;

    void removeUser(User userid) override;

    QString getRole() const { return (getBehavior() == BehaviorType::Script)?role_:""; }
    QStringList getSubscription() const override { return subscription_; }
    QStringList getReference() const { return reference_; }
    QStringList getRespondBoardcast() const override { return respondBoardcast_; }
private:
    void removeAllUser();
    void setType(const ComponentType& componentType)  { componentType_ = componentType; }
    void setRole(const QString& role)  { role_ = role; }
    void setSubscription(const QStringList& subscription) { subscription_ = subscription;subscription_.removeDuplicates();}
    void setReference(const QStringList& reference) { reference_ = reference;reference_.removeDuplicates();}
    void setRespondBoardcast(const QStringList& boardcast) { respondBoardcast_ = boardcast;}
    void setBehavior(BehaviorType behavior) { behaviorType_ = behavior; }

    bool sendAdminOnly();
    void stopScheduledTask(User userid);
private:
    ErrorCode onInitialize_();

    QJsonObject collectInputs();
    QJsonObject collectInputs(User userid,size_t counter, size_t interval);
    QJsonObject collectInputs(User userid,const QString& trigger,const QJsonValue& value);

    void getRelationParams(User userid,const QString& trigger,QJsonObject& jo);

    void analysisResult(User userid, const QJsonObject& result);

    std::shared_ptr<UserValue> getUserValue_(Jimmy::User userID,bool create_on_not_exist);
    void setValue_(User userid,const QJsonValue& value,bool enableSchedulePossible);

    void analysisLoop(User userid,const QJsonObject& jo);
    void analysisOrder(User userid,const QJsonObject& jo);
private:   
    ComponentType componentType_;
    BehaviorType behaviorType_;
    QString role_;

    QStringList subscription_;                                                      //订阅组件(订阅组件值改变会收到通知)
    QStringList reference_;                                                         //引用组件(引用组件值改变不会收到通知)
    QStringList respondBoardcast_;                                                  //响应的广播

    std::shared_mutex lockValue_;
    QHash<User,std::shared_ptr<UserValue>> userValue_;

    std::shared_ptr<ActionScript> actionScript_;
};

}



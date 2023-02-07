#pragma once

#include "corecomponent.h"
#include "commonstruct.h"
#include <shared_mutex>
#include <optional>

namespace Jimmy
{

/*
    TeamMasterComponent 没有缺省值,也没有值，不能被订阅，但可以订阅其他组件消息并更新相应 TeamSlaveComponent的值,TeamMasterComponent 脚本不支持 loop 和 order
*/

class ActionScript;

class TeamMasterComponent : public CoreComponent
{
public:
    TeamMasterComponent() = default;
    ~TeamMasterComponent() = default;

    ComponentType getType() const override {  return ComponentType::TeamMaster; }
    BehaviorType getBehavior() const { return BehaviorType::Script; }

    ErrorCode start() override;
    void stop() override;
    ErrorCode reloadRole(const QString& role) override;


    ErrorCode load(const QString& id,const QJsonObject& jo) override;

    QJsonValue getValue(User userid) override;

    //仅用于调试
    void setValue(Connection connection,const QJsonValue& value) override;

    void onBoardcast(User userid) override;
    void onAction(User userid,const QString& trigger,const QJsonValue& value) override;
    void onTime(User userid,size_t counter) override;
    void onLoop(User /*userid*/,const QJsonValue& /*value*/) override {};

    void removeUser(User userid) override;

    QString getRole() const { return role_; }
    QStringList getSubscription() const override { return subscription_; }
    QStringList getReference() const { return reference_; }
    QStringList getRespondBoardcast() const override { return respondBoardcast_; }
    QString getTeam() const { return team_;}

    QJsonValue getValue(User userid,const QString& slaveID);
    void setSlave(const QString& slaveID, CoreComponent* slave);
    QJsonValue getDefaultValue(const QString& slaveID);
private:
    ErrorCode onInitialize_();
    void removeAllUser();
    void setRole(const QString& role)  { role_ = role; }
    void setSubscription(const QStringList& subscription) { subscription_ = subscription; subscription_.removeDuplicates();}
    void setReference(const QStringList& reference) { reference_ = reference;reference_.removeDuplicates();}
    void setRespondBoardcast(const QStringList& boardcast) { respondBoardcast_ = boardcast;}
private:
    QString getAnswerValue(const QString& slaveI,const QJsonValue& value);
    void setTeam(const QString& team) { team_ = team;}
    QJsonObject collectInputs();
    QJsonObject collectInputs(User userid,size_t counter);
    QJsonObject collectInputs(User userid,const QString& trigger,const QJsonValue& value);

    std::shared_ptr<UserValue> getUserValue_(Jimmy::User userID,bool create_on_not_exist);

    void getRelationParams(User userid,const QString& trigger,QJsonObject& jo);

    void analysisResult(User userid, const QJsonObject& result);
    void setValue_(User userid,const QJsonObject& value);

    QHash<QString, QJsonValue> getSlavesDefaultValue();
private:
    QString role_;
    QString team_;

    QStringList subscription_;                                                      //订阅组件(订阅组件值改变会收到通知)
    QStringList reference_;                                                         //引用组件(引用组件值改变不会收到通知)
    QStringList respondBoardcast_;                                                  //响应的广播

    QHash<QString, CoreComponent*> slaves_;                                  //slaves_

    std::shared_mutex lockValue_;

    QHash<User,std::shared_ptr<UserValue>> userValue_;
    QHash<User,QHash<QString,QJsonValue>> slaveValue_;

    std::shared_ptr<ActionScript> actionScript_;
};


}


#pragma once

#include "corecomponent.h"

namespace Jimmy
{

/*
    TeamSlaveComponent 不能订阅和处理消息
*/

class TeamMasterComponent;
class TeamSlaveComponent : public CoreComponent
{
public:
    TeamSlaveComponent() = default;
    ~TeamSlaveComponent() = default;

    ComponentType getType() const override {  return ComponentType::TeamSlave; }
    BehaviorType getBehavior() const override { return BehaviorType::MasterSet; }
    ErrorCode start() override;
    void stop() override ;

    ErrorCode load(const QString& id,const QJsonObject& jo) override;

    QJsonValue getValue(User userid) override;

    //仅用于调试
    void setValue(Connection connection,const QJsonValue& value) override;

    void onTime(User /*userid*/,size_t /*counter*/) override {};
    void onAction(User /*userid*/,const QString& /*trigger*/,const QJsonValue& /*value*/) override {};
    void onBoardcast(User /*userid*/) override {};
    void onLoop(User /*userid*/,const QJsonValue& /*value*/) override {};
    void removeUser(User /*userid*/) override {};
    ErrorCode reloadRole(const QString& /*role*/) override { return Jimmy::ErrorCode::ec_ok; }

    QStringList getSubscription() const override { return QStringList(); }
    QStringList getRespondBoardcast() const override { return QStringList(); }

    QString getTeam() const { return team_;}
    void setMaster(std::shared_ptr<CoreComponent> parent) { parent_ = parent;}
private:
    void setTeam(const QString& team) { team_ = team;}
private:
    QString team_;
    std::shared_ptr<CoreComponent> parent_;
};

}


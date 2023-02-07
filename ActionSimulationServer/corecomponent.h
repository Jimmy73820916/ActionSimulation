#pragma once
#include <QString>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include "commonconst.h"
#include "commonstruct.h"
#include "errorcode.h"
#include <optional>

namespace Jimmy
{

struct UserValue
{
    QJsonValue          value;
    bool                schedulePossible{false};  //可能存在loop,order 事件
    QJsonValue          cache;
};

class CoreComponent
{
public:
    CoreComponent() = default;
    CoreComponent(const CoreComponent&) = default;
    CoreComponent(CoreComponent&&) = default;

    CoreComponent& operator=(const CoreComponent&) = default;
    CoreComponent& operator=(CoreComponent&&) = default;

    virtual ~CoreComponent() = default;

    QString getID() const { return id_; }


    virtual ErrorCode start() = 0;
    virtual void stop() = 0;
    virtual QJsonValue getValue(User userid) = 0;
    virtual void setValue(Connection connection,const QJsonValue& value) = 0;
    virtual ComponentType getType() const = 0;
    virtual BehaviorType getBehavior() const = 0;
    virtual ErrorCode load(const QString& id,const QJsonObject& jo) = 0;
    virtual void onTime(User userid,size_t counter) = 0;
    virtual void onAction(User userid,const QString& trigger,const QJsonValue& value) = 0;
    virtual void onBoardcast(User userid) = 0;
    virtual void onLoop(User userid,const QJsonValue& value) = 0;
    virtual void removeUser(User userid) = 0;
    virtual ErrorCode reloadRole(const QString& role) = 0;
    virtual QStringList getSubscription() const = 0;
    virtual QStringList getRespondBoardcast() const = 0;

    QString getAnswerValue(const QJsonValue& value);

    QJsonValue getDefaultValue() const { return defaultValue_; }
    void setDefaultValue(const QJsonValue& val) { defaultValue_ = val; }
protected:
    void setID(const QString& id) { id_ = id; }
    
    std::optional<ScheduledTask> analysisLoopbase(User userid,const QJsonObject& jo);
    std::optional<ScheduledTask> analysisOrderbase(User userid,const QJsonObject& jo);
    std::optional<ScheduledTask> analysisTimerbase(User userid,const QJsonObject& jo);

private:
    QString id_;   //ID
    QJsonValue defaultValue_;                                                       //缺省值
};


}



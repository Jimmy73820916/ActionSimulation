#include "teamslavecomponent.h"
#include "teammastercomponent.h"
#include "logger.h"

namespace Jimmy
{

ErrorCode TeamSlaveComponent::start()
{
    if(getID().isEmpty())
    {
        LOGERROR(QStringLiteral("[%1:%2] getID is empty")
            .arg(__FUNCTION__).arg(__LINE__));
        return ErrorCode::ec_error;
    }

    if (!parent_)
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 TeamMaster is unspecified")
            .arg(__FUNCTION__).arg(__LINE__).arg(getID()));
        return ErrorCode::ec_error;
    }

    static_cast<TeamMasterComponent*>(parent_.get())->setSlave(getID(),this);

    return ErrorCode::ec_ok;
}

void TeamSlaveComponent::stop()
{

}

void TeamSlaveComponent::setValue(Connection connection,const QJsonValue& value)
{
    if(parent_)
    {
        QJsonObject jo;
        jo.insert(getID(),value);
        parent_->setValue(connection,jo);
    }
}

ErrorCode TeamSlaveComponent::load(const QString& id,const QJsonObject& jo)
{
    setID(id);

    auto elemItor = jo.find("default_value");
    if(elemItor == jo.end())
    {
        LOGERROR(QStringLiteral("[%1:%2]component:%3 default_value is not exist")
            .arg(__FUNCTION__)
            .arg(__LINE__)
            .arg(getID()));

        return ErrorCode::ec_invalid_datatype;
    }
    setDefaultValue(*elemItor);

	elemItor = jo.find("team");
	if ((elemItor == jo.end())||!(*elemItor).isString())
	{
		LOGERROR(QStringLiteral("[%1:%2]component:%3 team is invalid")
			.arg(__FUNCTION__)
			.arg(__LINE__)
			.arg(getID()));

		return ErrorCode::ec_invalid_datatype;
	}
    setTeam(elemItor->toString());

    return ErrorCode::ec_ok;
}

QJsonValue TeamSlaveComponent::getValue(User userid)
{
    return static_cast<TeamMasterComponent*>(parent_.get())->getValue(userid,getID());
}


}


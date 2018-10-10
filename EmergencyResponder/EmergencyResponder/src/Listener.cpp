
#include "Listener.h"
#include "WildfireManager.h"

void Listener::onTitanEvent(const std::string& name, const Json::Value& params)
{
	if (name == "EResp::Begin")
	{
		wildfire->Begin();
	}
	else if (name == "EResp::DamageReport")
	{
		wildfire->CreateDamageReport();
	}
}
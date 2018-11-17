
#ifndef ER_LISTENER_H_
#define ER_LISTENER_H_

#include "TitanResources.h"
#include "FiretruckManager.h"

using namespace titan::api2;

class WildfireManager;

class Listener : public ITitanEventListener
{
private:

public:
	std::shared_ptr<WildfireManager> wildfire;
	std::shared_ptr<FiretruckManager> firetruck;

	void onTitanEvent(const std::string& name, const Json::Value& params) override;
};

#endif // ER_LISTENER_H_
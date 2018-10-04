
#ifndef ER_LISTENER_H_
#define ER_LISTENER_H_

#include <titan/plugin2/ITitanEventListener.h>

using namespace titan::api2;

class WildfireManager;

class Listener : public ITitanEventListener
{
private:

public:
	std::shared_ptr<WildfireManager> wildfire;

	void onTitanEvent(const std::string& name, const Json::Value& params) override;
};

#endif // ER_LISTENER_H_
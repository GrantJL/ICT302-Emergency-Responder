
#ifndef ER_PLUGIN_H_
#define ER_PLUGIN_H_

//#include "titan/plugin2/plugin.h"
#include "titan/plugin2/ITitan.h"

#include "Listener.h"
#include "WildfireManager.h"

#include <titan/plugin2/IEventManager.h>

//using namespace titan::api2;

const double STEP_INTERVAL = 1.0;

class EmergencyResponder 
{
private:
	std::shared_ptr<ITitan>				pluginApi;
	std::shared_ptr<IEventManager>		events;			//for events
	std::shared_ptr<IRenderManager>		renderer;		//Debug logging 
	std::shared_ptr<IScenarioManager>	scenario;		//Working with entities e.g. waypoints, static objects
	std::shared_ptr<IWorldManager>		world;			//surface position under a point and entity descriptor

	std::shared_ptr<Listener>			listener;
	std::shared_ptr<WildfireManager>	wildfire;

	double elapsedTime;

public:
	EmergencyResponder() : elapsedTime(0) { }

	void initialize(const std::shared_ptr<ITitan>& api) 
	{
		pluginApi = api;

		events = pluginApi->getEventManager();
		renderer = pluginApi->getRenderManager();
		scenario = pluginApi->getScenarioManager();
		world = pluginApi->getWorldManager();

		wildfire = std::make_shared<WildfireManager>(pluginApi);


		listener = std::make_shared<Listener>();
		listener->wildfire = wildfire;

		events->addTitanEventListener("EResp::Begin", listener);
	}

	void update(double dt) 
	{
		elapsedTime += dt;

		if (elapsedTime >= STEP_INTERVAL)
		{
			elapsedTime = 0;
			wildfire->Step();
		}
	}

	std::shared_ptr<IEventManager> getEventManager() const { return events; }
	std::shared_ptr<IRenderManager> getRenderManager() const { return renderer; }
	std::shared_ptr<IScenarioManager> getScenarioManager() const { return scenario; }
	std::shared_ptr<IWorldManager> getWorldManager() const { return world; }

};

#endif // ER_PLUGIN_H_
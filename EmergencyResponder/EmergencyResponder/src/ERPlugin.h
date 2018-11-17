
#ifndef ER_PLUGIN_H_
#define ER_PLUGIN_H_

#include <windows.h>

#include "TitanResources.h"

#include "Listener.h"
#include "WildfireManager.h"
#include "FiretruckManager.h"
#include "WildfireConfig.h"

const double STEP_INTERVAL = 1.0;

using namespace titan::api2;

class EmergencyResponder 
{
private:
	std::shared_ptr<ITitan>				titanApi;
	std::shared_ptr<IEventManager>		events;			//for events
	std::shared_ptr<IRenderManager>		renderer;		//Debug logging 
	std::shared_ptr<IScenarioManager>	scenario;		//Working with entities e.g. waypoints, static objects
	std::shared_ptr<IWorldManager>		world;			//surface position under a point and entity descriptor

	std::shared_ptr<Listener>			listener;
	std::shared_ptr<WildfireManager>	wildfire;
	std::shared_ptr<FiretruckManager>	firetruck;

	double elapsedTime;

public:
	EmergencyResponder() : elapsedTime(0) { }

	void initialize(const std::shared_ptr<ITitan>& api)
	{
		if (WildfireConfig::loadConfig(api->getUserDataDirectory() + "\\plugins\\EmergencyResponder\\Wildfire.json"))
			logtxt(api, "Config loaded succesfully");

		titanApi = api;

		events = titanApi->getEventManager();
		renderer = titanApi->getRenderManager();
		scenario = titanApi->getScenarioManager();
		world = titanApi->getWorldManager();

		wildfire = std::make_shared<WildfireManager>(titanApi);
		firetruck = std::make_shared<FiretruckManager>(titanApi);

		listener = std::make_shared<Listener>();
		listener->wildfire = wildfire;
		listener->firetruck = firetruck;

		events->addTitanEventListener("EResp::Begin", listener);
		events->addTitanEventListener("EResp::DamageReport", listener);
		events->addTitanEventListener("EResp::RequestFiretrucks", listener);
		events->addTitanEventListener("EResp::Dispatch", listener);
	}

	void update(double dt) 
	{
		elapsedTime += dt;

		if (firetruck->IsDispatching())
		{
			POINT point{ 0, 0 };
			Vec3d worldPosEcef;
			if (::GetCursorPos(&point))
			{
				HWND window = ::GetActiveWindow();
				ScreenToClient(window, &point);

				Point titanPoint(point.x, point.y);
				worldPosEcef = titanApi->getRenderManager()->getWorldPosition(titanPoint);

				firetruck->SetMousePosition(worldPosEcef);
				if (GetAsyncKeyState(VK_LBUTTON) < 0)
				{
					firetruck->DispatchFiretruck();
				}
			}
		}

		if (elapsedTime >= STEP_INTERVAL)
		{
			wildfire->Step(elapsedTime);

			elapsedTime = 0;
		}
	}

	std::shared_ptr<IEventManager> getEventManager() const { return events; }
	std::shared_ptr<IRenderManager> getRenderManager() const { return renderer; }
	std::shared_ptr<IScenarioManager> getScenarioManager() const { return scenario; }
	std::shared_ptr<IWorldManager> getWorldManager() const { return world; }

};

#endif // ER_PLUGIN_H_
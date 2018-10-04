
#ifndef ER_WILDFIRE_MANAGER_H_
#define ER_WILDFIRE_MANAGER_H_

#include <titan/plugin2/ITitan.h>X
#include <titan/plugin2/IRenderManager.h>
#include <titan/plugin2/IScenarioManager.h>
#include <titan/plugin2/IWorldManager.h>
#include <titan/plugin2/IEntity.h>

class WildfireManager
{
private:
	std::shared_ptr<IRenderManager> renderer;		//Debug logging 
	std::shared_ptr<IScenarioManager> scenario;
	std::shared_ptr<IWorldManager> world;

	Vec3d initialPosition;
	bool isInitialized;
public:

	WildfireManager(std::shared_ptr<ITitan> titanApi)
		: renderer(titanApi->getRenderManager()),
		  scenario(titanApi->getScenarioManager()),
		  world(titanApi->getWorldManager()),
		  isInitialized(false)
	{
	
	}

	void Begin()
	{
		renderer->debugLog("Begin");
		isInitialized = initializaPosition();
	}

	void Step()
	{
			renderer->debugLog("Step");
		if (isInitialized)
		{
			renderer->debugLog("Intialized Step");
			renderer->debugLog(std::to_string(initialPosition.x));
		}
	}

	bool initializaPosition()
	{
		std::set<std::shared_ptr<IEntity>> fireList;

		if (scenario)
		{// Get the wildfires in the scene
			fireList = scenario->getEntities(world->getEntityDescriptor("er_wildfire_control_object"));
		}

		auto fire = fireList.begin();
		if ( fire != fireList.end() )
		{
			initialPosition = (*fire)->getPosition();
			return true;
		}
		else
		{
			return false;
		}
	}
};

#endif // ER_WILDFIRE_MANAGER_H_
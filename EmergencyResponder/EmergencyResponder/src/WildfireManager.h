
#ifndef ER_WILDFIRE_MANAGER_H_
#define ER_WILDFIRE_MANAGER_H_

#include <titan/plugin2/ITitan.h>
#include <titan/plugin2/IRenderManager.h>
#include <titan/plugin2/IScenarioManager.h>
#include <titan/plugin2/IWorldManager.h>
#include <titan/plugin2/IEntity.h>

#include "Utility.h"
#include "Fire.h"

class WildfireManager
{
private:
	std::shared_ptr<ITitan> titanApi;

	/// Entity UUID, Entity last recorded health
	std::map<std::string, double> damagedEntities;

	Vec3d initialPosition;
	bool isInitialized;
	
	std::shared_ptr<IEntity> controlEntity;
	std::shared_ptr<Fire> fireOrigin;		// to be replaced with fire data structure

public:

	WildfireManager(std::shared_ptr<ITitan> api)
		: titanApi(api),
		  isInitialized(false)
	{
	
	}

	void Begin()
	{
		//titanApi->getRenderManager()->debugLog("Begin");
		isInitialized = initializePosition();
	}

	void Step(double dt)
	{
		//titanApi->getRenderManager()->debugLog("Step");
		if (isInitialized)
		{
			if (dt > 0)
			{
				//titanApi->getRenderManager()->debugLog("Intialized Step");
				//titanApi->getRenderManager()->debugLog(std::to_string(initialPosition.x));
				fireOrigin->step(dt, damagedEntities);
			}
		}
	}

	void CreateDamageReport()
	{
		logtxt(titanApi) << "Damage Report" << std::endl;
		for (auto it = damagedEntities.begin(); it != damagedEntities.end(); it++)
		{
			logtxt(titanApi) << (*it).first << "  " << (*it).second << std::endl;
		}
		logtxt(titanApi) << "END Damage Report" << std::endl;
	}

private:

	bool initializePosition()
	{
		std::set<std::shared_ptr<IEntity>> fireList;

		fireList = titanApi->getScenarioManager()->getEntities(titanApi->getWorldManager()->getEntityDescriptor("er_wildfire_control_object"));

		auto fire = fireList.begin();
		if ( fire != fireList.end())
		{
			controlEntity = (*fire);
			initialPosition = controlEntity->getPosition();
			
			fireOrigin = std::make_shared<Fire>(titanApi, initialPosition);

			return true;
		}
		else
		{
			return false;
		}
	}
};

#endif // ER_WILDFIRE_MANAGER_H_
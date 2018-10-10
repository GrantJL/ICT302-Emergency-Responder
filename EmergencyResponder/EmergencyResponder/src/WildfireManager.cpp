
#include "WildfireManager.h"

WildfireManager::WildfireManager(std::shared_ptr<ITitan> api) : titanApi(api), isInitialized(false)
{

}

void WildfireManager::Begin()
{
	isInitialized = initializePosition();
}

void WildfireManager::Step(double dt)
{
	if (isInitialized)
	{
		if (dt > 0)
		{
			fireOrigin->step(dt, damagedEntities);
		}
	}
}

void WildfireManager::CreateDamageReport()
{
	logtxt(titanApi, "Damage Report");

	for (auto it = damagedEntities.begin(); it != damagedEntities.end(); it++)
	{
		logtxt(titanApi, "" + it->first + "  " + std::to_string(it->second));
	}

	logtxt(titanApi, "END Damage Report");
}

bool WildfireManager::initializePosition()
{
	std::set<std::shared_ptr<IEntity>> fireList;

	fireList = titanApi->getScenarioManager()->getEntities(titanApi->getWorldManager()->getEntityDescriptor("er_wildfire_control_object"));

	auto fire = fireList.begin();
	if (fire != fireList.end())
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

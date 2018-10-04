#include "Fire.h"

#include <titan/plugin2/IEntity.h>
#include <titan/plugin2/ITitan.h>
#include <titan/plugin2/IScenarioManager.h>
#include <titan/plugin2/IDamageModel.h>
#include <titan/plugin2/IWorldManager.h>

#include <titan/plugin2/util/MathHelpers.h>


Fire::Fire(std::shared_ptr<titan::api2::ITitan> api, const titan::api2::Vec3d& position)
	: titanApi(api)
{
	titan::api2::EntityDescriptor descriptor = titanApi->getWorldManager()->getEntityDescriptor("small_wildfire");

	titan::api2::Quat quat;
	titan::api2::util::MathHelpers::createGroundAlignedQuaternion(titan::api2::Vec3d(1, 0, 0), position, quat);

	entity = titanApi->getScenarioManager()->createEntity(descriptor, position, quat);
	fuel = 30;
}

void Fire::setFuel(const double fuelValue)
{
	fuel = fuelValue;
}

void Fire::setBurning(const bool burningValue)
{
	burning = burningValue;
}

double Fire::getFuel()
{
	return fuel;
}

bool Fire::isBurning()
{
	return burning;
}

void Fire::step(const double dt)
{
	damageEntitiesAtFireLocation();
	if (fuel > 0)
	{
	}
}

void Fire::damageEntitiesAtFireLocation()
{
	// TODO: Report damage done

	titan::api2::Vec3d pos = entity->getPosition();
	std::set<std::shared_ptr<titan::api2::IEntity>> entities = titanApi->getScenarioManager()->getEntities(pos, radius);
	std::set<std::shared_ptr<titan::api2::IEntity>>::iterator entityIterator;

	for (entityIterator = entities.begin(); entityIterator != entities.end(); entityIterator++)
	{
		std::shared_ptr<titan::api2::IDamageModel> damageModel = (*entityIterator)->getDamageModel();

		if (!damageModel)
		{//Entity cannot be damaged
			continue;
		}
		else if (damageModel->getHealthNormalized() > 0)
		{
			damageModel->setHealthNormalized(damageModel->getHealthNormalized() - 0.2);
		}
	}
}
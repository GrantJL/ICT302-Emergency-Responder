#include "Fire.h"

Fire::Fire()
{
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
	if (fuel > 0)
	{
		// Reduce fuel
		// Reduce health of entities.
	}
}

void Fire::damageEntitiesAtFireLocation(std::shared_ptr<titan::api2::IEntity> fire)
{
	std::set<std::shared_ptr<titan::api2::IEntity>> entities = scenario->getEntities(fire->getPosition(), radius);
	std::set<std::shared_ptr<titan::api2::IEntity>>::iterator entityIterator;
	for (entityIterator = entities.begin(); entityIterator != entities.end(); entityIterator++)
	{
		std::shared_ptr<titan::api2::IDamageModel> damageModel = (*entityIterator)->getDamageModel();
		if(damageModel->getHealthNormalized() > 0)
			damageModel->setHealthNormalized(damageModel->getHealthNormalized() - 0.01);
	}
}
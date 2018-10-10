#include "Fire.h"

#include <math.h>

#include "TitanResources.h"
#include "Utility.h"

double Fire::vehicleModifier = 0.5;
double Fire::buildingModifier = 0.25;
double Fire::maxFuel = 600;

Fire::Fire(std::shared_ptr<ITitan> api, const Vec3d& position)
	: titanApi(api), burning(true)
{
	EntityDescriptor descriptor = titanApi->getWorldManager()->getEntityDescriptor("er_large_wildfire");

	util::MathHelpers::createGroundAlignedQuaternion(Vec3d(1, 0, 0), position, fireRotation);

	fireEntity = titanApi->getScenarioManager()->createEntity(descriptor, position, fireRotation);
	fuel = maxFuel;
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

void Fire::step(const double dt, std::map<std::string, double>& damagedEntities)
{
	if (fuel > 0)
	{
		double fuelPercent = (maxFuel - fuel) / maxFuel;
		fuelFactor = sin(fuelPercent * (0.8 * M_PI) + (0.1 * M_PI));

		damageEntitiesAtFireLocation(dt, damagedEntities);

		fuel -= (60 * dt);
	}
	else if (burning)
	{
		burning = false;

		titanApi->getScenarioManager()->removeEntity(fireEntity);

		EntityDescriptor descriptor = titanApi->getWorldManager()->getEntityDescriptor("er_large_wildfire_burnt");
		fireEntity = titanApi->getScenarioManager()->createEntity(descriptor, firePosition, fireRotation);
	}
}

void Fire::damageEntitiesAtFireLocation(double dt, std::map<std::string, double>& damagedEntities)
{
	// Get entities in the radius of the fire.
	Vec3d pos = fireEntity->getPosition();
	std::set<std::shared_ptr<IEntity>> entities = titanApi->getScenarioManager()->getEntities(pos, radius);
	std::set<std::shared_ptr<IEntity>>::iterator entityIterator;

	for (entityIterator = entities.begin(); entityIterator != entities.end(); entityIterator++)
	{
		std::shared_ptr<IEntity> entity = (*entityIterator);
		std::shared_ptr<IDamageModel> damageModel = entity->getDamageModel();

		// When the entity has a damage model AND still has health
		if (damageModel && damageModel->getHealthNormalized() > 0)
		{
			// Calculate damage based on distance (linear 1.0 - 0.0)
			Vec3d diff = util::MathHelpers::subtract(entity->getPosition(), pos);
			double distance = util::MathHelpers::magnitude(diff);
			double distFactor = (radius - distance) / radius;

			// Calculate the damage to deal
			double calculatedDamage = dt * damage * distFactor * fuelFactor;

			// Apply modifier if entity is a vehicle of building
			if (entity->isA(EntityCategory::EntityVehicle))
				calculatedDamage *= vehicleModifier;
			else if (entity->getDescriptor().blueprint.find("building"))
				calculatedDamage *= buildingModifier;

			// Apply the damage to the entity
			damageModel->setHealthNormalized(damageModel->getHealthNormalized() - calculatedDamage/*0.2*/);

			//logtxt(titanApi) << entity->getUuid() << " " << damageModel->getHealthNormalized() << std::endl;

			damagedEntities[entity->getUuid()] = damageModel->getHealthNormalized();
		}
	}
}
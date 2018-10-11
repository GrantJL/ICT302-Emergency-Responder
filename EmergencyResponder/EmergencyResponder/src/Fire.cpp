#include "Fire.h"

#include <math.h>

#include "TitanResources.h"
#include "Utility.h"

#include "WildfireManager.h"

double Fire::vehicleModifier = 0.5;
double Fire::buildingModifier = 0.25;
double Fire::maxFuel = 600;

std::vector<titan::api2::Vec3d> Fire::globalFires;
Fire::Fire(std::shared_ptr<ITitan> api, const Vec3d& position)
	: titanApi(api), burning(true), fuel(maxFuel)
{
	EntityDescriptor descriptor = titanApi->getWorldManager()->getEntityDescriptor("er_large_wildfire");

	firePosition = position;

	titan::api2::Quat quat;
	titan::api2::util::MathHelpers::createGroundAlignedQuaternion(titan::api2::Vec3d(1, 0, 0), firePosition, quat);

	fireEntity = titanApi->getScenarioManager()->createEntity(descriptor, titanApi->getWorldManager()->getNearestSurfacePositionBelow(firePosition), quat);
	// fuel = maxFuel;
	// TODO: may need to rethink this. ??Surface * 100, TreeDense * 500??
	fuel = maxFuel * getTreeDensity(titanApi, firePosition) * getSurfaceCombustion(titanApi, firePosition);
	titanApi->getRenderManager()->debugLog("Fire fuel set to " + std::to_string(fuel));
	burning = true;

	util::MathHelpers::createGroundAlignedQuaternion(Vec3d(1, 0, 0), firePosition, fireRotation);

	fireEntity = titanApi->getScenarioManager()->createEntity(descriptor, firePosition, fireRotation);
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
	static int fireCounter = 1;
	if (fuel > 0)
	{
		double fuelPercent = (maxFuel - fuel) / maxFuel;
		// Calculate strength of fire (Strongest half way through its burn).
		fuelFactor = sin(fuelPercent * (0.8 * M_PI) + (0.1 * M_PI));
		fuel -= dt;

		//titanApi->getRenderManager()->debugLog("Fuel at: " + std::to_string(fuel));

		// Damage entites in radius of the fire, add the damaged entites to damagedEntities 
		damageEntitiesAtFireLocation(dt, damagedEntities);

		if (fireCounter < MAX_FIRES && willPropagate())
		{
			if (willPropagate(WildfireManager::getPropagationProb().at(N)) && !propped[N])
			{
				//titanApi->getRenderManager()->debugLog("Proped north");
				Vec3d pos = titan::api2::util::MathHelpers::ecefToLla(firePosition);
				pos.x += 0.0000005;
				pos = titan::api2::util::MathHelpers::llaToEcef(pos);
				if (!fireAtPosition(pos))
				{
					Fire f(titanApi, pos);
					children.push_back(f);
					propped[N] = true;
					fireCounter++;
					globalFires.push_back(pos);
				}
			}
			if (willPropagate(WildfireManager::getPropagationProb().at(E)) && !propped[E])
			{
				//titanApi->getRenderManager()->debugLog("Proped east");
				Vec3d pos = titan::api2::util::MathHelpers::ecefToLla(firePosition);
				pos.y += 0.0000005;
				pos = titan::api2::util::MathHelpers::llaToEcef(pos);
				if (!fireAtPosition(pos))
				{
					Fire f(titanApi, pos);
					children.push_back(f);
					propped[E] = true;
					fireCounter++;
					globalFires.push_back(pos);
				}
			}
			if (willPropagate(WildfireManager::getPropagationProb().at(S)) && !propped[S])
			{
				//titanApi->getRenderManager()->debugLog("Proped south");
				Vec3d pos = titan::api2::util::MathHelpers::ecefToLla(firePosition);
				pos.x -= 0.0000005;
				pos = titan::api2::util::MathHelpers::llaToEcef(pos);
				if (!fireAtPosition(pos))
				{
					Fire f(titanApi, pos);
					children.push_back(f);
					propped[S] = true;
					fireCounter++;
					globalFires.push_back(pos);
				}
			}
			if (willPropagate(WildfireManager::getPropagationProb().at(W)) && !propped[W])
			{
				//titanApi->getRenderManager()->debugLog("Proped west");
				Vec3d pos = titan::api2::util::MathHelpers::ecefToLla(firePosition);
				pos.y -= 0.0000005;
				pos = titan::api2::util::MathHelpers::llaToEcef(pos);
				if (!fireAtPosition(pos))
				{
					Fire f(titanApi, pos);
					children.push_back(f);
					propped[W] = true;
					fireCounter++;
					globalFires.push_back(pos);
				}
			}
			
		}
	}
	else if (burning)
	{
		burning = false;

		titanApi->getScenarioManager()->removeEntity(fireEntity);
		fireCounter--;

		//EntityDescriptor descriptor = titanApi->getWorldManager()->getEntityDescriptor("er_large_wildfire_burnt");
		//fireEntity = titanApi->getScenarioManager()->createEntity(descriptor, firePosition, fireRotation);
	}
	// Step children
	std::vector<Fire>::iterator child;
	for (child = children.begin(); child != children.end(); child++)
	{
		child->step(dt, damagedEntities);
	}
	titanApi->getRenderManager()->debugLog("Fire Count: " + std::to_string(fireCounter));
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

			// Apply modifier if entity is a vehicle or building
			if (entity->isA(EntityCategory::EntityVehicle))
				calculatedDamage *= vehicleModifier;
			else if (entity->getDescriptor().blueprint.find("building"))
				calculatedDamage *= buildingModifier;

			// Apply the damage to the entity
			damageModel->setHealthNormalized(damageModel->getHealthNormalized() - calculatedDamage/*0.2*/);

			// TODO: Include blueprint in damage report if the entity is not already in the list.
			// Add to/update the list of entities damaged by the fire.
			damagedEntities[entity->getUuid()] = damageModel->getHealthNormalized();
		}
	}
}

bool Fire::willPropagate()
{
	return willPropagate(0.1);
}

bool Fire::willPropagate(const double percent)
{
	double value = rand() / ((double) RAND_MAX);
	logtxt(titanApi, "Rand: " + std::to_string(value) + "\n");
	if (value < percent)
		return true;
	return false;
}

bool Fire::fireAtPosition(const titan::api2::Vec3d position)
{
	std::vector<titan::api2::Vec3d>::iterator it;
	for (it = globalFires.begin(); it != globalFires.end(); it++)
	{
		if (compareVec3d(*it, position))
		{
			titanApi->getRenderManager()->debugLog("Wildfire at location");
			return true;
		}
	}
	return false;
}

bool Fire::compareVec3d(const titan::api2::Vec3d & vec1, const titan::api2::Vec3d & vec2)
{
	if ((int)vec1.x != (int)vec2.x)
		return false;
	if ((int)vec1.y != (int)vec2.y)
		return false;
	if ((int)vec1.z != (int)vec2.z)
		return false;
	return true;
}
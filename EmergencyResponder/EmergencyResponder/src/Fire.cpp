#include "Fire.h"

#include <math.h>

#include <titan/plugin2/IEntity.h>
#include <titan/plugin2/ITitan.h>
#include <titan/plugin2/IScenarioManager.h>
#include <titan/plugin2/IDamageModel.h>
#include <titan/plugin2/IWorldManager.h>

#include <titan/plugin2/util/MathHelpers.h>

#include "Utility.h"

double Fire::vehicleModifier = 0.5;
double Fire::buildingModifier = 0.25;
double Fire::maxFuel = 600;

std::vector<titan::api2::Vec3d> Fire::globalFires;

Fire::Fire(std::shared_ptr<titan::api2::ITitan> api, const titan::api2::Vec3d& newPosition)
	: titanApi(api)
{
	titan::api2::EntityDescriptor descriptor = titanApi->getWorldManager()->getEntityDescriptor("small_wildfire");

	titan::api2::Quat quat;
	titan::api2::util::MathHelpers::createGroundAlignedQuaternion(titan::api2::Vec3d(1, 0, 0), newPosition, quat);

	fireEntity = titanApi->getScenarioManager()->createEntity(descriptor, titanApi->getWorldManager()->getNearestSurfacePositionBelow(newPosition), quat);
	fuel = maxFuel * getTreeDensity(titanApi, newPosition) * getSurfaceCombustion(titanApi, newPosition);
	position = newPosition;
	titanApi->getRenderManager()->debugLog("Fire fuel set to " + std::to_string(fuel));
	burning = true;
	removed = false;
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
	if (fuel <= 0)
		burning = false;
	if (burning)
	{
		double fuelPercent = (maxFuel - fuel) / maxFuel;
		fuelFactor = sin(fuelPercent * (0.8 * M_PI) + (0.1 * M_PI));
		fuel -= dt;

		//titanApi->getRenderManager()->debugLog("Fuel at: " + std::to_string(fuel));

		damageEntitiesAtFireLocation(dt, damagedEntities);
		if (fireCounter < MAX_FIRES && willPropagate())
		{
			if (willPropagate(CONST_PROB[N]) && !propped[N])
			{
				//titanApi->getRenderManager()->debugLog("Proped north");
				Vec3d pos = titan::api2::util::MathHelpers::ecefToLla(position);
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
			if (willPropagate(CONST_PROB[E]) && !propped[E])
			{
				//titanApi->getRenderManager()->debugLog("Proped east");
				Vec3d pos = titan::api2::util::MathHelpers::ecefToLla(position);
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
			if (willPropagate(CONST_PROB[S]) && !propped[S])
			{
				//titanApi->getRenderManager()->debugLog("Proped south");
				Vec3d pos = titan::api2::util::MathHelpers::ecefToLla(position);
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
			if (willPropagate(CONST_PROB[W]) && !propped[W])
			{
				//titanApi->getRenderManager()->debugLog("Proped west");
				Vec3d pos = titan::api2::util::MathHelpers::ecefToLla(position);
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
	else
	{
		if (!removed)
		{
			titanApi->getScenarioManager()->removeEntity(fireEntity);
			fireCounter--;
			removed = true;
		}
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
	titan::api2::Vec3d pos = fireEntity->getPosition();
	std::set<std::shared_ptr<titan::api2::IEntity>> entities = titanApi->getScenarioManager()->getEntities(pos, radius);
	std::set<std::shared_ptr<titan::api2::IEntity>>::iterator entityIterator;

	for (entityIterator = entities.begin(); entityIterator != entities.end(); entityIterator++)
	{
		std::shared_ptr<titan::api2::IEntity> entity = (*entityIterator);
		std::shared_ptr<titan::api2::IDamageModel> damageModel = entity->getDamageModel();

		// When the entity has a damage model AND still has health
		if (damageModel && damageModel->getHealthNormalized() > 0)
		{
			// Calculate damage based on distance (linear 1.0 - 0.0)
			titan::api2::Vec3d diff = titan::api2::util::MathHelpers::subtract(entity->getPosition(), pos);
			double distance = titan::api2::util::MathHelpers::magnitude(diff);
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

			logtxt(titanApi) << entity->getUuid() << " " << damageModel->getHealthNormalized() << std::endl;

			damagedEntities[entity->getUuid()] = damageModel->getHealthNormalized();
		}
	}
}

bool Fire::willPropagate()
{
	return willPropagate(10);
}

bool Fire::willPropagate(const double percent)
{
	int value = rand() % 100;
	if (value < percent)
		return true;
	return false;
}

bool Fire::fireAtPosition(const titan::api2::Vec3d position)
{
	/*std::set<std::shared_ptr<titan::api2::IEntity>> entities = titanApi->getScenarioManager()->getEntities(position, radius);
	std::set<std::shared_ptr<titan::api2::IEntity>>::iterator entityIterator;
	for (entityIterator = entities.begin(); entityIterator != entities.end(); entityIterator++)
	{
		std::shared_ptr<titan::api2::IEntity> entity = (*entityIterator);
		if (entity->getDescriptor().path == "effects/graphEffects/fire")
		{
			titanApi->getRenderManager()->debugLog("Wildfire at location");
			return true;
		}
	}
	return false;*/
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
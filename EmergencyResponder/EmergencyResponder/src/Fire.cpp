#include "Fire.h"

#include <math.h>

#include "TitanResources.h"
#include "Utility.h"

#include "WildfireManager.h"

#include <titan/plugin2/IStaticObject.h>

//double Fire::vehicleModifier = 0.5;		// The damage percentage vehivles take.
//double Fire::buildingModifier = 0.25;	// The damage percentage buildings take.
//double Fire::maxFuel = 600;				// Maximum possible fuel (10 minutes, assuming burn rate of 1/s)
//double Fire::fireGap = 0.000002;		// Gap between fires in degrees(longitude)

// the 8 posible spread directions (N, NE, E, SE, ...)
const std::vector<Position> Fire::spreadDirections = {
	{1, 0},
	{1, 1},
	{0, 1},
	{-1, 1},
	{-1, 0},
	{-1, -1},
	{0, -1},
	{1, -1}
};

std::vector<Position> Fire::globalFires;	// Positions at which a fire has been created
std::set<std::shared_ptr<Fire>> Fire::activeFires;			// Collection of currently burning(active) fires.

double getRandom();


Fire::Fire(std::shared_ptr<ITitan> api, const Vec3d& realPos, const Position& basicPos)
	: titanApi(api), burning(true), fuel(WildfireConfig::maxFuel), realPosition(realPos), basicPosition(basicPos)
{
	// Get the entity descriptor
	EntityDescriptor descriptor = titanApi->getWorldManager()->getEntityDescriptor("er_large_wildfire");

	// Calculate rotation (probably unnecessary for the fire effect.
	titan::api2::Quat quat;
	titan::api2::util::MathHelpers::createGroundAlignedQuaternion(titan::api2::Vec3d(1, 0, 0), realPosition, quat);

	// Create fire effect at ground level.
	realPosition = titanApi->getWorldManager()->getNearestSurfacePositionBelow(realPosition);
	fireEntity = titanApi->getScenarioManager()->createEntity(descriptor, realPosition, quat);

	// Initialize the fire attributes.
	// how much fuel is available to the fire
	availableFuel = (0.65 * WildfireConfig::maxFuel * getTreeDensity(titanApi, realPosition)) + (0.35 * WildfireConfig::maxFuel * getSurfaceCombustion(titanApi, realPosition));
	// The current remaining fuel
	fuel = availableFuel;
}

Fire::~Fire()
{
	if (fireEntity != nullptr)
	{
		titanApi->getScenarioManager()->removeEntity(fireEntity);
		fireEntity = nullptr;
	}
	titanApi = nullptr;
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
		double fuelPercent = (availableFuel - fuel) / availableFuel;
		// Calculate strength of fire (Strongest half way through its burn).
		if (true) //(SIMPLE_FUEL_FUNCTIION)
		{	// This may improve performance
			if (fuelPercent < 0.5)
				fuelFactor = (1.4 * fuelPercent) + 0.3;
			else
				fuelFactor = (1.4 * (1 - fuelPercent)) + 0.3;
		}
		else
		{
			fuelFactor = sin(fuelPercent * (0.8 * M_PI) + (0.1 * M_PI));
		}
		fuel -= dt;

		reduceFuelForFiretrucksAtFireLocation(dt);
		reduceFuelForWeather(dt);

		//titanApi->getRenderManager()->debugLog("Fuel at: " + std::to_string(fuel));

		// Damage entites in radius of the fire, add the damaged entites to damagedEntities 
		damageEntitiesAtFireLocation(dt, damagedEntities);

		// TODO: max fires not calculated correctly?
		// Fires may not be getting removed correctly
		if (/*fireCounter < MAX_FIRES && */willPropagate())
		{


			double directionProbability = getRandom();
			// For each direction of fire spread
			for (size_t f = 0; f < 8; f++)
			{
				if (directionProbability < WildfireManager::getPropagationProb().at(f))
				{
					if ( !propped[f])
					{
						// Basic position relative to origin fire (0, 0) e.g. (0, 1), (-2, 3)
						Position newBasic = basicPosition;
						// Get the new spread position
						newBasic.x += spreadDirections[f].x;
						newBasic.y += spreadDirections[f].y;
						// Spawn a new fire if one isnt at that position.
						if (!fireAtPosition(newBasic))
						{
							// Calculate the real position of the new fire
							Vec3d newReal = calculateRelativePosition(realPosition, spreadDirections[f]);
							// create the new fire
							std::shared_ptr<Fire> newFire = std::make_shared<Fire>(titanApi, newReal, newBasic);
							// Add fire to our children
							// TODO: Improve this
							activeFires.insert(newFire);
							// Is there a better way to check if a fire can be spawned
							propped[f] = true;
							fireCounter++;
							// Add the basic fire position to the list
							globalFires.push_back(newBasic);
						}
					} // END if can propagate in direction
					break; // We only propagate once per tick
				}
				// If can propagate in that direction
				// TODO: probability can be improved, dont need to calculate a random number 9 times per fire/ per tick.
			}// END for each spread direction
		}
	}
	else if (burning)
	{ // If fire is still burning, but has no fuel.
		burning = false;

		titanApi->getScenarioManager()->removeEntity(fireEntity);
		fireCounter--;
		fireEntity = nullptr;

		// Currently have no way to represent burn ground
		// Not create the entity for efficiency
			//EntityDescriptor descriptor = titanApi->getWorldManager()->getEntityDescriptor("er_large_wildfire_burnt");
			//fireEntity = titanApi->getScenarioManager()->createEntity(descriptor, realPosition, fireRotation);
	}
	// TODO: Remove
	// Step children
	//std::vector<Fire>::iterator child;
	//for (child = children.begin(); child != children.end(); child++)
	//{
	//	child->step(dt, damagedEntities);
	//}
	//titanApi->getRenderManager()->debugLog("Fire Count: " + std::to_string(fireCounter));
}

void Fire::damageEntitiesAtFireLocation(double dt, std::map<std::string, double>& damagedEntities)
{
	// Get entities in the radius of the fire.
	Vec3d pos = fireEntity->getPosition();

	{
		std::set<std::shared_ptr<IEntity>> entities = titanApi->getScenarioManager()->getEntities(pos, WildfireConfig::baseDamageRadius);
		std::set<std::shared_ptr<IEntity>>::iterator entityIterator;

		for (entityIterator = entities.begin(); entityIterator != entities.end(); entityIterator++)
		{
			std::shared_ptr<IEntity> entity = (*entityIterator);
			damageEntity(dt, damagedEntities, entity);
		}
	}
	{
		// Using the StaticObjects default name as a key, as they do not have a UUID.
		// We are decrementing the value at this key, this will allow us to recognise
		// objects which do not store a damage model.
		// When calculating the damage report we will assume that damaged entities
		// with a negative health value are static objects, specifically buildings
		// monetary value.


		std::set<std::shared_ptr<IStaticObject>> entities = titanApi->getScenarioManager()->getStaticObjects(pos, WildfireConfig::baseDamageRadius);
		std::set<std::shared_ptr<IStaticObject>>::iterator entityIterator;

		for (entityIterator = entities.begin(); entityIterator != entities.end(); entityIterator++)
		{
			estimateBuildingDamage(dt, damagedEntities, (*entityIterator));
		}
	}
}


void Fire::damageEntity(double dt, std::map<std::string, double>& damagedEntities, std::shared_ptr<IEntity> entity)
{
	std::shared_ptr<IDamageModel> damageModel = entity->getDamageModel();

	// When the entity has a damage model AND still has health
	if (damageModel && damageModel->getHealthNormalized() > 0)
	{
		// Calculate damage based on distance (linear 1.0 - 0.0)
		Vec3d diff = util::MathHelpers::subtract(entity->getPosition(), fireEntity->getPosition());
		double distance = util::MathHelpers::magnitude(diff);
		double distFactor = (WildfireConfig::baseDamageRadius - distance) / WildfireConfig::baseDamageRadius;

		// TODO: Consider dealing less damage if the fire doesn't have much fuel (availableFuel).
		//		 Should a grassland fire deal as much damage as a forrest fire.?
		// Calculate the damage to deal
		double calculatedDamage = dt * WildfireConfig::baseDamage * distFactor * fuelFactor;

		// Apply modifier if entity is a vehicle or building
		if (entity->isA(EntityCategory::EntityVehicle))
			calculatedDamage *= WildfireConfig::vehicleDamageModifier;
		else if (entity->getDescriptor().blueprint.find("building"))
			calculatedDamage *= WildfireConfig::buildingDamageModifier;

		// Apply the damage to the entity
		damageModel->setHealthNormalized(damageModel->getHealthNormalized() - calculatedDamage);

		// TODO: Include blueprint in damage report if the entity is not already in the list.
		// Add to/update the list of entities damaged by the fire.
		damagedEntities[entity->getUuid()] = damageModel->getHealthNormalized();
	}
}

void Fire::reduceFuelForFiretrucksAtFireLocation(double dt)
{
	Vec3d pos = fireEntity->getPosition();
	std::set<std::shared_ptr<IEntity>> entities = titanApi->getScenarioManager()->getEntities(pos, firetruckRadius);
	std::set<std::shared_ptr<titan::api2::IEntity>>::iterator entityIter;
	for (entityIter = entities.begin(); entityIter != entities.end(); entityIter++)
	{
		if ((*entityIter)->getDescriptor().defaultEntityName == "Firetruck")
		{
			fuel -= firetruckFuelReduction * dt;
		}
	}
}

void Fire::reduceFuelForWeather(double dt)
{
	double rainVal = titanApi->getWorldManager()->getWeatherData(titan::api2::Vec3d(0, 0, 0)).rainDensity;
	double snowVal = titanApi->getWorldManager()->getWeatherData(titan::api2::Vec3d(0, 0, 0)).snowDensity;
	fuel -= rainFuelDampValue * rainVal * dt;
	fuel -= snowFuelDampValue * snowVal * dt;
}

void Fire::estimateBuildingDamage(double dt, std::map<std::string, double>& damagedEntities, std::shared_ptr<IStaticObject> building)
{
	if (building->getDescriptor().defaultEntityName.size() != 0)
	{
		// Calculate the damage to deal
		double calculatedDamage = dt * WildfireConfig::baseDamage * fuelFactor * WildfireConfig::buildingDamageModifier;

		// Add estimated damage
		// We will use a negative value to distinguish between regular entities and buildings.
		// TODO: Create a damage reporting model that better supports StaticObjects and their lack of UUIDs.
		damagedEntities[building->getDescriptor().defaultEntityName] -= calculatedDamage;
	}

}

double getRandom()
{
	return  rand() / ((double)RAND_MAX);
}

bool Fire::willPropagate()
{
	// Each fire has a 5% chance of propagating per tick
	return willPropagate(0.05);
}

bool Fire::willPropagate(const double percent)
{
	double value = getRandom();
	if (value < percent)
		return true;
	return false;
}


bool Fire::fireAtPosition(const Position position)
{
	std::vector<Position>::iterator it;
	for (it = globalFires.begin(); it != globalFires.end(); it++)
	{
		if (compatePositions(*it, position))
		{
			return true;
		}
	}
	return false;
}

bool Fire::compatePositions(const Position& posA, const Position& posB)
{
	if (posA.x != posB.x)
		return false;
	if (posA.y != posB.y)
		return false;
	return true;
}

Vec3d Fire::calculateRelativePosition(const titan::api2::Vec3d& parentPos, const Position& position)
{
	Vec3d newPosition(parentPos);

	newPosition = util::MathHelpers::ecefToLla(newPosition);

	newPosition.x += position.x * WildfireConfig::fireGap;
	newPosition.y += position.y * WildfireConfig::fireGap;

	return util::MathHelpers::llaToEcef(newPosition);
}
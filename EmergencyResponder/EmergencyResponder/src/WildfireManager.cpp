
#include "WildfireManager.h"

std::vector<double> WildfireManager::propagationProb(8);

WildfireManager::WildfireManager(std::shared_ptr<ITitan> api) 
	: titanApi(api), isInitialized(false)
{

}

void WildfireManager::Reset()
{
	Fire::reset();
	// Delete all active fires.
	for (auto fireIt = Fire::activeFires.begin(); fireIt != Fire::activeFires.end(); /* Iterator set in loop, to support element removal*/)
	{
			fireIt = Fire::activeFires.erase(fireIt);
	}
	isInitialized = false;
}

void WildfireManager::Begin()
{
	if (isInitialized)
	{
		Reset();
		fireOrigin = nullptr;
	}
	isInitialized = initializePosition();
}

void WildfireManager::Step(double dt)
{
	if (isInitialized)
	{
		if (dt > 0)
		{
			updatePropagationmatrix();
			fireOrigin->step(dt, damagedEntities);
			for (auto fireIt = Fire::activeFires.begin(); fireIt != Fire::activeFires.end(); /* Iterator set in loop, to support element removal*/)
			{
				// Step the fire
				(*fireIt)->step(dt, damagedEntities);

				// Remove fire if it's exhausted.
				// Set the next iterator.
				if ( !(*fireIt)->isBurning() && (*fireIt)->getFuel() <= 0 )
				{
					//(fireIt*)
					fireIt = Fire::activeFires.erase(fireIt);
				}
				else
				{
					fireIt++;
				}
			}
		}
	}
}

void WildfireManager::CreateDamageReport()
{
	//logtxt(titanApi, "Damage Report");
	std::ofstream report;
	report.open(titanApi->getUserDataDirectory() + "\\plugins\\EmergencyResponder\\damageReport.csv");

	if (report.good())
	{
		report << "Type, Casualties, Injuries, Monetary Damage" << std::endl;
		int deaths = 0;
		int injuries = 0;
		int vehiclesDestroyed = 0;
		int vehiclesDamaged = 0;
		double estimatedValueLost = 0.0;

		for (auto it = damagedEntities.begin(); it != damagedEntities.end(); it++)
		{
			//// Get the name of the entity, in future we will likely store the entity blueprint.
			//std::string name;

			// It the second value is non-negative, it is an entities health value.
			if ((*it).second >= 0.0)
			{
				std::shared_ptr<IEntity> entity = titanApi->getScenarioManager()->getEntity((*it).first);
				if (entity != nullptr)
				{
					if (entity->isA(EntityCategory::EntityVehicle))
					{
						if ((*it).second > 0.0)
							vehiclesDamaged++;
						else
							vehiclesDestroyed++;
					}
					else
					{
						if ((*it).second > 0.0)
							injuries++;
						else
							deaths++;
					}
				}
			}
			else
			{
				// If the damage is a negative value, the entity is a Static Object (building).
				// See Fire::estimateBuidlingDamage() and Fire::damageEntitiesAtFireLocation()
				
				if (WildfireConfig::buildingValue.find((*it).first) != WildfireConfig::buildingValue.end())
				{
					estimatedValueLost += std::abs((*it).second) * WildfireConfig::buildingValue[(*it).first];
				}
				else
				{
					estimatedValueLost += std::abs((*it).second) * WildfireConfig::buildingValue["default"];
				}
			}
		}
	
		report << "People, " << deaths << ", " << injuries << ", 0" << std::endl;
		// Currently not estimating the value of vehicles
		report << "Vehicles, " << vehiclesDestroyed << ", " << vehiclesDamaged << ", 0" << std::endl;
		report << "Buildings, 0, 0, " /*<< std::fixed*/ << (long)estimatedValueLost /*<< std::scientific*/ << std::endl;
	}
	else
	{
		logtxt(titanApi, "Failed to create damage report!");
	}

	

}

void WildfireManager::updatePropagationmatrix()
{
	// Get wind data.
	WeatherData weatherData = titanApi->getWorldManager()->getWeatherData(initialPosition);
	double windHeading = weatherData.windDirection;
	double windSpeed = weatherData.windSpeed;

	double sum = 0.0;	// used to normalize the vector

	// In 8 cardinal directions
	for (size_t i = 0; i < propagationProb.size(); i++)
	{
		// Record the angle of the wind compared to spread direction.
		// Store the angle if the wind is in the same direction as the spread (180 degrees).
		propagationProb[i] = std::max(0.0, cos(windHeading - (i*M_PI_4)));
		sum += propagationProb[i]; // Add to sum for normalizing
	}
	for (size_t i = 0; i < propagationProb.size(); i++)
	{
		// Normalize the vector
		propagationProb[i] /= sum;
	}

	//if (windSpeed < WIND_SPEED_LIMIT)
	// Considered further limiting the propagation to a tighter direction/cone
	//  in the wind direction after this limit was reached.
	// Currently after the wind speed has reached this WIND_SPEED_LIMIT
	//  the fire's propagation will not alter.
	{
		// Increase spread probability in the direction of the wind.
		// When the wind is 0, spread will be equal in all directions.
		// When the wind reaches WIND_SPEED_LIMIT (default 20.0m/s),
		//   spread will only occur in the wind direction.

		// The wind strength as a percentage of WIND_SPEED_LIMIT
		double limitedWindSpeed = std::min(windSpeed, WildfireConfig::windSpeedLimit);
		double windStrength = (limitedWindSpeed / 20.0);
		for (size_t i = 0; i < propagationProb.size(); i++)
		{
			propagationProb[i] = ((1 - windStrength) / 8) + (propagationProb[i] * (1 - (1 - windStrength)));
			
			// Accumalate the vector
			if (i > 0)
				propagationProb[i] += propagationProb[i - 1];
		}
	}
}

bool WildfireManager::initializePosition()
{
	std::set<std::shared_ptr<IEntity>> fireList;

	fireList = titanApi->getScenarioManager()->getEntities(titanApi->getWorldManager()->getEntityDescriptor("er_wildfire_control_object"));

	// Get the first entity in the set.
	auto fire = fireList.begin();
	if (fire != fireList.end())
	{
		//controlEntity = (*fire);
		initialPosition = (*fire)->getPosition();

		Position position{ 0, 0 };
		fireOrigin = std::make_shared<Fire>(titanApi, initialPosition, position);
		//Fire::activeFires.insert(std::make_shared<Fire>(titanApi, initialPosition, position));

		return true;
	}
	else
	{
		return false;
	}
}

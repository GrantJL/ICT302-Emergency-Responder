
#include "WildfireManager.h"

std::vector<double> WildfireManager::propagationProb(8);

WildfireManager::WildfireManager(std::shared_ptr<ITitan> api) 
	: titanApi(api), isInitialized(false)
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
			updatePropagationmatrix();
			fireOrigin->step(dt, damagedEntities);
		}
	}
}

void WildfireManager::CreateDamageReport()
{
	logtxt(titanApi, "Damage Report");

	for (auto it = damagedEntities.begin(); it != damagedEntities.end(); it++)
	{
		// Get the name of the entity, in future we will likely store the entity blueprint.
		std::string name;
		std::shared_ptr<IEntity> en = titanApi->getScenarioManager()->getEntity((*it).first);
		if (en != nullptr)
			name = en->getName();
		else
			name = "Unknown";

		// Output the entity's name and health (when it was last damaged by fire).
		logtxt(titanApi, name + " " + std::to_string(it->second));
	}

	logtxt(titanApi, "END Damage Report");
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
	if (windSpeed < 20.0)
	{
		// Increase spread probability in the direction of the wind.
		// When the wind is 0, spread will be equal in all directions.
		// When the wind reaches 20m/s, spread will only occur in the wind direction.

		// The wind strength as a percentage of 20m/s
		double windStrength = (windSpeed / 20.0);
		for (size_t i = 0; i < propagationProb.size(); i++)
		{
			propagationProb[i] = ((1 - windStrength) / 8) + (propagationProb[i] * (1 - (1 - windStrength)));
			logtxt(titanApi, "" + std::to_string(propagationProb[i]) + " ");
		}
		logtxt(titanApi, "\n");
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
		controlEntity = (*fire);
		initialPosition = controlEntity->getPosition();

		Position position{ 0, 0 };
		fireOrigin = std::make_shared<Fire>(titanApi, initialPosition, position);

		return true;
	}
	else
	{
		return false;
	}
}

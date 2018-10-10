
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

	/// Probability vector of fire spread directions (based on wind direction/speed)
	std::vector<double> propagationProb;

	/// Entity UUID, Entity last recorded health
	std::map<std::string, double> damagedEntities;

	Vec3d initialPosition;
	bool isInitialized;
	
	std::shared_ptr<IEntity> controlEntity;
	std::shared_ptr<Fire> fireOrigin;		// to be replaced with fire data structure
public:

	WildfireManager(std::shared_ptr<ITitan> api)
		: titanApi(api),
		  isInitialized(false),
		  propagationProb(8)
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
				updatePropagationmatrix();

				fireOrigin->step(dt, damagedEntities);
			}
		}
	}

	/**
	 * Prints a report of damaged entities to a log file.
	 */
	void CreateDamageReport()
	{
		*logtxt(titanApi) << "Damage Report" << std::endl;
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
			*logtxt(titanApi) << name << "  " << (*it).second << std::endl;
		}
		*logtxt(titanApi) << "END Damage Report" << std::endl;
	}

private:

	void updatePropagationmatrix()
	{
		// Get wind data.
		titan::api2::WeatherData weatherData = titanApi->getWorldManager()->getWeatherData(initialPosition);
		double windHeading = weatherData.windDirection;
		double windSpeed = weatherData.windSpeed;

		double sum = 0.0;	// used to normalize the vector

		// In 8 cardinal directions
		for (int i = 0; i < propagationProb.size(); i++)
		{
			// Record the angle of the wind compared to spread direction.
			// Store the angle if the wind is in the same direction as the spread (180 degrees).
			propagationProb[i] = std::max(0.0, cos(windHeading - (i*M_PI_4)));
			sum += propagationProb[i]; // Add to sum for normalizing
		}
		for (int i = 0; i < propagationProb.size(); i++)
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
			for (int i = 0; i < propagationProb.size(); i++)
			{
				propagationProb[i] = ((1 - windStrength) / 8) + (propagationProb[i] * (1 - (1 - windStrength)));
			}
		}
	}
	
	/**
	 * Initializes the wildfires position to the position of the first found entity of the correct type.
	 */
	bool initializePosition()
	{
		std::set<std::shared_ptr<IEntity>> fireList;

		fireList = titanApi->getScenarioManager()->getEntities(titanApi->getWorldManager()->getEntityDescriptor("er_wildfire_control_object"));

		// Get the first entity in the set.
		auto fire = fireList.begin();
		if ( fire != fireList.end() )
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
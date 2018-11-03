#ifndef ER_WILDFIRE_CONFIG_
#define ER_WILDFIRE_CONFIG_

#include "TitanResources.h"

#include <string>

class WildfireConfig
{
public:
	static std::map<std::string, double> buildingValue;
	static std::vector<double> surfaceFactor;
	static const int surfaceFactorSize = 16;
	static double fireGap;					/// Gap between fires in degrees(longitude)
	static double vehicleDamageModifier;	/// The damage percentage vehivles take.
	static double buildingDamageModifier;	/// The damage percentage buildings take.
	static int maxFuel;						/// Maximum possible fuel (10 minutes, assuming burn rate of 1/s)
	static double baseDamageRadius;			/// Radius in which fires deal damage
	static double baseDamage;				/// Base damage dealt per second. (Percentage of total health)
	static double windSpeedLimit;			/// Speed at which fire spread reaches its narrowest state

	static bool loadConfig(const std::string& file);
};

#endif ER_WILDFIRE_CONFIG_
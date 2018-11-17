#ifndef ER_WILDFIRE_CONFIG_
#define ER_WILDFIRE_CONFIG_

#include "TitanResources.h"

#include <string>

class WildfireConfig
{
public:
	static std::map<std::string, double> buildingValue;	/// Map of building names and their value
	static std::map<std::string, double> vehicleValue;	/// Map of vehicle names and their value
	static std::vector<double> surfaceFactor;			/// Vector of surface types and their combustability factor (titan::...::TerrainMaterial)
	static const int surfaceFactorSize = 16;/// Number of surface types
	static double fireGap;					/// Gap between fires in degrees(longitude)
	static double vehicleDamageModifier;	/// The damage percentage vehivles take.
	static double buildingDamageModifier;	/// The damage percentage buildings take.
	static int maxFuel;						/// Maximum possible fuel (10 minutes, assuming burn rate of 1/s)
	static double baseDamageRadius;			/// Radius in which fires deal damage
	static double baseDamage;				/// Base damage dealt per second. (Percentage of total health)
	static double windSpeedLimit;			/// Speed at which fire spread reaches its narrowest state
	static double propagationRate;			/// The probability a Fire will spread per tick (spreads in one direction)

	static bool loadConfig(const std::string& file);
};

#endif ER_WILDFIRE_CONFIG_
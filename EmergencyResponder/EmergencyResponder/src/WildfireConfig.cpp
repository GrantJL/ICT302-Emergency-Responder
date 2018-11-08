#include "WildfireConfig.h"

#include <json/json.h>
#include <iostream>
#include <fstream>

// Initialize data structures
std::vector<double> WildfireConfig::surfaceFactor(surfaceFactorSize, 1.0);
std::map<std::string, double> WildfireConfig::buildingValue;
std::map<std::string, double> WildfireConfig::vehicleValue;

// Initialize defaults
double WildfireConfig::fireGap = 0.000002;
double WildfireConfig::vehicleDamageModifier = 0.5;
double WildfireConfig::buildingDamageModifier = 0.25;
int WildfireConfig::maxFuel = 600;
double WildfireConfig::baseDamageRadius = 5.0;
double WildfireConfig::baseDamage = 0.2;
double WildfireConfig::windSpeedLimit = 20.0;


bool WildfireConfig::loadConfig(const std::string& file)
{
	std::ifstream configFile;
	configFile.open(file);

	if (!configFile.bad())
	{
		Json::Value root;

		Json::Reader reader;
		reader.parse(configFile, root, false);

		if (root.isMember("FireGap") && root["FireGap"].isNumeric())
			fireGap = root["FireGap"].asDouble();

		if (root.isMember("VehicleDamageMult") && root["VehicleDamageMult"].isNumeric())
			vehicleDamageModifier = root["VehicleDamageMult"].asDouble();

		if (root.isMember("BuildingDamageMult") && root["BuildingDamageMult"].isNumeric())
			buildingDamageModifier = root["BuildingDamageMult"].asDouble();

		if (root.isMember("MaxFuel") && root["MaxFuel"].isNumeric())
			maxFuel = root["MaxFuel"].asInt();

		if (root.isMember("DamageRadius") && root["DamageRadius"].isNumeric())
			baseDamageRadius = root["DamageRadius"].asDouble();

		if (root.isMember("BaseDamage") && root["BaseDamage"].isNumeric())
			baseDamage = root["BaseDamage"].asDouble();

		if (root.isMember("WindSpeedLimit") && root["WindSpeedLimit"].isNumeric())
			windSpeedLimit = root["WindSpeedLimit"].asDouble();

		if (root.isMember("SurfaceMaterialFuelFactor") && root["SurfaceMaterialFuelFactor"].isArray())
		{
			int size = root["SurfaceMaterialFuelFactor"].size();

			if (size == surfaceFactorSize)
			{
				for (int i = 0; i < surfaceFactorSize; i++)
				{
					if (root["SurfaceMaterialFuelFactor"][(int)i].isNumeric())
						surfaceFactor.at(i) = root["SurfaceMaterialFuelFactor"][(int)i].asDouble();
				}
			}
		}

		if (root.isMember("BuildingMonetaryValue") && root["BuildingMonetaryValue"].isObject())
		{
			Json::Value configVals = root["BuildingMonetaryValue"];
			std::vector<std::string> names = configVals.getMemberNames();

			for (auto it = names.begin(); it != names.end(); it++)
			{
				if (configVals[(*it)].isNumeric())
					buildingValue[(*it)] = configVals[(*it)].asDouble();
			}
		}
		
		if (root.isMember("VehicleMonetaryValue") && root["VehicleMonetaryValue"].isObject())
		{
			Json::Value configVals = root["VehicleMonetaryValue"];
			std::vector<std::string> names = configVals.getMemberNames();

			for (auto it = names.begin(); it != names.end(); it++)
			{
				if (configVals[(*it)].isNumeric())
					vehicleValue[(*it)] = configVals[(*it)].asDouble();
			}
		}

		configFile.close();

		return true;
	}
	else
	{
		return false;
	}
}
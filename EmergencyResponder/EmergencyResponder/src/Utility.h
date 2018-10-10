#ifndef ER_UTILITY_H_
#define ER_UTILITY_H_

#include "TitanResources.h"

#include <fstream>
#include <string>

using namespace titan::api2;

static double getTreeDensity(const Vec3d& position)
{
	return 1.0;
}

static double getSurfaceCombustion(const Vec3d& position)
{
	return 1.0;
}


static std::shared_ptr<std::ofstream> logtxt(const std::shared_ptr<ITitan>& api/*, const std::string& message*/)
{
	static std::shared_ptr<std::ofstream> logger = std::make_shared<std::ofstream>();

	if (!logger->is_open())
	{
		logger->open(api->getUserDataDirectory() + "\\" + "ER" + ".log");
	}

	return logger; // logger << message << std::endl;
}


#endif // ER_UTILITY_H_
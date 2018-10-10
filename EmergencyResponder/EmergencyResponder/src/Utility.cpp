
#include "Utility.h"

static double getTreeDensity(const Vec3d& position)
{
	return 1.0;
}

static double getSurfaceCombustion(const Vec3d& position)
{
	return 1.0;
}

void logtxt(const std::shared_ptr<ITitan>& api, const std::string& message)
{
	static std::ofstream logger;

	if (!logger.is_open())
	{
		logger.open(api->getUserDataDirectory() + "\\" + "ER-Damage" + ".log");
	}

	logger << message << std::endl;
} 
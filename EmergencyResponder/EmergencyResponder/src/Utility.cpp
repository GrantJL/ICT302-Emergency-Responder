
#include "Utility.h"

void logtxt(const std::shared_ptr<ITitan>& api, const std::string& message)
{
	static std::ofstream logger;

	if (!logger.is_open())
	{
		logger.open(api->getUserDataDirectory() + "\\" + "ER-Damage" + ".log");
	}

	logger << message << std::endl;
} 

#ifndef ER_WILDFIRE_MANAGER_H_
#define ER_WILDFIRE_MANAGER_H_

#include <titan/plugin2/IRenderManager.h>

class WildfireManager
{
private:

public:
	std::shared_ptr<IRenderManager> renderer;		//Debug logging 

	void Begin()
	{
		renderer->debugLog("Begin");
	}

	void Step()
	{
		renderer->debugLog("Step");
	}
};

#endif // ER_WILDFIRE_MANAGER_H_
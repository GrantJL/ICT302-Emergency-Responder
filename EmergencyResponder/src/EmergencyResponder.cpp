
#include "titan/plugin2/plugin.h"

#include <fstream>
#include <string>

//#include "titan/plugin2/ITitan.h"
//#include <titan/plugin2/IRenderManager.h>
//#include <titan/plugin2/IScenarioManager.h>
//#include <titan/plugin2/IWorldManager.h>
//#include "titan/plugin2/IEntity.h"
//#include <titan/plugin2/IEntity.h>
//#include <titan/plugin2/types.h>
//#include <titan/plugin2/ICamera.h>
//#include <titan/plugin2/util/MathHelpers.h>
//#include <titan/plugin2/IStaticObject.h>
//#include <json/json.h>

#include "ERPlugin.h"

using namespace titan::api2;
//using namespace titan::api2::util;


//------------------------------------------------------------//
//                      PLUGIN CONSTANTS                      //
//------------------------------------------------------------//
const std::string PLUGIN_NAME = "EmergencyResponder";

//------------------------------------------------------------//
//                      PLUGIN VARIABLES                      //
//------------------------------------------------------------//
EmergencyResponder simulation;
std::ofstream logger;

//------------------------------------------------------------//
//                      PLUGIN FUNCTIONS                      //
//------------------------------------------------------------//
void pluginStartup(const std::shared_ptr<ITitan>& api)
{
	simulation.initialize(api);

	logger.open(api->getUserDataDirectory() + "\\" + PLUGIN_NAME + ".log");		//possible to do make dir ***\plugins\PLUGIN_NAME
	logger << "[" << PLUGIN_NAME << "] Plugin Initialised" << std::endl;
}

void pluginUpdate(double dt) 
{
	simulation.update(dt);
}

void pluginShutdown()
{
	logger << "[" << PLUGIN_NAME << "] Plugin Shutdown" << std::endl;
	logger.close();
}

/*
void pluginStartup(const shared_ptr<ITitan>& api)
{
	titanApi = api;
	logger.open(api->getUserDataDirectory() + "\\" + PLUGIN_NAME + ".log");

	events = titanApi->getEventManager();
	renderer = titanApi->getRenderManager();
	world = titanApi->getWorldManager();
	listener = make_shared<Listener>();
	renderer->debugLog(api->getUserDataDirectory() + "\\plugins\\EResp\\" + PLUGIN_NAME + ".log");

	// Listen to our events (Currently using EResp to identify this plugins events)
	events->addTitanEventListener("EResp::request", listener);
	events->addTitanEventListener("EResp::spawnFire", listener);
}

void pluginShutdown()
{
	logger.close();

	// Unregister listeners
	events->removeTitanEventListener("EResp::spawnFire", listener);
	events->removeTitanEventListener("EResp::request", listener);

	listener = nullptr;
	renderer = nullptr;
	events = nullptr;
	titanApi = nullptr;
}
*/
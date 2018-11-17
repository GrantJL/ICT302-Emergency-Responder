
#include "titan/plugin2/plugin.h"

#include <fstream>
#include <string>

#include "ERPlugin.h"
#include "Utility.h"

using namespace titan::api2;

//------------------------------------------------------------//
//                      PLUGIN CONSTANTS                      //
//------------------------------------------------------------//
const std::string PLUGIN_NAME = "EmergencyResponder";

//------------------------------------------------------------//
//                      PLUGIN VARIABLES                      //
//------------------------------------------------------------//
EmergencyResponder simulation;
std::shared_ptr<ITitan> titanApi;

//------------------------------------------------------------//
//                      PLUGIN FUNCTIONS                      //
//------------------------------------------------------------//
void pluginStartup(const std::shared_ptr<ITitan>& api)
{
	titanApi = api;

	simulation.initialize(titanApi);

	logtxt(titanApi, "[" + PLUGIN_NAME + "] Plugin Initialised");
}

void pluginUpdate(double dt) 
{
	simulation.update(dt);
}

void pluginShutdown()
{
	logtxt(titanApi, "[" + PLUGIN_NAME + "] Plugin Shutdown");
}
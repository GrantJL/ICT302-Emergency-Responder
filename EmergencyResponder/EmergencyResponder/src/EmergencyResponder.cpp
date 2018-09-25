#include "titan/plugin2/plugin.h"

#include <fstream>
#include <Windows.h>
#include "titan/plugin2/IEntity.h"
#include "titan/plugin2/ITitan.h"
#include <titan/plugin2/ITitanEventListener.h>
#include <titan/plugin2/IEventManager.h>
#include <titan/plugin2/IEntity.h>
#include <titan/plugin2/types.h>
#include <titan/plugin2/IRenderManager.h>
#include <titan/plugin2/IWorldManager.h>
#include <titan/plugin2/ICamera.h>
#include <titan/plugin2/util/MathHelpers.h>
#include <titan/plugin2/IScenarioManager.h>

#include <json/json.h>

using namespace std;
using namespace titan::api2;
using namespace titan::api2::util;


//----------------------------------------------------------
//                        CONSTANTS
//----------------------------------------------------------
const string PLUGIN_NAME = "Emergency Responder";

//----------------------------------------------------------
//                      PLUGIN MEMBERS
//----------------------------------------------------------
ofstream logger;
shared_ptr<ITitan> titanApi;

class Listener; //forward declaration
shared_ptr<Listener> listener;
shared_ptr<IRenderManager> renderer;
shared_ptr<IEventManager> events;
shared_ptr<IWorldManager> world;

set<shared_ptr<IEntity>> entities;

// Listener to listen for our UI button event "Plugo::rotate"
class Listener : public ITitanEventListener
{
public:
	void onTitanEvent(const string& name, const Json::Value& params) override
		
	{
		//fading log
		renderer->debugLog("Event received " + name);

		if (name == "EResp::request")
		{
			// Get a list of all entities in the scene
			shared_ptr<IScenarioManager> scene = titanApi->getScenarioManager();
			
			Json::Value fire(Json::arrayValue);
			entities = scene->getEntities(world->getEntityDescriptor("er_wildfire_system"));

			logger << "Fires:" << endl;
			for (shared_ptr<IEntity> entity : entities)
			{// Add entity names to Json array
				logger << "  " << entity->getName() << endl;
				fire.append(entity->getName());
			}

			set<shared_ptr<IEntity>> ents = scene->getEntities();

			// Create a new Json array to hold entity names
			Json::Value names(Json::arrayValue);
			logger << "Entities:" << endl;
			for (shared_ptr<IEntity> entity : ents)
			{// Add entity names to Json array
				logger << "  " << entity->getName() << endl;
				names.append(entity->getName());
			}

			Json::Value args;	//Create a Json object
			args["entities"] = names;
			args["fires"] = fire;
			events->sendTitanEvent("EResp::entities", args);	//Send to event bus with Json
		}
		else if (name == "EResp::spawnFire")
		{
			EntityDescriptor desc = world->getEntityDescriptor("er_wildfire_system");

			Vec3d pos = renderer->getActiveCamera()->getPosition();
			Vec3d rotation = MathHelpers::getNorthFacingVector(pos);

			Quat rot;
			MathHelpers::createGroundAlignedQuaternion(rotation, pos, rot);

			shared_ptr<IScenarioManager> scene = titanApi->getScenarioManager();
			scene->createEntity(desc, pos, rot);

			Json::Value args;
			TerrainMaterial mat = world->getSurfaceMaterialBelow(pos);
			args["material"] = mat;
			renderer->debugLog(std::to_string(mat));
			events->sendTitanEvent("EResp::entities", args);
		}
	}
};

//----------------------------------------------------------a
//                     PLUGIN FUNCTIONS
//----------------------------------------------------------
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

void pluginUpdate(double dt)
{

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
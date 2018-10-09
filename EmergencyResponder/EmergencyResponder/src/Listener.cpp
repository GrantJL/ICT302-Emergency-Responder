
#include "Listener.h"
#include "WildfireManager.h"

void Listener::onTitanEvent(const std::string& name, const Json::Value& params)
{
	if (name == "EResp::Begin")
	{
		wildfire->Begin();
	}
	else if (name == "EResp::DamageReport")
	{
		wildfire->CreateDamageReport();
	}

	/*
	//fading log
	renderer->debugLog("Event received " + name);

	if (name == "EResp::request")
	{
	// Get a list of all entities in the scene
	std::shared_ptr<IScenarioManager> scene = pluginApi->getScenarioManager();

	// Json array, this will hold the fire entities
	Json::Value fireArr(Json::arrayValue);
	fires = scene->getEntities(world->getEntityDescriptor("er_wildfire_system"));
	logger << "Fires:" << endl;
	for (std::shared_ptr<IEntity> entity : fires)
	{// Add fires to Json array
	logger << "  " << entity->getName() << endl;

	Json::Value obj; // Each entity is a json obj with name and id
	obj["name"] = entity->getName();
	obj["id"] = entity->getUuid();
	fireArr.append(obj);
	}

	// This will get SOME buildings, after you shoot them.
	std::set<std::shared_ptr<IStaticObject>> statics = scene->getStaticObjects(renderer->getActiveCamera()->getPosition(), 500.0);

	// Create a new Json array to hold static entities
	Json::Value staticArr(Json::arrayValue);
	logger << "Static objects:" << endl;
	for (std::shared_ptr<IStaticObject> entity : statics)
	{// Add entity names to Json array
	logger << "  " << entity->getDescriptor().defaultEntityName << endl;

	Json::Value obj;
	obj["name"] = entity->getDescriptor().name;
	obj["id"] = ""; // Static objects have no uiid (accessible)
	staticArr.append(obj);
	}

	Json::Value args;	//Create a Json object
	args["buildings"] = staticArr;
	args["fires"] = fireArr;
	events->sendTitanEvent("EResp::entities", args);	//Send to event bus with Json
	}
	else if (name == "EResp::spawnFire")
	{// spawns a wildfire at the camera's position
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
	*/
}
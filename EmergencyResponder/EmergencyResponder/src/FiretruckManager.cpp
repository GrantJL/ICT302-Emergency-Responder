#include "FiretruckManager.h"

FiretruckManager::FiretruckManager(std::shared_ptr<titan::api2::ITitan> api)
{
	titanApi = api;
	dispatching = false;
}

void FiretruckManager::SendFiretrucks()
{
	titan::api2::EntityDescriptor firetruckDescriptor = titanApi->getWorldManager()->getEntityDescriptor("firetruck");
	std::set<std::shared_ptr<titan::api2::IEntity>> entities = titanApi->getScenarioManager()->getEntities(firetruckDescriptor);
	std::set<std::shared_ptr<titan::api2::IEntity>>::iterator entityIter;
	for (entityIter = entities.begin(); entityIter != entities.end(); entityIter++)
	{
		Json::Value data;
		std::shared_ptr<titan::api2::IEntity> entity = *entityIter;
		data["uuid"] = entity->getUuid();
		data["name"] = entity->getName();
		titanApi->getEventManager()->sendTitanEvent("EResp::SendFiretrucks", data);
	}
}

void FiretruckManager::SetMousePosition(titan::api2::Vec3d position)
{
	mousePosition = position;
	mousePosition = titanApi->getWorldManager()->getNearestSurfacePositionBelow(mousePosition);
}

void FiretruckManager::DispatchFiretruck()
{
	std::shared_ptr<titan::api2::IEntity> entity = titanApi->getScenarioManager()->getEntity(uuid);
	std::shared_ptr<titan::api2::IWaypointPath> path = titanApi->getScenarioManager()->createWaypointPath();
	if (m_paths.find(uuid) != m_paths.end())
	{
		m_paths[uuid]->removeWaypoint(0);
	}
	m_paths[uuid] = path;
	path->addWaypoint(mousePosition);
	entity->setWaypointPath(path);
	titanApi->getEventManager()->sendTitanEvent("EResp::ResetUI", NULL);
	dispatching = false;
}

void FiretruckManager::BeginDispatching(std::string newUuid)
{
	uuid = newUuid;
	dispatching = true;
}

bool FiretruckManager::IsDispatching()
{
	return dispatching;
}
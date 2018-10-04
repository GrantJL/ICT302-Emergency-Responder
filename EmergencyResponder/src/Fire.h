#ifndef ER_FIRE_H
#define ER_FIRE_H

#include <memory>

#include "titan/plugin2/IEntity.h"
#include "titan/plugin2/IScenarioManager.h"
#include "titan/plugin2/IDamageModel.h"

class Fire
{
public:
	std::shared_ptr<titan::api2::IScenarioManager> scenario;

	Fire();
	//Fire(const Fire & other); // Copy fires neighbours.

	void setFuel(const double fuelValue);
	void setBurning(const bool burningValue);

	double getFuel();
	bool isBurning();

	void step(const double dt);

	void damageEntitiesAtFireLocation(std::shared_ptr<titan::api2::IEntity> fire);

private:
	double fuel;
	bool burning;

	const double radius = 5.0;
	const double damage = 1.0;

};

#endif // ER_FIRE_H
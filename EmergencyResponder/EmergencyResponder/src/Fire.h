#ifndef ER_FIRE_H
#define ER_FIRE_H

#include <memory>

#include <titan/plugin2/types.h>

class Fire
{
private:
	// Titan Pointers
	std::shared_ptr<titan::api2::ITitan> titanApi;

	std::shared_ptr<titan::api2::IEntity> entity;

	double fuel;
	bool burning;

	const double radius = 5.0;
	const double damage = 1.0;

public:
	Fire(std::shared_ptr<titan::api2::ITitan> titanApi, const titan::api2::Vec3d& position);
	//Fire(const Fire & other); // Copy fires neighbours.

	void setFuel(const double fuelValue);
	void setBurning(const bool burningValue);

	double getFuel();
	bool isBurning();

	void step(const double dt);

	void damageEntitiesAtFireLocation();


};

#endif // ER_FIRE_H
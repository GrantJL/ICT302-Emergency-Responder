#ifndef ER_FIRE_H
#define ER_FIRE_H

#include <memory>

#include "TitanResources.h"

using namespace titan::api2;

class Fire
{
private:
	static double vehicleModifier;
	static double buildingModifier;
	static double maxFuel;
	double fuelFactor; // calculate once per tick

	// Titan Pointers
	std::shared_ptr<ITitan> titanApi;
	std::shared_ptr<IEntity> fireEntity;

	Vec3d firePosition;
	Quat fireRotation;

	double fuel;
	bool burning;

	const double radius = 5.0;
	const double damage = 0.2;

public:

	/**
	 * Sets the amount of damage vehicles take in comparison to regular entities.
	 * Default value is 0.5, i.e. vehicles take twice as long to destroy.
	 */
	static void setVehicleModifier(double val) { vehicleModifier = val; };
	/**
	* Sets the amount of damage buildings take in comparison to regular entities.
	* Default value is 0.25, i.e. buildings take four times longer to destroy.
	*/
	static void setBuildingModifier(double val) { buildingModifier = val; };

	Fire(std::shared_ptr<ITitan> titanApi, const Vec3d& position);
	//Fire(const Fire & other); // Copy fires neighbours.

	void setFuel(const double fuelValue);
	void setBurning(const bool burningValue);

	double getFuel();
	bool isBurning();

	/**
	 * Steps the fire simulation, adding damaged entites and their updated health to <code>damagedEntities</code>.
	 * @param dt the time in seconds since the last step
	 * @param damagedEntities a map of entities UUID and their health.
	 */
	void step(const double dt, std::map<std::string, double>& damagedEntities);

private:

	/**
	* Damages entities in range of the fire, adding damaged entites and their updated health to <code>damagedEntities</code>.
	* @param dt the time in seconds since the last step
	* @param damagedEntities a map of entities UUID and their health.
	*/
	void  damageEntitiesAtFireLocation(double dt, std::map<std::string, double>& damagedEntities);

};

#endif // ER_FIRE_H
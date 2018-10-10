#ifndef ER_FIRE_H
#define ER_FIRE_H

#include <memory>

#include <titan/plugin2/types.h>

enum FIRE_SPREAD_DIR
{
	N = 0,
	NE = 1,
	E = 2,
	SE = 3,
	S = 4,
	SW = 5,
	W = 6,
	NW = 7
};

class Fire
{
private:
	static double vehicleModifier;
	static double buildingModifier;
	static double maxFuel;
	double fuelFactor; // calculate once per tick

	// Titan Pointers
	std::shared_ptr<titan::api2::ITitan> titanApi;
	std::shared_ptr<titan::api2::IEntity> fireEntity;
	std::vector<Fire> children;

	double fuel;
	titan::api2::Vec3d position;
	bool burning;
	bool propped[8] =
	{
		false, false, false,
		false, false, false,
		false, false
	};

	const double radius = 5.0;
	const double damage = 0.2;

	const double CONST_PROB[8] = 
							{ 12.5, 12.5, 12.5,
							  12.5, /*0*/ 12.5,
							  12.5, 12.5, 12.5};

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

	Fire(std::shared_ptr<titan::api2::ITitan> titanApi, const titan::api2::Vec3d& position);
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
	bool willPropagate();
	bool willPropagate(const double percent);
	bool fireAtPosition(const titan::api2::Vec3d position);

};

#endif // ER_FIRE_H
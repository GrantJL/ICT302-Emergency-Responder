#ifndef ER_FIRE_H
#define ER_FIRE_H

#include <memory>
#include <vector>

#include "TitanResources.h"

using namespace titan::api2;

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

struct Position {
	int x;
	int y;
};


class Fire
{
private:
	static double vehicleModifier;
	static double buildingModifier;
	static double maxFuel;

	/// The relative positions for fire spread
	static const std::vector<Position> spreadDirections;

	static double fireGap; /// The gap between fires (in long/lat degrees)
	static std::vector<Position> globalFires; /// A list of all fire positions

	double fuelFactor; /// How intense the fire curently is/ based on its lifetime.(calculate once per tick)

	// Titan Pointers
	std::shared_ptr<ITitan> titanApi;
	std::shared_ptr<IEntity> fireEntity; /// Pointer to fire effect entitiy, THIS MAY BE NULLPTR WHEN FIRE IS EXHAUSTED.

	std::vector<Fire> children; /// A list of fires spawned from this Fire.

	Position basicPosition; /// This fires position in interger euler coordinates.
	Vec3d realPosition;		/// This fires real world position (in ECEF)
	Quat fireRotation;		/// This fire rotation (not required?)

	double fuel;
	bool burning;	/// Is the fire currently burning.
	bool propped[8] =	/// Has the fire propagated in each direction. (N, NE, E, ...)
	{
		false, false, false,
		false, false, false,
		false, false
	};

	const double radius = 5.0; // TODO: Configurize (this is the radius in which the fire deals damage)
	const double firetruckRadius = 20.0; // TODO: Configurize (value for the radius that firetrucks can do damage)
	const double firetruckFuelReduction = 10.0; // TODO: Configurize (value for how much fuel the firetruck can destroy)
	const double rainFuelDampValue = 20.0; // TODO: Configurize (Value for how strong the rain damping is)
	const double snowFuelDampValue = 15.0; // TODO: Configurize (value for how strong the snow damping is)
	const double damage = 0.2; // TODO: Configurize (Max damage a fire can to per tick.
	const int MAX_FIRES = 800; 

	const double CONST_PROB[8] = // TODO: Remove?
							{ 12.5, 12.5, 12.5,
							  12.5, /*0*/ 12.5,
							  12.5, 12.5, 12.5};

public:
	Fire(std::shared_ptr<ITitan> api, const Vec3d& realPosition, const Position& basicPosition);

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

	/**
	* Calculates a real world position relative to the parentPos.
	* Calculates a new real world coordinate relative to parent pos, 
	* this uses the static fireGap variable as the gap between the parent
	* and new position.
	*
	* @param parentPos - the position to calculate a new position from (ECEF coords).
	* @param position - the offest to calculate a new posiiton from.
	* @retutn calculated position relative to the parentPos (ECEF coords). 
	*/
	static Vec3d calculateRelativePosition(const titan::api2::Vec3d& parentPos, const Position& position);

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
	* Returns true when the two positions match, false otherwise.
	*/
	static bool compatePositions(const Position& posA, const Position& posB);

	/**
	* Damages entities in range of the fire, adding damaged entites and their updated health to <code>damagedEntities</code>.
	* @param dt the time in seconds since the last step
	* @param damagedEntities a map of entities UUID and their health.
	*/
	void damageEntitiesAtFireLocation(double dt, std::map<std::string, double>& damagedEntities);

	/**
	 * Reduces the fuel of the fire if it is within the radius of a firetruck. Should there be more than one firetruck,
	 * then there will be more fuel being reduced.
	 * @param dt The time since the last step
	 */
	void reduceFuelForFiretrucksAtFireLocation(double dt);
	/**
	 * Reduces the fuel of the fire depending on the weather. It will reduce faster with higher rain and snow densities.
	 * @param dt The time since the last step.
	 */
	void reduceFuelForWeather(double dt);
	/**
	* Returns true when this fire should propagate.
	*/
	bool willPropagate();
	// TODO: Document this better
	/**
	* Returns true <code>percent</code> percent of the time.	
	*/
	bool willPropagate(const double percent);
	/**
	* Returns true when a fire has already been spawned at given position.
	*/
	bool fireAtPosition(const Position position);

};

#endif // ER_FIRE_H
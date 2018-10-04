#ifndef ER_FIRE_H
#define ER_FIRE_H

class Fire
{
public:
	Fire();
	//Fire(const Fire & other); // Copy fires neighbours.

	void setFuel(const double fuelValue);
	void setBurning(const bool burningValue);

	double getFuel();
	bool isBurning();

	void step(const double dt);

private:
	double fuel;
	bool burning;

};

#endif // ER_FIRE_H
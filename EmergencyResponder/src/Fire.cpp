#include "Fire.h"

Fire::Fire()
{
	fuel = 30;
}

void Fire::setFuel(const double fuelValue)
{
	fuel = fuelValue;
}

void Fire::setBurning(const bool burningValue)
{
	burning = burningValue;
}

double Fire::getFuel()
{
	return fuel;
}

bool Fire::isBurning()
{
	return burning;
}

void Fire::step(const double dt)
{
	if (fuel > 0)
	{
		// Reduce fuel
		// Reduce health of entities.
	}
}
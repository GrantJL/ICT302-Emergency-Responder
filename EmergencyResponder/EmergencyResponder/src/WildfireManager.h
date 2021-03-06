
#ifndef ER_WILDFIRE_MANAGER_H_
#define ER_WILDFIRE_MANAGER_H_

#include "TitanResources.h"

#include "Utility.h"
#include "Fire.h"

#include "WildfireConfig.h"

using namespace titan::api2;

class WildfireManager
{
private:
	std::shared_ptr<ITitan> titanApi;

	/// Probability vector of fire spread directions (based on wind direction/speed)
	static std::vector<double> propagationProb;

	/// Entity UUID, Entity last recorded health
	std::map<std::string, double> damagedEntities;

	Vec3d initialPosition;
	bool isInitialized;
	
	std::shared_ptr<IEntity> controlEntity;
	std::shared_ptr<Fire> fireOrigin;		// to be replaced with fire data structure

public:

	WildfireManager(std::shared_ptr<ITitan> api);

	void Reset();
	void Begin();
	void Step(double dt);

	void CreateDamageReport();

	static std::vector<double> getPropagationProb()
	{
		return propagationProb;
	}

private:
	/**
	 * Updates the matrix which determines the base likelyhood a fire will proopagate in a direction.
	 * The Propagation likely hood is based on the current wind direction.
	 */
	void updatePropagationmatrix();
	bool initializePosition();


};

#endif // ER_WILDFIRE_MANAGER_H_

#ifndef ER_WILDFIRE_MANAGER_H_
#define ER_WILDFIRE_MANAGER_H_

#include "TitanResources.h"

#include "Utility.h"
#include "Fire.h"

using namespace titan::api2;

class WildfireManager
{
private:
	std::shared_ptr<ITitan> titanApi;

	/// Probability vector of fire spread directions (based on wind direction/speed)

	/// Entity UUID, Entity last recorded health
	std::map<std::string, double> damagedEntities;

	Vec3d initialPosition;
	bool isInitialized;
	
	std::shared_ptr<IEntity> controlEntity;
	std::shared_ptr<Fire> fireOrigin;		// to be replaced with fire data structure

public:

	WildfireManager(std::shared_ptr<ITitan> api);

	void Begin();
	void Step(double dt);

	void CreateDamageReport();

	static std::vector<double> propagationProb;
	static std::vector<double> getPropagationProb()
	{
		return propagationProb;
	}

private:
	void updatePropagationmatrix();
	bool initializePosition();
};

#endif // ER_WILDFIRE_MANAGER_H_
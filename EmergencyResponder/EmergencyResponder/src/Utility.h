#ifndef ER_UTILITY_H_
#define ER_UTILITY_H_

#include "TitanResources.h"

#include <fstream>
#include <string>

using namespace titan::api2;

static double getTreeDensity(const Vec3d& position);

static double getSurfaceCombustion(const Vec3d& position);

void logtxt(const std::shared_ptr<ITitan>& api, const std::string& message);

#endif // ER_UTILITY_H_
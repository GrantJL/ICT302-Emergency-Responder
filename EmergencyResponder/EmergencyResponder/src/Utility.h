#ifndef ER_UTILITY_H_
#define ER_UTILITY_H_

#include "TitanResources.h"

#include <fstream>
#include <string>

using namespace titan::api2;

static std::map<uint64_t, double> treeDensityCache;

static inline double getTreeDensity(const std::shared_ptr<ITitan> & api, const Vec3d& position)
{
	uint64_t tileId = api->getWorldManager()->getTileId(position);
	std::map<uint64_t, double>::iterator it = treeDensityCache.find(tileId);
	if (it != treeDensityCache.end())
	{// Tile ID exists in map
		return it->second;
	}

	std::vector<MeshRow> rows = api->getWorldManager()->getMeshVertices(tileId);
	std::vector<MeshRow>::iterator rowIter;
	double totalTreeDensity = 0;
	double count = 0;
	for (rowIter = rows.begin(); rowIter != rows.end(); rowIter++)
	{
		std::vector<MeshVertex> verts = (*rowIter).vertices;
		std::vector<MeshVertex>::iterator vertIter;
		for (vertIter = verts.begin(); vertIter != verts.end(); vertIter++)
		{
			totalTreeDensity += vertIter->treeDensity;
			count++;
		}
	}

	totalTreeDensity /= count;
	treeDensityCache[tileId] = totalTreeDensity;
	return totalTreeDensity;
}

static inline double getSurfaceCombustion(const std::shared_ptr<ITitan> & api, const Vec3d& position)
{
	TerrainMaterial material = api->getWorldManager()->getSurfaceMaterialBelow(position);
	switch (material)
	{
	case MaterialDirt:
	case MaterialGrass:
	case MaterialForestFloor:
	case MaterialUnknown:
		return 1.0;
	case MaterialMoss:
		return 0.5;
	case MaterialRock:
		return 0.2;
	case MaterialMud:
	case MaterialGravel:
	case MaterialAsphalt:
	case MaterialAsphaltPaintWhite:
	case MaterialSand:
	case MaterialConcrete:
	case MaterialAsphaltPaintYellow:
	case MaterialCobbles:
	case MaterialDirtDug:
	case MaterialSnow:
		return 0.0;
	default:
		return 0.0;
	}
}


void logtxt(const std::shared_ptr<ITitan>& api, const std::string& message);

#endif // ER_UTILITY_H_
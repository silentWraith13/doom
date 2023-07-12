#pragma once
#include "Engine/Math/AABB3.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct TileDefinition;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Tile
{
public:
	Tile(const AABB3& tileBounds, TileDefinition* tileDef);
	~Tile();
	AABB3 GetTileBounds() const;
	TileDefinition* GetTileDefinition() const;
	
	//member variables
	AABB3 m_tileBounds;
	TileDefinition* m_tileDef;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
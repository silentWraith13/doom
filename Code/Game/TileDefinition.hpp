#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct TileDefinition
{
public:
	TileDefinition();
	static void					 InitializeTileDefinitions(const char* path);
	static void					 ClearDefinitions();
	static TileDefinition* const GetTileDef(const std::string& tileDefName);
	bool						 LoadFromXmlElement(const XmlElement& xmlElement);

public:
	std::string		m_name;
	bool			m_isSolid = false;
	Rgba8			m_mapImagePixelColor  = Rgba8(255, 255, 255, 255);
	IntVec2			m_floorSpriteCoords   = IntVec2(-1, -1);
	IntVec2			m_ceilingSpriteCoords = IntVec2(-1, -1);
	IntVec2			m_wallSpriteCoords	  = IntVec2(-1, -1);
	static std::vector<TileDefinition*> s_tileDefinitions;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------
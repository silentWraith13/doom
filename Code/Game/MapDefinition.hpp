#pragma once
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class SpawnInfo;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct MapDefinition
{
public:
	MapDefinition();
	static void					 InitializeMapDefinitions(const char* path);
	static void					 ClearDefinitions();
	static MapDefinition* const  GetMapDef(const std::string& mapDefName);
	bool						 LoadFromXmlElement(const XmlElement& xmlElement);

public:
	std::string		m_name;
	Image			m_image;
	Shader*			m_shader			 = nullptr;
	Texture*		m_spriteSheetTexture = nullptr;
	IntVec2			m_spriteSheetCellCount;
	std::vector<SpawnInfo> m_spawnInfos;
	static std::vector<MapDefinition*> s_mapDefinitions;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------
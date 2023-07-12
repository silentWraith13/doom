#include "Game/MapDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/SpawnInfo.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::vector<MapDefinition*> MapDefinition::s_mapDefinitions;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
MapDefinition::MapDefinition()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void MapDefinition::InitializeMapDefinitions(const char* path)
{
	XmlDocument mapDefsXml;
	mapDefsXml.LoadFile(path);
	XmlElement* rootElement = mapDefsXml.RootElement();
	XmlElement* mapDefinitionElement = rootElement->FirstChildElement();

	while (mapDefinitionElement)
	{
		std::string elementName = mapDefinitionElement->Name();
		MapDefinition* mapDef = new MapDefinition();
		mapDef->LoadFromXmlElement(*mapDefinitionElement);
		s_mapDefinitions.push_back(mapDef);
		mapDefinitionElement = mapDefinitionElement->NextSiblingElement();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void MapDefinition::ClearDefinitions()
{
	for (MapDefinition* mapDef : s_mapDefinitions)
	{
		delete mapDef;
	}
	s_mapDefinitions.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
MapDefinition* const MapDefinition::GetMapDef(const std::string& mapDefName)
{
	for (MapDefinition* mapDef : s_mapDefinitions)
	{
		if (mapDef->m_name == mapDefName)
		{
			return mapDef;
		}
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool MapDefinition::LoadFromXmlElement(const XmlElement& xmlElement)
{
 	m_name = ParseXmlAttribute(xmlElement, "name", m_name);
	std::string imagePath = ParseXmlAttribute(xmlElement, "image", "");
	std::string shaderName = ParseXmlAttribute(xmlElement, "shader", "");
	std::string spriteSheetPath = ParseXmlAttribute(xmlElement, "spriteSheetTexture", "");
	m_spriteSheetCellCount = ParseXmlAttribute(xmlElement, "spriteSheetCellCount", m_spriteSheetCellCount);
	
	// Create Image
	if (!imagePath.empty())
	{
		m_image = Image(imagePath.c_str());
	}

	// Create Shader
	if (!shaderName.empty())
	{
		m_shader = g_theRenderer->CreateShaderOrGetFromFile((shaderName).c_str());

		if (m_shader == nullptr)
		{
			ERROR_AND_DIE("Failed to create shader after loading from xml");
		}
	}

	// Create Texture
	if (!spriteSheetPath.empty())
	{
		m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
		if (m_spriteSheetTexture == nullptr)
		{
			ERROR_AND_DIE("Failed to create or get texture after loading from xml");
		}

	}

	const XmlElement* spawnInfosElement = xmlElement.FirstChildElement("SpawnInfos");
	if (spawnInfosElement)
	{
		const XmlElement* spawnInfoElement = spawnInfosElement->FirstChildElement("SpawnInfo");
		while (spawnInfoElement)
		{
			std::string actorName = ParseXmlAttribute(*spawnInfoElement, "actor", "");
			Vec3 position = ParseXmlAttribute(*spawnInfoElement, "position", Vec3(0.f, 0.f, 0.f));
			EulerAngles orientation = ParseXmlAttribute(*spawnInfoElement, "orientation", EulerAngles());
			Vec3 velocity = ParseXmlAttribute(*spawnInfoElement, "velocity", Vec3(0.f, 0.f, 0.f));
			ActorDefinition* actorDef = nullptr;

			if (!actorName.empty())
			{
				actorDef = ActorDefinition::GetActorDef(actorName);
			}

			SpawnInfo spawnInfo(actorDef, position, orientation, velocity);

			m_spawnInfos.push_back(spawnInfo);

			spawnInfoElement = spawnInfoElement->NextSiblingElement("SpawnInfo");
		}
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

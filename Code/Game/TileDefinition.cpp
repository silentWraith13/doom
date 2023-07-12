#include "Game/TileDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TileDefinition::InitializeTileDefinitions(const char* path)
{
	XmlDocument tileDefsXml;
	tileDefsXml.LoadFile(path);
	XmlElement* rootElement = tileDefsXml.RootElement();
	XmlElement* tileDefinitionElement = rootElement->FirstChildElement();
	
	while (tileDefinitionElement)
	{
		std::string elementName = tileDefinitionElement->Name();
		TileDefinition* tileDef = new TileDefinition();
		tileDef->LoadFromXmlElement(*tileDefinitionElement);
		s_tileDefinitions.push_back(tileDef);
		tileDefinitionElement = tileDefinitionElement->NextSiblingElement();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void TileDefinition::ClearDefinitions()
{
	for (TileDefinition* tileDef : s_tileDefinitions)
	{
		delete tileDef;
	}
	s_tileDefinitions.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
TileDefinition* const TileDefinition::GetTileDef(const std::string& tileDefName)
{
	for (TileDefinition* tileDef : s_tileDefinitions)
	{
		if (tileDef->m_name == tileDefName)
		{
			return tileDef;
		}
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool TileDefinition::LoadFromXmlElement(const XmlElement& xmlElement)
{
	m_name = ParseXmlAttribute(xmlElement, "name", m_name);
	m_isSolid = ParseXmlAttribute(xmlElement, "isSolid", m_isSolid);
	m_mapImagePixelColor = ParseXmlAttribute(xmlElement, "mapImagePixelColor", Rgba8(0, 0, 0));
	m_floorSpriteCoords = ParseXmlAttribute(xmlElement, "floorSpriteCoords", m_floorSpriteCoords);
	m_ceilingSpriteCoords = ParseXmlAttribute(xmlElement, "ceilingSpriteCoords", m_ceilingSpriteCoords);
	m_wallSpriteCoords = ParseXmlAttribute(xmlElement, "wallSpriteCoords", m_wallSpriteCoords);

	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------

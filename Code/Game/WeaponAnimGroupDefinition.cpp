#include "Game/WeaponAnimGroupDefinition.hpp"
#include "Game/GameCommon.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
WeaponAnimGroupDefinitions::WeaponAnimGroupDefinitions()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool WeaponAnimGroupDefinitions::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_secondsPerFrame = ParseXmlAttribute(element, "secondsPerFrame", m_secondsPerFrame);
	m_startFrame = ParseXmlAttribute(element, "startFrame", -1);
	m_endFrame = ParseXmlAttribute(element, "endFrame", -1);
	std::string shaderName = ParseXmlAttribute(element, "shader", "");
	std::string spriteSheetPath = ParseXmlAttribute(element, "spriteSheet", "");
	m_cellCount = ParseXmlAttribute(element, "cellCount", m_cellCount);
	if (!shaderName.empty())
	{
		m_shader = g_theRenderer->CreateShaderOrGetFromFile((shaderName).c_str());
		if (m_shader == nullptr)
		{
			ERROR_AND_DIE("Failed to create shader after loading from xml");
		}
	}
	if (!spriteSheetPath.empty())
	{
		m_texture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
		if (m_texture == nullptr)
		{
			ERROR_AND_DIE("Failed to create or get texture after loading from xml");
		}

	}
	m_spriteSheet = new SpriteSheet(*m_texture, m_cellCount);
	m_spriteAnimDefs = new SpriteAnimDefinition(*m_spriteSheet, m_startFrame, m_endFrame, m_secondsPerFrame, SpriteAnimPlaybackType::ONCE);

	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------


#include "Game/SpriteAnimGroupDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteAnimationGroupDefinition::SpriteAnimationGroupDefinition()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool SpriteAnimationGroupDefinition::LoadFromXmlElement(const XmlElement& element, SpriteSheet*spriteSheet)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_scaleBySpeed = ParseXmlAttribute(element, "scaleBySpeed", m_scaleBySpeed);
	m_secondsPerFrame = ParseXmlAttribute(element, "secondsPerFrame", m_secondsPerFrame);
	std::string playbackType = ParseXmlAttribute(element, "playbackMode", "");
	if (playbackType == "Loop")
	{
		m_playbackType = SpriteAnimPlaybackType::LOOP;
	}
	if (playbackType == "Once")
	{
		m_playbackType = SpriteAnimPlaybackType::ONCE;
	}
	if (playbackType == "PingPong")
	{
		m_playbackType = SpriteAnimPlaybackType::PINGPONG;
	}

	const XmlElement* directionElement = element.FirstChildElement("Direction");
	while (directionElement)
	{
		Vec3 direction = ParseXmlAttribute(*directionElement, "vector", Vec3(0.f, 0.f, 0.f));
		direction.Normalize();
		m_directions.push_back(direction);

		const XmlElement* animationElement = directionElement->FirstChildElement("Animation");
		if (animationElement)
		{
			m_startIndex = ParseXmlAttribute(*animationElement, "startFrame", -1);
			m_endIndex = ParseXmlAttribute(*animationElement, "endFrame", -1);
			SpriteAnimDefinition spriteAnimDef = SpriteAnimDefinition(*spriteSheet, m_startIndex, m_endIndex, m_secondsPerFrame, m_playbackType);
			m_spriteAnimDefs.push_back(spriteAnimDef);
		}

		directionElement = directionElement->NextSiblingElement("Direction");
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/Vec3.hpp"
#include <map>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class SpriteAnimationGroupDefinition
{
public:
	SpriteAnimationGroupDefinition();
	bool LoadFromXmlElement(const XmlElement& element, SpriteSheet* spriteSheet);

	std::string m_name;
	bool m_scaleBySpeed = false; 
	float m_secondsPerFrame = 1.0f;
	int  m_startIndex = -1;
	int  m_endIndex = -1;
	SpriteAnimPlaybackType m_playbackType = SpriteAnimPlaybackType::ONCE;
	std::vector<Vec3> m_directions;
	std::vector<SpriteAnimDefinition> m_spriteAnimDefs;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
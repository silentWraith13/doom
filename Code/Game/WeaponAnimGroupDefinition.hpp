#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/Vec3.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class WeaponAnimGroupDefinitions
{
public:
	WeaponAnimGroupDefinitions();
	bool LoadFromXmlElement(const XmlElement& element);

	//Animation
	std::string m_name;
	Shader* m_shader = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	Texture* m_texture = nullptr;
	IntVec2 m_cellCount = IntVec2(1, 1);
	float m_secondsPerFrame = 1.0f;
	int m_startFrame = 0;
	int m_endFrame = 0;
	SpriteAnimDefinition* m_spriteAnimDefs;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
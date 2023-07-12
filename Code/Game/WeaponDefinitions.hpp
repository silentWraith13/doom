#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class WeaponAnimGroupDefinitions;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct WeaponDefinition
{
public:
	WeaponDefinition();
	static void					   InitializeWeaponDefinitions(const char* path);
	static void					   ClearDefinitions();
	static WeaponDefinition* const GetWeaponDefinition(const std::string& weaponDefinitionName);
	bool						   LoadFromXmlElement(const XmlElement& xmlElement);

	// Member variables
	std::string m_name;
	float m_refireTime = 0.0f;

	// Ray properties
	int m_rayCount = 0;
	float m_rayCone = 0.0f;
	float m_rayRange = 0.0f;
	FloatRange m_rayDamage = FloatRange(0.0f, 0.0f);
	float m_rayImpulse = 0.0f;

	// Projectile properties
	int m_projectileCount = 0;
	float m_projectileCone = 0.0f;
	float m_projectileSpeed = 0.0f;
	std::string m_projectileActor;

	// Melee properties
	int m_meleeCount = 0;
	float m_meleeRange = 0.0f;
	float m_meleeArc = 0.0f;
	FloatRange m_meleeDamage = FloatRange(0.0f, 0.0f);
	float m_meleeImpulse = 0.0f;

	// Static vector to store WeaponDefinition instances
	static std::vector<WeaponDefinition*> s_weaponDefinitions;

	//HUD
	Shader* m_shader = nullptr;
	Texture* m_baseHUDTexture = nullptr;
	Texture* m_reticleTexture = nullptr;
	Vec2 m_reticleSize = Vec2(1.f, 1.f);
	Vec2 m_spriteSize = Vec2(1.f, 1.f);
	Vec2 m_spritePivot = Vec2(0.5f, 0.f);

	//Animation
	std::string m_animationName;
	SpriteSheet* m_spriteSheet = nullptr;
	IntVec2 m_cellCount = IntVec2(1, 1);
	float m_secondsPerFrame = 1.0f;
	int m_startFrame = 0;
	int m_endFrame = 0;
	Texture* m_spriteTexture = nullptr;

	std::vector<WeaponAnimGroupDefinitions> m_weaponAnimGroupDefs;

	//Sound
	std::string m_sound;
	SoundID m_fireSound;
};
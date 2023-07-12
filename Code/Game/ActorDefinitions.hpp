#pragma once
#include "Game/SpriteAnimGroupDefinition.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <vector>


//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct ActorDefinition
{
public:
	ActorDefinition();
	static void					  InitializeActorDefinitions(const char* path);
	static void					  ClearDefinitions();
	static ActorDefinition* const GetActorDef(const std::string& actorDefName);
	bool						  LoadFromXmlElement(const XmlElement& xmlElement);
	
	// Base
	std::string m_name;
	bool m_visible = false;
	int m_health = 1;
	float m_corpseLifetime = 0.0f;
	std::string m_faction = "NEUTRAL";
	bool m_renderForward = false;
	Rgba8 m_solidColor = Rgba8(32, 32, 32);
	Rgba8 m_wireframeColor = Rgba8(192, 192, 192);
	bool  m_dieOnSpawn = false;
	bool  m_hasPointLight = true;
	// Collision
	float m_physicsRadius = 0.0f;
	float m_physicsHeight = 0.0f;
	bool m_collidesWithWorld = false;
	bool m_collidesWithActors = false;
	bool m_dieOnCollide = false;
	FloatRange m_damageOnCollide = FloatRange(0.0f, 0.0f);
	float m_impulseOnCollide = 0.0f;

	// Physics
	bool m_simulated = false;
	bool m_flying = false;
	float m_walkSpeed = 0.0f;
	float m_runSpeed = 0.0f;
	float m_drag = 0.0f;
	float m_turnSpeed = 0.0f;

	// Possession
	bool m_canBePossessed = false;
	float m_eyeHeight = 0.0f;
	float m_cameraFOVDegrees = 60.0f;

	// AI
	bool m_aiEnabled = false;
	float m_sightRadius = 0.0f;
	float m_sightAngle = 0.0f;

	// Inventory 
	std::vector<std::string> m_weapons;

	// Static vector to store ActorDefinition instances
	static std::vector<ActorDefinition*> s_actorDefinitions;

	//Visuals
	Shader*  m_shader = nullptr;
	Texture* m_spriteSheetTexture = nullptr;
	IntVec2	 m_spriteSheetCellCount = IntVec2(1, 1);
	Vec2	 m_actorSpriteSize = Vec2(1.f, 1.f);
	Vec2	 m_pivot = Vec2(0.5f, 0.5f);
	bool     m_renderRounded = false;
	bool	 m_renderLit = false;
	BillboardType m_billboardType;
	std::vector<SpriteAnimationGroupDefinition> m_animationGroupDefs;
	SpriteSheet* m_spriteSheet = nullptr;
	
	//std::map<std::string, SoundID> m_soundIDs;
	SoundID m_hurtSound = MISSING_SOUND_ID;
	SoundID m_deathSound = MISSING_SOUND_ID;
	SoundID m_attackSound = MISSING_SOUND_ID;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
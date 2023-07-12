#pragma once
#include "Game/WeaponDefinitions.hpp"
#include "Game/Actor.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/Clock.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class WeaponAnimGroupDefinitions;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Weapon
{
public:
	Weapon();
	Weapon(WeaponDefinition* definition);
	void Fire(Actor* owner);
	void Update(float deltaSeconds);
	Vec3 GetRandomDirectionInCone(const Vec3& forward, float coneAngle);
	void Render();
	void RenderHUD();
	void RenderReticle();
	void RenderWeaponAnimation(const SpriteDefinition& spriteDef);
	void PlayAnimationByName(const std::string& animationName);
	void UpdateAnimations();
	SpriteDefinition GetCurrentSpriteDef();
	
	//Member variables
	WeaponDefinition* m_definition;
	WeaponAnimGroupDefinitions* m_currentGroupAnimation = nullptr;
	float m_lastFireTime = 0.0f;
	Stopwatch* m_refireStopwatch = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	Clock* m_animationClock;
	std::string		 m_currentAnimationGroupName;
	WeaponAnimGroupDefinitions* m_currentAnimationGroup = nullptr;
	bool m_drawHUD = true;
	bool m_isFiring = false;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
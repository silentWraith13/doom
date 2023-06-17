#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/ActorUID.hpp"
#include "Game/Controller.hpp"
#include "Game/AI.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Clock.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Map;
struct Vertex_PCU;
class Player;
class SpawnInfo;
class Weapon;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Actor
{
public:
	Actor(Map* owner, const SpawnInfo&  spawnInfo, ActorUID actorUID);
	~Actor();
	void		Update(float deltaSeconds);
	void		Render();
	
	void		RenderUnlitQuad(const SpriteDefinition& spriteDef);
	void		RenderLitQuad(const SpriteDefinition& spriteDef);
	void		RenderRoundedLitQuad(const SpriteDefinition& spriteDef);
	void        DrawRedDamageQuad();
	Mat44		GetModelMatrix() const;
	void		SetCameraViewForActors();
	
	void		OnPossessed(Controller* controller);
	void		OnUnpossessed();
	std::string GetOppositeFaction() const;
	
	void		UpdatePhysics(float deltaSeconds);
	void		AddForce(const Vec3& force);
	void		AddImpulse(const Vec3& impulse);
	void		MoveInDirection(const Vec3& direction, float speed);
	void		TurnInDirection(const Vec3& direction, float maxTurnAmount);
	void		OnCollision(Actor* otherActor);
	void		Die();
	void		EquipWeapon(const std::string& weaponName);
	void		Attack();
	void		Damage(float damageAmount);

	void		UpdateAnimation(float deltaSeconds);
	void		PlayAnimationByName(const std::string& animationName);
	SpriteDefinition GetCurrentSpriteDef();
	
	//Member variables
	Camera						m_screenCam;
	Vec3						m_position;
	Vec3						m_velocity;
	Vec3						m_acceleration;
	EulerAngles					m_orientation;
	std::vector<Vertex_PNCU>	m_litVertices;
	std::vector<Vertex_PCU>		m_vertices;
	Actor*						m_owner;
	float						m_deathTick = 0.f;
	bool						m_isDead;
	bool						m_isGarbage = false;
	bool						m_isPossessed = false;
	int							m_currentHealth = 0;
	bool						m_isActorStatic = false;
	Map*						m_map = nullptr;
	ActorUID					m_actorUID;
	ActorDefinition*			m_actorDefinition = nullptr;
	Controller*					m_currentController = nullptr;
	AI*							m_AIController = nullptr;
	AI*							m_savedAIController = nullptr;
	Camera						m_camera;
	std::vector <Weapon*>		m_inventory;
	Weapon*						m_equippedWeapon = nullptr;
	SpriteSheet*				m_spriteSheet = nullptr;
	Vec2						m_spriteSize;
	Vec2						m_pivot;
	BillboardType				m_billboardType;
	
	Clock*						m_animationClock;
	std::string				    m_currentAnimationGroupName;
	SpriteAnimationGroupDefinition* m_currentAnimationGroup = nullptr;
	SoundID m_hurtSoundID;
	SoundPlaybackID m_hurtPlayBackID;
	SoundID m_deathSoundID;
	SoundPlaybackID m_deathPlayBackID;
	Stopwatch* m_animationStopwatch;
	float m_continuousDamageTimeLeft = 0.f;
	bool m_affectedByContinuousDamage = false;
	bool m_isBlinkingRed = false;
	bool m_isDamaged = false;
	float m_damageEffectDuration = 0.f;
	bool  m_isRotating = false;
	float m_rotatingSpeed = 0.f;
	float m_blastRadius = 0.0f; 
	bool m_shouldExplode = false; 
	bool m_deathAnimationPlayed = false;
	bool m_hasPointLight = false;
	Vec3 m_pointLightPosition;
	float m_pointLightIntensity;
	Rgba8 m_pointLightColor;
	float m_pointLightRadius;
	bool m_didDamageOnDeath = false;
	SoundID m_groupDmgSound;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
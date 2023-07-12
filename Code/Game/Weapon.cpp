#include "Game/Weapon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/WeaponAnimGroupDefinition.hpp"
#include "Game/SpawnInfo.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Weapon::Weapon()
{
	m_refireStopwatch = new Stopwatch(m_lastFireTime);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Weapon::Weapon( WeaponDefinition* definition)
	:m_definition(definition)
{
	m_refireStopwatch = new Stopwatch(m_lastFireTime);
	m_animationClock = new Clock(g_theApp->m_clock);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Weapon::Fire(Actor* owner)
{
	if (!m_refireStopwatch->IsStopped() && !m_refireStopwatch->HasDurationElapsed())
	{
		return;
	}

	m_refireStopwatch->SetDuration(m_definition->m_refireTime);
	m_refireStopwatch->Start();
	m_isFiring = false;
	
	if (owner->m_equippedWeapon->m_definition->m_name == "Pistol")
	{
		for (int i = 0; i < m_definition->m_rayCount; i++)
		{
			Mat44 modelMatrix = owner->GetModelMatrix();
			Vec3 fwdVec = modelMatrix.GetIBasis3D();
			Vec3 randomDirection = GetRandomDirectionInCone(fwdVec, m_definition->m_rayCone);
			float raycastDistance = m_definition->m_rayRange;
			owner->m_position.z = owner->m_position.z + owner->m_actorDefinition->m_eyeHeight;
			RaycastResult actorResult = owner->m_map->RaycastWorldActors(owner->m_position, randomDirection, raycastDistance, owner);
			FloatRange damageRange = FloatRange(m_definition->m_rayDamage);
			RandomNumberGenerator rng;
			float minDmg = damageRange.m_min;
			float maxDamage = damageRange.m_max;
			float weaponDamage = rng.RollRandomFloatInRange(minDmg, maxDamage);
			Vec3 rayStart = owner->m_position;
			Vec3 rayEnd = owner->m_position + randomDirection * raycastDistance;
			
			RaycastResult worldResult = owner->m_map->RaycastWorld(owner->m_position, randomDirection, raycastDistance, owner);
			if (worldResult.m_didImpact)
			{
				ActorDefinition* bulletHit = ActorDefinition::GetActorDef("BulletHit");
				SpawnInfo spawnInfo;
				spawnInfo.m_position = worldResult.m_impactPos;
				spawnInfo.m_actorDefinition = bulletHit;
				Actor* spawnedActor = owner->m_map->SpawnActor(spawnInfo);
				spawnedActor->m_owner->m_actorUID = owner->m_actorUID;
			}
			
			if (actorResult.m_didImpact)
			{
				float actorImpactDistance = (actorResult.m_impactPos - rayStart).GetLength();
				float worldImpactDistance = (worldResult.m_impactPos - rayStart).GetLength();

				// Check if the actor is behind the wall
				if (actorImpactDistance > worldImpactDistance)
				{
					// Actor is behind the wall, do not apply damage
					continue;
				}
				if (actorResult.m_hitActor)
				{
					actorResult.m_hitActor->m_AIController->DamagedBy(owner);
					actorResult.m_hitActor->Damage(weaponDamage);
					ActorDefinition* bloodSplatter = ActorDefinition::GetActorDef("BloodSplatter");
					SpawnInfo spawnInfo;
					spawnInfo.m_position = actorResult.m_impactPos;
					spawnInfo.m_actorDefinition = bloodSplatter;
					Actor* spawnedActor = owner->m_map->SpawnActor(spawnInfo);
					spawnedActor->m_owner->m_actorUID = owner->m_actorUID; 
				}
			}
		}
		PlayAnimationByName("Attack");
	}
	
	if (owner->m_equippedWeapon->m_definition->m_name == "PlasmaRifle")
	{
		for (int i = 0; i < m_definition->m_projectileCount; i++)
		{
			Mat44 modelMatrix = owner->GetModelMatrix();
			Vec3 fwdVec = modelMatrix.GetIBasis3D();
			Vec3 randomDirection = GetRandomDirectionInCone(fwdVec, m_definition->m_projectileCone);
			ActorDefinition* projectileActorDef = ActorDefinition::GetActorDef(m_definition->m_projectileActor);
			if (projectileActorDef)
			{
				SpawnInfo spawnInfo;
				owner->m_position.z = owner->m_position.z + owner->m_actorDefinition->m_eyeHeight - 0.04f;
				spawnInfo.m_position = owner->m_position;
				spawnInfo.m_orientation = owner->m_orientation;
				spawnInfo.m_velocity = randomDirection * m_definition->m_projectileSpeed;
				spawnInfo.m_actorDefinition = projectileActorDef;
				Actor* spawnedActor = owner->m_map->SpawnActor(spawnInfo);
				spawnedActor->m_owner->m_actorUID = owner->m_actorUID; 

 				spawnedActor->m_hasPointLight = true;
 				spawnedActor->m_pointLightPosition = Vec3(0.f, 0.f, 0.f); 
 				spawnedActor->m_pointLightIntensity = 0.1f;
 				spawnedActor->m_pointLightColor = Rgba8(0, 0, 120, 0);
 				spawnedActor->m_pointLightRadius = 0.7f;
			}
		}
		PlayAnimationByName("Attack");
	}

	if (owner->m_equippedWeapon->m_definition->m_name == "DemonMelee")
	{
		for (int i = 0; i < m_definition->m_meleeCount; ++i)
		{

			std::vector<Actor*> actorsInRange = owner->m_map->GetActorsWithinRangeAndAngle(owner, m_definition->m_meleeRange + 0.2f, m_definition->m_meleeArc+0.2f);
			for (Actor* actor : actorsInRange)
			{
				if (actor->m_actorDefinition->m_faction != owner->m_actorDefinition->m_faction)
				{
					RandomNumberGenerator rng;
					float weaponDamage = rng.RollRandomFloatInRange(m_definition->m_meleeDamage.m_min, m_definition->m_meleeDamage.m_max);
					actor->Damage(weaponDamage);
					Vec3 impulseDirection = actor->m_position - owner->m_position;
					impulseDirection.Normalize();
					actor->AddImpulse(impulseDirection * m_definition->m_meleeImpulse);
				}
			}
		}
	}

	if (owner->m_equippedWeapon->m_definition->m_name == "MarineMelee")
	{
		for (int i = 0; i < m_definition->m_meleeCount; ++i)
		{

			std::vector<Actor*> actorsInRange = owner->m_map->GetActorsWithinRangeAndAngle(owner, m_definition->m_meleeRange + 0.1f, m_definition->m_meleeArc + 0.1f);
			for (Actor* actor : actorsInRange)
			{
				if (actor->m_actorDefinition->m_faction != owner->m_actorDefinition->m_faction)
				{
					RandomNumberGenerator rng;
					float weaponDamage = rng.RollRandomFloatInRange(m_definition->m_meleeDamage.m_min, m_definition->m_meleeDamage.m_max);
					actor->Damage(weaponDamage);
					Vec3 impulseDirection = actor->m_position - owner->m_position;
					impulseDirection.Normalize();
					actor->AddImpulse(impulseDirection * m_definition->m_meleeImpulse);
				}
			}
		}
		PlayAnimationByName("Attack");
	}
	if (owner->m_equippedWeapon->m_definition->m_name == "Chainsaw")
	{
		for (int i = 0; i < m_definition->m_meleeCount; ++i)
		{

			std::vector<Actor*> actorsInRange = owner->m_map->GetActorsWithinRangeAndAngle(owner, m_definition->m_meleeRange + 0.1f, m_definition->m_meleeArc + 0.1f);
			for (Actor* actor : actorsInRange)
			{
				if (actor->m_actorDefinition->m_faction != owner->m_actorDefinition->m_faction)
				{
					RandomNumberGenerator rng;
					float weaponDamage = rng.RollRandomFloatInRange(m_definition->m_meleeDamage.m_min, m_definition->m_meleeDamage.m_max);
					actor->Damage(weaponDamage);
					ActorDefinition* bloodSplatter = ActorDefinition::GetActorDef("BloodSplatter");
					SpawnInfo spawnInfo;
					spawnInfo.m_position = actor->m_position;
					spawnInfo.m_position.z = actor->m_position.z + actor->m_actorDefinition->m_eyeHeight;
					spawnInfo.m_actorDefinition = bloodSplatter;
					Actor* spawnedActor = owner->m_map->SpawnActor(spawnInfo);
					spawnedActor->m_owner->m_actorUID = owner->m_actorUID;

					SpawnInfo spawnInfo1;
					spawnInfo1.m_position = actor->m_position;
					spawnInfo1.m_position.z = actor->m_position.z + 0.3f;
					spawnInfo1.m_actorDefinition = bloodSplatter;
					Actor* spawnedActor1 = owner->m_map->SpawnActor(spawnInfo1);
					spawnedActor1->m_owner->m_actorUID = owner->m_actorUID;

					Vec3 impulseDirection = actor->m_position - owner->m_position;
					impulseDirection.Normalize();
					actor->AddImpulse(impulseDirection * m_definition->m_meleeImpulse);
				}
			}
		}
		PlayAnimationByName("Attack");
	}
	if (owner->m_equippedWeapon->m_definition->m_name == "BFG")
	{
		for (int i = 0; i < m_definition->m_projectileCount; i++)
		{
			Mat44 modelMatrix = owner->GetModelMatrix();
			Vec3 fwdVec = modelMatrix.GetIBasis3D();
			Vec3 randomDirection = GetRandomDirectionInCone(fwdVec, m_definition->m_projectileCone);
			ActorDefinition* projectileActorDef = ActorDefinition::GetActorDef(m_definition->m_projectileActor);
			if (projectileActorDef)
			{
				SpawnInfo spawnInfo;
				owner->m_position.z = owner->m_position.z + owner->m_actorDefinition->m_eyeHeight - 0.04f;
				spawnInfo.m_position = owner->m_position;
				spawnInfo.m_orientation = owner->m_orientation;
				spawnInfo.m_velocity = randomDirection * m_definition->m_projectileSpeed;
				spawnInfo.m_actorDefinition = projectileActorDef;
				Actor* spawnedActor = owner->m_map->SpawnActor(spawnInfo);
				spawnedActor->m_owner->m_actorUID = owner->m_actorUID; // Set the owner of the projectile
			}
		}
		PlayAnimationByName("Attack");
	}

	if (owner->m_equippedWeapon->m_definition->m_name == "Minigun")
	{
		for (int i = 0; i < m_definition->m_rayCount; i++)
		{
			Mat44 modelMatrix = owner->GetModelMatrix();
			Vec3 fwdVec = modelMatrix.GetIBasis3D();
			Vec3 randomDirection = GetRandomDirectionInCone(fwdVec, m_definition->m_rayCone);
			float raycastDistance = m_definition->m_rayRange;
			owner->m_position.z = owner->m_position.z + owner->m_actorDefinition->m_eyeHeight;
			RaycastResult actorResult = owner->m_map->RaycastWorldActors(owner->m_position, randomDirection, raycastDistance, owner);
			FloatRange damageRange = FloatRange(m_definition->m_rayDamage);
			RandomNumberGenerator rng;
			float minDmg = damageRange.m_min;
			float maxDamage = damageRange.m_max;
			float weaponDamage = rng.RollRandomFloatInRange(minDmg, maxDamage);
			Vec3 rayStart = owner->m_position;
			Vec3 rayEnd = owner->m_position + randomDirection * raycastDistance;
			
			RaycastResult worldResult = owner->m_map->RaycastWorld(owner->m_position, randomDirection, raycastDistance, owner);
			if (worldResult.m_didImpact)
			{
				ActorDefinition* bulletHit = ActorDefinition::GetActorDef("BulletHit");
				SpawnInfo spawnInfo;
				spawnInfo.m_position = worldResult.m_impactPos;
				spawnInfo.m_actorDefinition = bulletHit;
				Actor* spawnedActor = owner->m_map->SpawnActor(spawnInfo);
				spawnedActor->m_owner->m_actorUID = owner->m_actorUID;
			}
			
			if (actorResult.m_didImpact)
			{
				if (actorResult.m_hitActor)
				{
					actorResult.m_hitActor->m_AIController->DamagedBy(owner);
					actorResult.m_hitActor->Damage(weaponDamage);
					ActorDefinition* bloodSplatter = ActorDefinition::GetActorDef("BloodSplatter");
					
					//1st
					SpawnInfo spawnInfo1;
					spawnInfo1.m_position = actorResult.m_impactPos;
					spawnInfo1.m_actorDefinition = bloodSplatter;
					Actor* spawnedActor = owner->m_map->SpawnActor(spawnInfo1);
					spawnedActor->m_owner->m_actorUID = owner->m_actorUID;

					//2nd
					SpawnInfo spawnInfo2;
					spawnInfo2.m_position.z = actorResult.m_impactPos.z + 0.075f;
					spawnInfo2.m_actorDefinition = bloodSplatter;
					Actor* spawnedActor2 = owner->m_map->SpawnActor(spawnInfo2);
					spawnedActor2->m_owner->m_actorUID = owner->m_actorUID;

					//3nd
					SpawnInfo spawnInfo3;
					spawnInfo3.m_position.z = actorResult.m_impactPos.x + 0.075f;
					spawnInfo3.m_actorDefinition = bloodSplatter;
					Actor* spawnedActor3 = owner->m_map->SpawnActor(spawnInfo3);
					spawnedActor3->m_owner->m_actorUID = owner->m_actorUID;
				}
			}	
		}
		PlayAnimationByName("Attack");
	}

	if (owner->m_equippedWeapon->m_definition->m_name == "Unmakyr")
	{
		for (int i = 0; i < m_definition->m_projectileCount; i++)
		{
			Mat44 modelMatrix = owner->GetModelMatrix();
			Vec3 fwdVec = modelMatrix.GetIBasis3D();
			Vec3 randomDirection = GetRandomDirectionInCone(fwdVec, m_definition->m_projectileCone);
			ActorDefinition* projectileActorDef = ActorDefinition::GetActorDef(m_definition->m_projectileActor);
			if (projectileActorDef)
			{
				SpawnInfo spawnInfo;
				owner->m_position.z = owner->m_position.z + owner->m_actorDefinition->m_eyeHeight - 0.04f;
				spawnInfo.m_position= owner->m_position;
				spawnInfo.m_orientation = owner->m_orientation;
				spawnInfo.m_velocity = randomDirection * m_definition->m_projectileSpeed;
				spawnInfo.m_actorDefinition = projectileActorDef;
				Actor* spawnedActor = owner->m_map->SpawnActor(spawnInfo);
				spawnedActor->m_owner->m_actorUID = owner->m_actorUID; 
				
			    spawnedActor->m_hasPointLight = true;
				spawnedActor->m_pointLightPosition = Vec3(0.f, 0.f, 0.f); 
				spawnedActor->m_pointLightIntensity =10.f;
				spawnedActor->m_pointLightColor = Rgba8(155, 0, 0, 0);
				spawnedActor->m_pointLightRadius = 1.f;
				
			}
		}
		PlayAnimationByName("Attack");
	}
	m_isFiring = true;
 	g_theAudio->StartSoundAt(m_definition->m_fireSound, owner->m_position);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Weapon::Update(float deltaSeconds)
{
	(void)deltaSeconds;
	UpdateAnimations();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Weapon::GetRandomDirectionInCone(const Vec3& forward, float coneAngle)
{
	Vec3 randomDirection = forward;

	// Create random rotations around X and Y axes
	RandomNumberGenerator rng;
	float randomRotationX = rng.RollRandomFloatInRange(-coneAngle, coneAngle);
	float randomRotationY = rng.RollRandomFloatInRange(-coneAngle, coneAngle);

	// Rotate the forward vector around X and Y axes using the random rotations
	EulerAngles randomEulerAngles(randomRotationX, randomRotationY, 0.0f);
	Mat44 randomRotationMatrix = randomEulerAngles.GetAsMatrix_XFwd_YLeft_ZUp();
	randomDirection = randomRotationMatrix.TransformVectorQuantity3D(randomDirection);

	return randomDirection;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Weapon::Render()
{
	RenderReticle();
	RenderHUD();
	if (m_currentAnimationGroup)
	{
		SpriteDefinition spriteDef = GetCurrentSpriteDef();
		RenderWeaponAnimation(spriteDef);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Weapon::RenderHUD()
{
	std::vector<Vertex_PCU> verts;
	AABB2 hudBounds(Vec2(0.f, 0.f), Vec2(1600.f, 150.f));
	AddVertsForAABB2D(verts, hudBounds, Rgba8(255, 255, 255));
	
	g_theRenderer->BindShader(m_definition->m_shader);
	g_theRenderer->BindTexture(m_definition->m_baseHUDTexture);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Weapon::RenderReticle()
{
	std::vector<Vertex_PCU> verts;
	AABB2 reticleBounds(Vec2(792.f, 392.f), Vec2(808.f, 408.f));
	AddVertsForAABB2D(verts, reticleBounds, Rgba8(255, 255, 255));

	g_theRenderer->BindShader(m_definition->m_shader);
	g_theRenderer->BindTexture(m_definition->m_reticleTexture);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Weapon::RenderWeaponAnimation(const SpriteDefinition& spriteDef)
{
	if (!m_currentAnimationGroup)
		return;

	std::vector<Vertex_PCU> weaponVerts;
	Vec2 weaponMins(0.f, 0.f);
	Vec2 weaponMaxs = m_definition->m_spriteSize;
	AABB2 weaponBounds = AABB2(weaponMins, weaponMaxs);
	Vec2 center = Vec2(800.f, 280.f);
	weaponBounds.SetCenter(center);

	Vec2 uvMins = Vec2(0.f, 0.f);
	Vec2 uvMaxs = Vec2(0.f, 0.f);
	spriteDef.GetUVs(uvMins, uvMaxs);

	AddVertsForAABB2D(weaponVerts, weaponBounds, Rgba8(255, 255, 255), AABB2(uvMins, uvMaxs));
	g_theRenderer->BindShader(m_currentAnimationGroup->m_shader);
	g_theRenderer->BindTexture(m_currentAnimationGroup->m_texture);
	g_theRenderer->DrawVertexArray((int)weaponVerts.size(), weaponVerts.data());

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Weapon::PlayAnimationByName(const std::string& animationName)
{
	if (animationName == m_currentAnimationGroupName)
		return;

	m_currentAnimationGroupName = animationName;

	for (int i = 0; i < m_definition->m_weaponAnimGroupDefs.size(); i++)
	{
		if (m_definition->m_weaponAnimGroupDefs[i].m_name == m_currentAnimationGroupName)
		{
			m_currentAnimationGroup = &m_definition->m_weaponAnimGroupDefs[i];
			m_animationClock->Reset();
			return;
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Weapon::UpdateAnimations()
{
	if (!m_definition || !m_animationClock || m_animationClock->IsPaused())
	{
		return;
	}

	if (!m_currentAnimationGroup && !m_definition->m_weaponAnimGroupDefs.empty())
	{
		PlayAnimationByName("Idle");
	}

	if (m_currentAnimationGroup)
	{
		float elapsedTime = m_animationClock->GetTotalSeconds();
		int animTime = (m_currentAnimationGroup->m_endFrame - m_currentAnimationGroup->m_startFrame) + 1;
		float totalDuration = m_currentAnimationGroup->m_secondsPerFrame * animTime;

 		if (elapsedTime > totalDuration)
 		{
 			PlayAnimationByName(m_definition->m_weaponAnimGroupDefs[0].m_name);
 		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteDefinition Weapon::GetCurrentSpriteDef()
{
	SpriteAnimDefinition* spriteAnimDef = nullptr;

	for (int weaponGroupDefIndex = 0; weaponGroupDefIndex < m_definition->m_weaponAnimGroupDefs.size(); weaponGroupDefIndex++)
	{
		if (m_currentAnimationGroupName == m_definition->m_weaponAnimGroupDefs[weaponGroupDefIndex].m_name)
		{
			spriteAnimDef = m_definition->m_weaponAnimGroupDefs[weaponGroupDefIndex].m_spriteAnimDefs;
		}
	}

	float elapsedTime = m_animationClock->GetTotalSeconds();
	SpriteDefinition spriteDef = spriteAnimDef->GetSpriteDefAtTime(elapsedTime);
	return spriteDef;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

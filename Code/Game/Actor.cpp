#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/Map.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/Weapon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/vertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include <cmath>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Actor::Actor(Map* owner, const SpawnInfo& spawnInfo, ActorUID actorUID)
	:m_map(owner), m_actorUID(actorUID)
{
	m_position = spawnInfo.m_position;
	m_orientation = spawnInfo.m_orientation;
	m_velocity = spawnInfo.m_velocity;
	m_actorDefinition = spawnInfo.m_actorDefinition;
	m_currentController = new Controller(m_map);
	m_AIController = new AI(m_map);
	m_currentHealth = m_actorDefinition->m_health;
	m_owner = this;
	for (const std::string& weaponName : m_actorDefinition->m_weapons)
	{
		(void)(weaponName);
		EquipWeapon(m_actorDefinition->m_weapons[0]);
	}
	SetCameraViewForActors();
	m_spriteSheet = new SpriteSheet(*m_actorDefinition->m_spriteSheetTexture, m_actorDefinition->m_spriteSheetCellCount);
	m_spriteSize = m_actorDefinition->m_actorSpriteSize;
	m_pivot = m_actorDefinition->m_pivot;
	m_billboardType = m_actorDefinition->m_billboardType;
	m_animationClock = new Clock(g_theApp->m_clock);

	m_animationStopwatch = new Stopwatch(&g_theApp->m_clock, 1.0f);
	
	if (m_actorDefinition->m_dieOnSpawn)
	{
		PlayAnimationByName("Death");
		m_isDead = true;
	}

	m_screenCam.m_mode = Camera::eMode_Orthographic;
	m_screenCam.SetOrthographicView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Actor::~Actor()
{
	m_litVertices.clear();
	m_vertices.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::Update(float deltaSeconds)
{
	if (m_isGarbage)
		return;
	
	UpdateAnimation(deltaSeconds);
	
	if (!m_isDead)
	{
		UpdatePhysics(deltaSeconds);
	}
	
	if (m_isPossessed)
	{
		SetCameraViewForActors();
	}

 	else if (m_AIController && m_actorDefinition->m_aiEnabled)
 	{
		if (!m_isDead)
		{
 			m_AIController->Update(deltaSeconds);
		}
 	}
	
	if (m_damageEffectDuration > 0.f)
	{
		m_damageEffectDuration -= deltaSeconds;
	}
	
	if (m_isDead)
	{
		m_deathTick += g_theApp->m_clock.GetDeltaSeconds();
		if (m_deathTick <= m_actorDefinition->m_corpseLifetime)
		{
			PlayAnimationByName("Death");
		}
		if (m_deathTick > m_actorDefinition->m_corpseLifetime)
		{
			m_isGarbage = true;
			return;
		}
		return;
	}

	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::Render()
{
	if (m_isGarbage)
	{
		return;
	}
	if (m_hasPointLight)
	{
		Vec3 pointLightWorldPosition = GetModelMatrix().TransformPosition3D(m_pointLightPosition);
		g_theRenderer->SetPointLightConstants(pointLightWorldPosition, m_pointLightIntensity, m_pointLightColor, m_pointLightRadius);
	}
 	if (m_actorDefinition->m_visible)
 	{
		if (!m_isDead || m_deathTick <= m_actorDefinition->m_corpseLifetime)
		{
 			SpriteDefinition spriteDef = GetCurrentSpriteDef();
 			if (!m_actorDefinition->m_renderLit)
 			{
 				RenderUnlitQuad(spriteDef);
 			}
 			if (m_actorDefinition->m_renderLit)
 			{
 				if (m_actorDefinition->m_renderRounded)
 				{
 					RenderRoundedLitQuad(spriteDef);
 				}
 				else
 				{
  					RenderLitQuad(spriteDef);
 				}
 			}
		}

		
 	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::RenderUnlitQuad(const SpriteDefinition& spriteDef)
{
	Mat44 billboardMatrix;

	if (m_map->m_player->m_cameraMode == CameraMode::FIRST_PERSON_CAMERA_MODE)
	{
		billboardMatrix = GetBillboardMatrix(m_billboardType, m_map->GetCurrentPlayerActor()->GetModelMatrix(), m_position, Vec2(1.f,1.f));
	}
	if (m_map->m_player->m_cameraMode == CameraMode::FREE_FLY_CAMERA_MODE)
	{
		billboardMatrix = GetBillboardMatrix(m_billboardType, m_map->m_player->GetModelMatrix(), m_position, Vec2(1.f, 1.f));
	}
	Vec3 pivotTranslation(0.f, -m_spriteSize.x * m_pivot.x, -m_spriteSize.y * m_pivot.y);
	Mat44 translationMatrix;
	translationMatrix.SetTranslation3D(pivotTranslation);
	Vec3 bottomLeft(0.f, 0.f, 0.f);
	Vec3 bottomRight(0.f, m_spriteSize.y, 0.f);
	Vec3 topRight(0.f, m_spriteSize.y, m_spriteSize.x);
	Vec3 topLeft(0.f, 0.f, m_spriteSize.x);
	billboardMatrix.Append(translationMatrix);
	Vec2 uvAtMins;
	Vec2 uvAtMaxs;
	spriteDef.GetUVs(uvAtMins, uvAtMaxs);
	AABB2 UVs(uvAtMins, uvAtMaxs);
	
	m_vertices.clear();
	AddVertsForQuad3D(m_vertices, bottomLeft, bottomRight, topRight, topLeft, Rgba8(255, 255, 255), UVs);
	g_theRenderer->SetModelConstants(billboardMatrix);
	g_theRenderer->BindTexture(m_actorDefinition->m_spriteSheetTexture);
	g_theRenderer->DrawVertexArray((int)m_vertices.size(), m_vertices.data());
	
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::RenderLitQuad(const SpriteDefinition& spriteDef)
{
	Mat44 billboardMatrix;
	if (m_map->m_player->m_cameraMode == CameraMode::FIRST_PERSON_CAMERA_MODE)
	{
		billboardMatrix = GetBillboardMatrix(m_billboardType, m_map->GetCurrentPlayerActor()->GetModelMatrix(), m_position, Vec2(1.f, 1.f));
	}
	if (m_map->m_player->m_cameraMode == CameraMode::FREE_FLY_CAMERA_MODE)
	{
		billboardMatrix = GetBillboardMatrix(m_billboardType, m_map->m_player->GetModelMatrix(), m_position, Vec2(1.f, 1.f));
	}
	
	Vec3 pivotTranslation(0.f, -m_spriteSize.x * m_pivot.x, -m_spriteSize.y * m_pivot.y);
	Mat44 translationMatrix;
	translationMatrix.SetTranslation3D(pivotTranslation);
	Vec3 bottomLeft(0.f, 0.f, 0.f);
	Vec3 bottomRight(0.f, m_spriteSize.y, 0.f);
	Vec3 topRight(0.f, m_spriteSize.y, m_spriteSize.x);
	Vec3 topLeft(0.f, 0.f, m_spriteSize.x);
	billboardMatrix.Append(translationMatrix);
	Vec2 uvAtMins;
	Vec2 uvAtMaxs;
	spriteDef.GetUVs(uvAtMins, uvAtMaxs);
	AABB2 UVs(uvAtMins, uvAtMaxs);
	
	m_litVertices.clear();
	AddVertsForQuad3D(m_litVertices, bottomLeft, bottomRight, topRight, topLeft, Rgba8(255, 255, 255), UVs);
	
	g_theRenderer->SetModelConstants(billboardMatrix);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthModes(DepthMode::ENABLED);
	g_theRenderer->BindShader(m_actorDefinition->m_shader);
	g_theRenderer->BindTexture(m_actorDefinition->m_spriteSheetTexture);
	g_theRenderer->DrawVertexArray((int)m_litVertices.size(), m_litVertices.data());
	
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::RenderRoundedLitQuad(const SpriteDefinition& spriteDef)
{
	Mat44 billboardMatrix;
	if (m_map->m_player->m_cameraMode == CameraMode::FIRST_PERSON_CAMERA_MODE)
	{
		billboardMatrix = GetBillboardMatrix(m_billboardType, m_map->GetCurrentPlayerActor()->GetModelMatrix(), m_position, Vec2(1.f, 1.f));
	}
	if (m_map->m_player->m_cameraMode == CameraMode::FREE_FLY_CAMERA_MODE)
	{
		billboardMatrix = GetBillboardMatrix(m_billboardType, m_map->m_player->GetModelMatrix(), m_position, Vec2(1.f, 1.f));
	}
	
	Vec3 pivotTranslation(0.f, -m_spriteSize.x * m_pivot.x, -m_spriteSize.y * m_pivot.y);
	Mat44 translationMatrix;
	translationMatrix.SetTranslation3D(pivotTranslation);
	Vec3 bottomLeft(0.f, 0.f, 0.f);
	Vec3 bottomRight(0.f, m_spriteSize.y, 0.f);
	Vec3 topRight(0.f, m_spriteSize.y, m_spriteSize.x);
	Vec3 topLeft(0.f, 0.f, m_spriteSize.x);
	billboardMatrix.Append(translationMatrix);
  	Vec2 uvAtMins;
  	Vec2 uvAtMaxs;
  	spriteDef.GetUVs(uvAtMins, uvAtMaxs);
  	AABB2 UVs(uvAtMins, uvAtMaxs);
	
	m_litVertices.clear();
	AddVertsForRoundedQuad3D(m_litVertices, bottomLeft, bottomRight, topRight, topLeft, Rgba8(255, 255, 255), UVs);

	g_theRenderer->SetModelConstants(billboardMatrix);
	g_theRenderer->SetDepthModes(DepthMode::ENABLED);
	g_theRenderer->BindShader(m_actorDefinition->m_shader);
	g_theRenderer->BindTexture(m_actorDefinition->m_spriteSheetTexture);
	g_theRenderer->DrawVertexArray((int)m_litVertices.size(), m_litVertices.data());
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::DrawRedDamageQuad()
{
	if (m_isDamaged)
	{
		float blinkFrequency = 10.f; // Adjust frequency as needed
		float alpha = (sinf(g_theApp->m_clock.GetTotalSeconds() * blinkFrequency) + 1.f) * 0.5f;
		Rgba8 redColor(255, 0, 0, static_cast<unsigned char>(alpha * 255.f));
		AABB2 redQuadBounds(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
		AddVertsForAABB2D(m_vertices, redQuadBounds, redColor);
		
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray((int)m_vertices.size(), m_vertices.data());

		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Actor::GetModelMatrix() const
{
	Mat44 rotation = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	rotation.SetTranslation3D(m_position);
	Mat44 modelMatrix = rotation;
	return modelMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::SetCameraViewForActors()
{
	m_camera.m_mode = Camera::eMode_Perspective;
	float fovDegrees = m_actorDefinition->m_cameraFOVDegrees;
	m_camera.SetPerspectiveView(g_theWindow->m_config.m_clientAspect, fovDegrees, 0.1f, 100.0f);
	Vec3 Ibasis(0.f, 0.f, 1.f);
	Vec3 JBasis(-1.f, 0.f, 0.f);
	Vec3 Kbasis(0.f, 1.f, 0.f);
	m_camera.SetRenderBasis(Ibasis, JBasis, Kbasis);

	/*DebuggerPrintf(Stringf("%.2f, %.2f, %.2f \n",).c_str());*/
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::OnPossessed(Controller* controller)
{
	m_currentController = controller;

	// Save the AI controller if the player possesses an actor with an AI controller
	if (controller == m_map->m_player && m_AIController)
	{
		m_savedAIController = m_AIController;
		m_AIController = nullptr;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::OnUnpossessed()
{
	// Restore the saved AI controller if there is one
	if (m_savedAIController)
	{
		m_AIController = m_savedAIController;
		m_savedAIController = nullptr;
	}

	m_currentController = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::UpdatePhysics(float deltaSeconds)
{
	if ((!m_isDead || !m_isGarbage) && m_actorDefinition->m_simulated)
	{
		// Apply drag force
		Vec3 dragForce = -1 * m_velocity * m_actorDefinition->m_drag;
		m_acceleration += dragForce;
		m_position += m_velocity * deltaSeconds;
		m_velocity += m_acceleration * deltaSeconds;
		m_acceleration = Vec3(0.f, 0.f, 0.f);

		// Force the position z-component to be zero if the actor is not flying
		if (!m_actorDefinition->m_flying)
		{
			m_position.z = 0.0f;
		}
		if (m_isRotating)
		{
			if (m_rotatingSpeed != 0.0f)
			{
				m_orientation.m_rollDegrees += m_rotatingSpeed * deltaSeconds;
				// Keep the roll angle in the range of [0, 360) degrees
				m_orientation.m_rollDegrees = fmod(m_orientation.m_rollDegrees, 360.0f);
			}
		}
	}
	else
		return;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::AddForce(const Vec3& force)
{
	m_acceleration += force;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::AddImpulse(const Vec3& impulse)
{
	m_velocity += impulse;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::MoveInDirection(const Vec3& direction, float speed)
{
	Vec3 force = direction * speed * m_actorDefinition->m_drag;
	AddForce(force);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::TurnInDirection(const Vec3& direction, float maxTurnAmount)
{
	Vec2 orientation = Vec2(direction.x, direction.y);
	float degrees = orientation.GetOrientationDegrees();

	m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, degrees, maxTurnAmount);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::EquipWeapon(const std::string& weaponName)
{
	WeaponDefinition* weaponDef = WeaponDefinition::GetWeaponDefinition(weaponName);
	if (weaponDef)
	{
		for ( int weapons =0; weapons < m_inventory.size(); weapons++)
		{
			if (m_inventory[weapons]->m_definition->m_name == weaponName)
			{
				m_equippedWeapon = m_inventory[weapons];
				return;
			}
		}

		Weapon* newWeapon = new Weapon(weaponDef);
		m_inventory.push_back(newWeapon);
		m_equippedWeapon = newWeapon;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::Attack()
{
	if (!m_inventory.empty())
	{
		if (m_equippedWeapon)
		{
			PlayAnimationByName("Attack");
			m_equippedWeapon->Fire(this);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::Damage(float damageAmount)
{
	m_isDamaged = true;

	if (m_isDead || m_isGarbage)
	{
		return;
	}

	m_currentHealth -= (int)damageAmount;

	if (m_currentHealth <= 0 && !m_isDead)
	{
		GetClamped(m_currentHealth, 0, m_currentHealth);
		if (m_currentHealth < 0)
		{
			m_currentHealth = 0;
		}
		Die();
	}

	else if (m_AIController)
	{
		Actor* damagingActor = m_map->GetActorByUID(m_AIController->m_targetUID);

		if (damagingActor)
		{
			m_AIController->DamagedBy(damagingActor);
		}
		else
		{
			m_AIController->m_targetUID = ActorUID::INVALID;
		}
	}
	if (!m_isDead || !m_isGarbage)
	{
		m_damageEffectDuration = 0.08f; 
	}
	
	PlayAnimationByName("Hurt");
	
	if (m_actorDefinition->m_hurtSound != MISSING_SOUND_ID)
	{
		g_theAudio->StartSoundAt(m_actorDefinition->m_hurtSound, m_position);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::UpdateAnimation(float deltaSeconds)
{
	(void)deltaSeconds;
	
	if (!m_actorDefinition || !m_animationClock || m_animationClock->IsPaused())
	{
		return;
	}

	if (!m_currentAnimationGroup && !m_actorDefinition->m_animationGroupDefs.empty())
	{
		PlayAnimationByName("Walk");
	}

	if (m_currentAnimationGroup && m_currentAnimationGroup->m_scaleBySpeed)
	{
		float speedScale = m_velocity.GetLength() / m_actorDefinition->m_runSpeed;
		m_animationClock->SetTimeScale(speedScale);
	}
	else
	{
		m_animationClock->SetTimeScale(1.0f);
	}

	if (m_currentAnimationGroup)
	{
		float elapsedTime = m_animationClock->GetTotalSeconds();
		int animTime = (m_currentAnimationGroup->m_endIndex - m_currentAnimationGroup->m_startIndex) + 1;
		float totalDuration = m_currentAnimationGroup->m_secondsPerFrame * animTime;

		if (!m_deathAnimationPlayed && m_currentAnimationGroupName == "Death" && elapsedTime >= totalDuration)
		{
			m_animationClock->Pause();
			m_deathAnimationPlayed = true;
		}
		else if (elapsedTime > totalDuration)
		{
			PlayAnimationByName(m_actorDefinition->m_animationGroupDefs[0].m_name);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::PlayAnimationByName(const std::string& animationName)
{
	if (animationName == m_currentAnimationGroupName)
		return;

	m_currentAnimationGroupName = animationName;

	for (int i = 0; i < m_actorDefinition->m_animationGroupDefs.size(); i++)
	{
		if (m_actorDefinition->m_animationGroupDefs[i].m_name == m_currentAnimationGroupName)
		{
			m_currentAnimationGroup = &m_actorDefinition->m_animationGroupDefs[i];
			if (!m_isDead)
			{
				m_animationClock->Reset();
			}
			return;
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpriteDefinition Actor::GetCurrentSpriteDef()
{
	if (!m_currentAnimationGroup)
	{
		return m_spriteSheet->GetSpriteDef(0);
	}

	const SpriteAnimDefinition* spriteAnimDef = nullptr;
	float maxDotProduct = -2.0f;
	
	Vec3 localViewDirection = (m_position - m_map->m_player->m_playerCamera.m_position);
	localViewDirection.z = 0.0f; 
	localViewDirection.Normalize();

	for (int i = 0; i < m_currentAnimationGroup->m_directions.size(); ++i)
	{
		Mat44 matrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
		Mat44 ortho = matrix.GetOrthonormalInverse();
		Vec3 transformedLocalViewDirection = ortho.TransformVectorQuantity3D(localViewDirection);
		float dotProduct = DotProduct3D(transformedLocalViewDirection, m_currentAnimationGroup->m_directions[i]);
		if (dotProduct > maxDotProduct)
		{
			maxDotProduct = dotProduct;
			spriteAnimDef = &m_currentAnimationGroup->m_spriteAnimDefs[i];
		}
	}

	float elapsedTime = m_animationClock->GetTotalSeconds();
	SpriteDefinition spriteDef = spriteAnimDef->GetSpriteDefAtTime(elapsedTime);
	return spriteDef;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::OnCollision(Actor* otherActor)
{
	if (!otherActor)
	{
		if (m_actorDefinition->m_dieOnCollide && !m_isDead)
		{
			Die();
			return;
		}
	}
	if (this == nullptr)
	{
		return;
	}

	if (otherActor && !m_deathAnimationPlayed && !m_isDead)
	{
		if (m_actorDefinition->m_name == "UnmakyrProjectile")
		{
			float blastRadius = 3.0f;
			std::vector<Actor*> actorsInRange = m_map->GetActorsWithinRangeAndAngle(this, blastRadius, 360.0f);

			for (Actor* actor : actorsInRange)
			{
				if (actor && !actor->m_isDead)
				{
					if (actor->m_actorDefinition->m_faction == "Demon")
					{
						FloatRange damageRange = m_actorDefinition->m_damageOnCollide;
						if (damageRange.m_max > 0.f)
						{
							RandomNumberGenerator rng;
							float minDmg = damageRange.m_min;
							float maxDamage = damageRange.m_max;
							float weaponDamage = rng.RollRandomFloatInRange(minDmg, maxDamage);
							actor->Damage(weaponDamage);

							ActorDefinition* unmakyr = ActorDefinition::GetActorDef("UnmakyrCollide");
							SpawnInfo spawnInfo;
							spawnInfo.m_position = actor->m_position;
							spawnInfo.m_position.z = actor->m_position.z + actor->m_actorDefinition->m_eyeHeight;
							spawnInfo.m_actorDefinition = unmakyr;
							Actor* spawnedActor = m_map->SpawnActor(spawnInfo);
							spawnedActor->m_owner->m_actorUID = actor->m_actorUID;

							ActorDefinition* blood = ActorDefinition::GetActorDef("BloodSplatter");
							SpawnInfo spawnInfo1;
							spawnInfo1.m_position = actor->m_position;
							spawnInfo1.m_position.z = actor->m_position.z + actor->m_actorDefinition->m_eyeHeight;
							spawnInfo1.m_actorDefinition = blood;
							Actor* spawnedActor1 = m_map->SpawnActor(spawnInfo1);
							spawnedActor1->m_owner->m_actorUID = actor->m_actorUID;

							ActorDefinition* explosion = ActorDefinition::GetActorDef("Explosion");
							SpawnInfo spawnInfo2;
							spawnInfo2.m_position = actor->m_position;
							spawnInfo2.m_position.z = actor->m_position.z + actor->m_actorDefinition->m_eyeHeight / 2;
							spawnInfo2.m_actorDefinition = explosion;
							Actor* spawnedActor2 = m_map->SpawnActor(spawnInfo2);
							spawnedActor2->m_owner->m_actorUID = actor->m_actorUID;
						}
					}
				}
			}

			Die();
			return;
		}

		if (m_actorDefinition->m_collidesWithActors && (m_actorDefinition->m_faction != otherActor->m_actorDefinition->m_faction))
		{
			FloatRange damageRange = m_actorDefinition->m_damageOnCollide;
			if (damageRange.m_max > 0.f)
			{
				RandomNumberGenerator rng;
				float minDmg = damageRange.m_min;
				float maxDamage = damageRange.m_max;
				float weaponDamage = rng.RollRandomFloatInRange(minDmg, maxDamage);
				otherActor->Damage(weaponDamage);

				// Add impulse to otherActor
				float impulse = m_actorDefinition->m_impulseOnCollide;
				Vec3 impulseDirection = otherActor->m_position - m_position;
				impulseDirection.Normalize();
				otherActor->AddImpulse(impulseDirection * impulse);
			}
			
		}
		
	}
	
	if (m_AIController)
	{
		m_AIController->DamagedBy(otherActor);
	}
	if (m_actorDefinition->m_dieOnCollide && !m_isDead)
	{
		Die();
		return;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Actor::Die()
{
	if (!m_deathAnimationPlayed)
	{
		PlayAnimationByName("Death");
		if (m_actorDefinition->m_deathSound != MISSING_SOUND_ID)
		{
			g_theAudio->StartSoundAt(m_actorDefinition->m_deathSound, m_position);
		}
		m_isDead = true;
		m_deathTick = 0.f;

		if (m_actorDefinition->m_name == "LostSoul" && !m_didDamageOnDeath)
		{
			m_didDamageOnDeath = true; 
			float damageRange = 5.0f; 
			float damageAmount = 20.0f; 

			std::vector<Actor*> actorsInRange = m_map->GetActorsWithinRangeAndAngle(this, damageRange, 360.0f);

			for (Actor* actor : actorsInRange)
			{
				if (actor->m_actorDefinition->m_faction != m_actorDefinition->m_faction)
				{
					actor->Damage(damageAmount);
					
					ActorDefinition* explosion = ActorDefinition::GetActorDef("Explosion");
					SpawnInfo spawnInfo;
					spawnInfo.m_position = m_position;
					spawnInfo.m_position.z = actor->m_position.z + actor->m_actorDefinition->m_eyeHeight;
					spawnInfo.m_actorDefinition = explosion;
					Actor* spawnedActor = m_map->SpawnActor(spawnInfo);
					spawnedActor->m_owner->m_actorUID = actor->m_actorUID;
				}
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::string Actor::GetOppositeFaction() const
{
	if (m_actorDefinition->m_faction == "Marine")
	{
		return "Demon";
	}

	if (m_actorDefinition->m_faction == "Demon")
	{
		return "Marine";
	}

	return m_actorDefinition->m_faction;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------

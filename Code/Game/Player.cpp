#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/WeaponDefinitions.hpp"
#include "Game/App.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Player::Player(Map* owner)
	:Controller(owner)
{
	m_cameraMode = CameraMode::FIRST_PERSON_CAMERA_MODE;
}

Player::Player()
{
	m_cameraMode = CameraMode::FIRST_PERSON_CAMERA_MODE;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Player::~Player()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::Update(float deltaSeconds)
{
	if (m_cameraMode == CameraMode::FREE_FLY_CAMERA_MODE)
	{
		UpdateFreeFlyControls(deltaSeconds);
		HandleRaycastInputs();
	}

	UpdateInput(deltaSeconds);


	UpdateCamera(deltaSeconds);
	Actor* actor = m_map->GetCurrentPlayerActor();

	HandleCrouching();

	if (actor)
	{
		if (actor->m_equippedWeapon)
		{
			actor->m_equippedWeapon->Update(deltaSeconds);	
			Vec3 i, j, k;
			m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
 			g_theAudio->UpdateListener(0, m_playerCamera.m_position, m_playerCamera.m_renderIBasis, m_playerCamera.m_renderKBasis);
 			g_theAudio->SetSoundPosition(actor->m_equippedWeapon->m_definition->m_fireSound, m_position);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::Render()
{
	if (m_cameraMode == CameraMode::FREE_FLY_CAMERA_MODE)
		return;

	g_theRenderer->BeginCamera(g_theGame->m_screenCamera);
	Actor* actor = m_map->GetCurrentPlayerActor();
	if (actor)
	{
		if (actor->m_actorDefinition->m_name == "Marine")
		{
			actor->m_equippedWeapon->Render();
		}
		RenderHealth();
	}

	DrawRedDamageQuad();
	g_theRenderer->EndCamera(g_theGame->m_screenCamera);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::UpdateFreeFlyControls(float deltaSeconds)
{
	const float MOVE_SPEED = 1.f;
	const float TURN_RATE = 90.f;
	
	m_velocity = Vec3(0.f, 0.f, 0.f);
	
	if (g_theInput->IsKeyDown('W'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_velocity += forward * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_velocity += left * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_velocity -= forward * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_velocity -= left * MOVE_SPEED;
	}

	if (g_theInput->WasKeyJustPressed('H'))
	{
		m_position = Vec3(0.f, 0.f, 0.f);
		m_orientation= EulerAngles(0.f, 0.f ,0.f);
	}

	if (g_theInput->IsKeyDown('Z'))
	{
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_velocity += up * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('C'))
	{
		Vec3 down = Vec3(0.f, 0.f, -1.f);
		m_velocity += down * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('E'))
	{
		m_orientation.m_rollDegrees += TURN_RATE * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('Q'))
	{
		m_orientation.m_rollDegrees -= TURN_RATE * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		m_velocity *= 15.0f;
	}
	
	m_position += m_velocity * deltaSeconds;
	PlayerMouseControls(deltaSeconds);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::PlayerMouseControls(float deltaSeconds)
{
	const float MOUSE_SPEED = 0.075f;
	Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
	m_orientation.m_yawDegrees -= cursorDelta.x * MOUSE_SPEED;
	m_orientation.m_pitchDegrees += cursorDelta.y * MOUSE_SPEED;

	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);

	
	m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds;
	m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds;
	m_orientation.m_rollDegrees += m_angularVelocity.m_rollDegrees * deltaSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::DrawRedDamageQuad()
{
	Actor* player = m_map->GetCurrentPlayerActor();
	if (player->m_isDamaged && player->m_damageEffectDuration > 0.f)
	{
		Rgba8 redColor(100, 0, 0, 150);
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
Mat44 Player::GetModelMatrix() const
{
	Mat44 rotation = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	rotation.SetTranslation3D(m_position);
	Mat44 modelMatrix = rotation;
	return modelMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::UpdateInput(float deltaSeconds)
{
	(void)(deltaSeconds);
	if (g_theInput->WasKeyJustPressed('F'))
	{
		if (m_cameraMode == CameraMode::FIRST_PERSON_CAMERA_MODE)
		{
			Actor* possessedActor = m_map->GetActorByUID(m_actorUID);
			if (possessedActor)
			{
				if (possessedActor->m_isDead)
				{
					return;
				}

				Vec3 actorEyePos = Vec3(possessedActor->m_position.x, possessedActor->m_position.y, possessedActor->m_position.z + possessedActor->m_actorDefinition->m_eyeHeight);
				m_position = actorEyePos;
				m_orientation = possessedActor->m_orientation;
			}
		}
		m_cameraMode = static_cast<CameraMode>((static_cast<int>(m_cameraMode) + 1) % 2);
	}

	
	

	if (m_cameraMode == CameraMode::FIRST_PERSON_CAMERA_MODE)
	{
		Actor* possessedActor = m_map->GetActorByUID(m_actorUID);
		if (possessedActor)
		{
			possessedActor->m_isPossessed = true;
			if (possessedActor->m_isDead)
			{
				return;
			}
			Vec3 iForward, jLeft, kUp;
			possessedActor->m_orientation.GetAsVectors_XFwd_YLeft_ZUp(iForward, jLeft, kUp);
			iForward.z = 0.0f; // Ignore the z-component of the forward vector
			iForward.Normalize();
			jLeft.z = 0.0f; // Ignore the z-component of the left vector
			jLeft.Normalize();
			float speed = possessedActor->m_actorDefinition->m_walkSpeed;

			Vec3 moveDirection = Vec3(0.f, 0.f, 0.f);
			// Keyboard Input
			if (g_theInput->IsKeyDown('W')) { moveDirection += iForward; }
			if (g_theInput->IsKeyDown('A')) { moveDirection += jLeft; }
			if (g_theInput->IsKeyDown('S')) { moveDirection -= iForward; }
			if (g_theInput->IsKeyDown('D')) { moveDirection -= jLeft; }

			// XBOX Input
			XboxController const& controller = g_theInput->GetController(0);

			// Move left or right, relative camera orientation
			float leftStickX = controller.GetLeftStick().GetPosition().x;
			float leftStickY = controller.GetLeftStick().GetPosition().y;

			moveDirection += jLeft * leftStickX;
			moveDirection += iForward * leftStickY;
			
			if (possessedActor->m_equippedWeapon->m_definition->m_name == "Minigun")
			{
				m_originalRayCone = possessedActor->m_equippedWeapon->m_definition->m_rayCone;
			}
			if (g_theInput->WasKeyJustPressed('V'))
			{
				m_isCrouching = !m_isCrouching;
			}
			
			// Move and turn the possessed actor
			if (moveDirection != Vec3(0.f, 0.f, 0.f))
			{
				moveDirection.Normalize();
				possessedActor->MoveInDirection(moveDirection, speed);
			}

			if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
			{
				speed = possessedActor->m_actorDefinition->m_runSpeed;
			}

			// Move and turn the possessed actor
			if (moveDirection != Vec3(0.f, 0.f, 0.f))
			{
				moveDirection.Normalize();
				possessedActor->MoveInDirection(moveDirection, speed);
			}

			const float MOUSE_SPEED = 0.075f;
			Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
			possessedActor->m_orientation.m_yawDegrees -= cursorDelta.x * MOUSE_SPEED;
			possessedActor->m_orientation.m_pitchDegrees += cursorDelta.y * MOUSE_SPEED;

			// Clamp the camera's pitch
			possessedActor->m_orientation.m_pitchDegrees = GetClamped(possessedActor->m_orientation.m_pitchDegrees, -85.f, 85.f);

			m_position += possessedActor->m_position;
			
			if (g_theInput->WasKeyJustPressed('1'))
			{
				possessedActor->EquipWeapon("Pistol");
			}
			if (g_theInput->WasKeyJustPressed('2'))
			{
				possessedActor->EquipWeapon("PlasmaRifle");
			}
			if (g_theInput->WasKeyJustPressed('3'))
			{
				possessedActor->EquipWeapon("MarineMelee");
			}
 			if (g_theInput->WasKeyJustPressed('4'))
 			{
 				possessedActor->EquipWeapon("Minigun");
 			}
			if (g_theInput->WasKeyJustPressed('5'))
			{
				possessedActor->EquipWeapon("Chainsaw");
			}
			if (g_theInput->WasKeyJustPressed('6'))
			{
				possessedActor->EquipWeapon("Unmakyr");
			}
			
			if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
			{
				possessedActor->Attack();
			}

			//XBOX
			float rightStickX = controller.GetRightStick().GetPosition().x;
			possessedActor->m_orientation.m_yawDegrees += rightStickX * MOUSE_SPEED;

			// Pitch the camera
			float rightStickY = controller.GetRightStick().GetPosition().y;
			possessedActor->m_orientation.m_pitchDegrees -= rightStickY * MOUSE_SPEED;

			// Sprint
			if (controller.IsButtonDown(XBOX_BUTTON_A))
			{
				speed = possessedActor->m_actorDefinition->m_runSpeed;
			}

			// Fire current weapon while held
			if (controller.IsButtonDown(XBOX_BUTTON_R_THUMB))
			{
				possessedActor->Attack();
			}

			// Select weapon 1
			if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
			{
				possessedActor->EquipWeapon("Pistol");
			}

			// Select weapon 2
			if (controller.WasButtonJustPressed(XBOX_BUTTON_Y))
			{
				possessedActor->EquipWeapon("PlasmaRifle");
			}
		}
		
		m_playerCamera.m_position = m_position;
		m_playerCamera.m_position = possessedActor->m_position;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::UpdateCamera(float deltaSeconds)
{
	(void)deltaSeconds;
	if (m_cameraMode == CameraMode::FIRST_PERSON_CAMERA_MODE)
	{
		Actor* possessedActor = m_map->GetActorByUID(m_actorUID);
		if (possessedActor)
		{
			if (possessedActor->m_isDead)
			{
				return;
			}
			m_playerCamera = possessedActor->m_camera;
			m_playerCamera.m_orientation = possessedActor->m_camera.m_orientation;
			m_playerCamera.m_orientation = m_orientation;
			m_playerCamera.m_position = possessedActor->m_position;
			float zPos = possessedActor->m_actorDefinition->m_eyeHeight;
			Vec3 cameraPos = Vec3(possessedActor->m_position.x, possessedActor->m_position.y, zPos);
			m_playerCamera.SetTransform(cameraPos, possessedActor->m_orientation); 
		}
		else
		{
			ERROR_AND_DIE("Something went wrong while trying to possess an actor")
		}
	}
	else if (m_cameraMode == CameraMode::FREE_FLY_CAMERA_MODE)
	{
		m_playerCamera.m_mode = Camera::eMode_Perspective;
		m_playerCamera.SetPerspectiveView(g_theWindow->m_config.m_clientAspect, 60.f, 0.1f, 100.0f);
		m_playerCamera.SetTransform(m_position, m_orientation);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::HandleRaycastInputs()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		Vec3 playerPosition = m_playerCamera.m_position;
		Vec3 playerForwardDirection = m_playerCamera.GetForwardVector();
		float raycastDistance = 10.f;

		RaycastResult3D raycastResult = m_map->RaycastAll(playerPosition, playerForwardDirection, raycastDistance);

		Vec3 rayStart = playerPosition;
		Vec3 rayEnd = playerPosition + playerForwardDirection * raycastDistance;
		DebugAddWorldLine(rayStart, rayEnd, 0.02f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);

		if (raycastResult.m_didImpact)
		{
			DebugAddWorldPoint(raycastResult.m_impactPos, 0.06f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USE_DEPTH);

			Vec3 arrowEnd = raycastResult.m_impactPos + raycastResult.m_impactNormal * 0.3f;
			DebugAddWorldArrow(raycastResult.m_impactPos, arrowEnd, 0.03f, 10.f, Rgba8(0, 0, 255), Rgba8(0, 0, 255), DebugRenderMode::USE_DEPTH);
		}
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::RenderHealth()
{
	Actor* player = m_map->GetCurrentPlayerActor();
	std::string text = std::to_string(player->m_currentHealth);
	DebugAddScreenText(text, Vec2(450.f, 70.f), 25.f, Vec2(0.5f, 0.5f), 0.f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::HandleCrouching()
{
	Actor* actor = m_map->GetCurrentPlayerActor();
	//RandomNumberGenerator rng;
	//float t = rng.RollRandomFloatInRange(1.0, 5.f);

	if (m_isCrouching)
	{
		m_playerCamera.m_position.z /= 2;

// 		std::string soundPath;
// 		if (t < 1.f)
// 		{
// 			soundPath = "Data/Audio/Crouching1.wav";
// 		}
// 		else
// 		{
// 			soundPath = "Data/Audio/Crouching2.wav";
// 		}
// 
// 		if (!m_crouchSoundPlayed)
// 		{
// 			m_crouchingSound = g_theAudio->CreateOrGetSound(soundPath);
// 			if (m_crouchingSound != MISSING_SOUND_ID)
// 			{
// 				g_theAudio->StartSoundAt(m_crouchingSound, m_position);
// 			}
// 			m_crouchSoundPlayed = true;
// 		}
	}
	else
	{
		actor->m_equippedWeapon->m_definition->m_rayCone = m_originalRayCone;
		m_playerCamera.m_position.z = m_playerCamera.m_position.z;
		m_crouchSoundPlayed = false;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

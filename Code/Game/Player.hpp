#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Controller.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Game;
class Map;
class Weapon;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class CameraMode
{
	FREE_FLY_CAMERA_MODE,
	FIRST_PERSON_CAMERA_MODE
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Player: public Controller
{
public:
	Player();
	Player(Map* owner);
	~Player();
	
	void		 Update(float deltaSeconds);
	void		 Render();
	void		 UpdateFreeFlyControls(float deltaSeconds);
	void		 PlayerMouseControls(float deltaSeconds);
	void         DrawRedDamageQuad();
	Mat44		 GetModelMatrix() const;
	void		 UpdateInput(float deltaSeconds);
	void		 UpdateCamera(float deltaSeconds);
	void		 HandleRaycastInputs();
	void         RenderHealth();
	void		 HandleCrouching();
public:
	
	//Member variables
	Camera		m_playerCamera;
	Game*		m_game = nullptr;
	Vec3		m_position;
	Vec3		m_velocity;
	std::vector<Vertex_PCU> m_vertices;
	EulerAngles	m_orientation;
	EulerAngles m_angularVelocity;
	CameraMode	m_cameraMode;
	Weapon* m_weapon;
	AABB2 bounds;
	bool m_isCrouching = false;
	bool m_crouchSoundPlayed = false;
	SoundID m_crouchingSound;
	float m_originalRayCone = 0.f;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------

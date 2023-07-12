#pragma once
#include "Game/Controller.hpp"
#include "Game/Map.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum AIState
{
	ROAMING,
	CHASING,
	ATTACKING
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class AI : public Controller
{
public:
	AI();
	AI(Map* owner);
	void Update(float deltaSeconds);
	void DamagedBy(Actor* actor);
	
	ActorUID m_targetUID = ActorUID::INVALID;
	AIState m_currentState;
	Vec3 m_destination;
	float  m_timeSinceLastRoamUpdate;
	float m_roamUpdateInterval = 2.f;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/ActorDefinitions.hpp"


//--------------------------------------------------------------------------------------------------------------------------------------------------------
class SpawnInfo
{
public:
	SpawnInfo();
	SpawnInfo(ActorDefinition* actorDefinition, const Vec3& position, const EulerAngles& orientation, const Vec3& velocity);

	//member variables
	ActorDefinition* m_actorDefinition;
	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles m_orientation;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
#include "Game/SpawnInfo.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpawnInfo::SpawnInfo(ActorDefinition* actorDefinition, const Vec3& position, const EulerAngles& orientation, const Vec3& velocity )
	:m_actorDefinition(actorDefinition), m_position(position), m_velocity(velocity), m_orientation(orientation)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
SpawnInfo::SpawnInfo()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

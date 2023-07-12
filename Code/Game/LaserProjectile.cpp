#include "Game/LaserProjectile.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
LaserProjectile::LaserProjectile(const Vec3& position, const Vec3& direction, float radius, float duration)
	: m_position(position), m_direction(direction), m_radius(radius), m_duration(duration)
{
	m_velocity = m_direction * 2.0f;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void LaserProjectile::Update(float deltSeconds)
{
	m_position += m_velocity * deltSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

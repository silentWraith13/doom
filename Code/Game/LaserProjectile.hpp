#pragma once
#include "Engine/Math/Vec3.hpp"

class LaserProjectile
{
public:
	LaserProjectile(const Vec3& position, const Vec3& direction, float radius, float duration);
	void Update(float deltSeconds);


	Vec3 m_position;
	Vec3 m_velocity;
	Vec3 m_direction;
	float m_radius;
	float m_duration;
};
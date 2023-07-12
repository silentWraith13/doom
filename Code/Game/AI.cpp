#include "Game/AI.hpp"
#include "Game/Actor.hpp"
#include "Game/Weapon.hpp"
#include "Game/Map.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AI::AI()
	:Controller()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AI::AI(Map* owner)
	: Controller(owner)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AI::Update(float deltaSeconds)
{
	Actor* controlledActor = m_map->GetActorByUID(m_actorUID);
	if (!controlledActor)
	{
		return;
	}

	m_timeSinceLastRoamUpdate += deltaSeconds;

	// Update AI destination once every few seconds
	if (m_timeSinceLastRoamUpdate > m_roamUpdateInterval)
	{
		m_timeSinceLastRoamUpdate = 0.f;

		if (m_destination == Vec3(0.f, 0.f, 0.f) || (controlledActor->m_position - m_destination).GetLength() < 0.5f)
		{
			m_destination = m_map->GetRandomPointInBounds();
		}
	}

	// Check for the closest visible enemy only if there is no current target
	if (m_targetUID == ActorUID::INVALID)
	{
		Actor* closestVisibleEnemy = m_map->GetClosestVisibleEnemy(controlledActor, controlledActor->m_actorDefinition->m_sightRadius, controlledActor->m_actorDefinition->m_sightAngle);
		if (closestVisibleEnemy)
		{
			m_targetUID = closestVisibleEnemy->m_actorUID;
		}
	}
	else
	{
		Actor* target = m_map->GetActorByUID(m_targetUID);
		if (!target || target->m_isDead || target->m_isGarbage)
		{
			m_targetUID = ActorUID::INVALID;
		}
	}

	if (m_targetUID != ActorUID::INVALID)
	{
		Actor* target = m_map->GetActorByUID(m_targetUID);
		if (target)
		{
			Vec3 targetPos = target->m_position;
			Vec3 directionToTarget = (targetPos - controlledActor->m_position);
			directionToTarget.Normalize();

			// Calculate the distance to the target
			float distanceToTarget = (targetPos - controlledActor->m_position).GetLength();

			// Check if the AI is within the melee range
			if (distanceToTarget <= controlledActor->m_equippedWeapon->m_definition->m_meleeRange)
			{
				// If within range, perform the melee attack
				controlledActor->Attack();
			}
			else
			{
				// Calculate the stopping distance based on the AI's melee range
				float stoppingDistance = controlledActor->m_equippedWeapon->m_definition->m_meleeRange;

				// Turn towards the target
				controlledActor->TurnInDirection(directionToTarget, controlledActor->m_actorDefinition->m_turnSpeed);

				// Move towards the target only if the distance to the target is greater than the stopping distance
				if (distanceToTarget > stoppingDistance)
				{
					Vec3 I, J, K;
					controlledActor->m_orientation.GetAsVectors_XFwd_YLeft_ZUp(I, J, K);
					I.Normalize();
					controlledActor->MoveInDirection(I, controlledActor->m_actorDefinition->m_runSpeed);
				}
			}
		}
		else
		{
			m_targetUID = ActorUID::INVALID;
		}
	}
	
	else
	{
		// If the AI reaches its destination or doesn't have a destination, choose a new random destination
		if (m_destination == Vec3(0.f, 0.f, 0.f) || (controlledActor->m_position - m_destination).GetLength() < 0.5f)
		{
			m_destination = m_map->GetRandomPointInBounds();
		}

		// Keep raycasting until a clear destination is found
		bool validDestination = false;
		while (!validDestination)
		{
			Vec3 directionToDestination = (m_destination - controlledActor->m_position);
			directionToDestination.Normalize();

			// Check for nearby walls using raycasting
			float wallCheckDistance = 0.8f; 
			RaycastResult raycastResult = m_map->RaycastWorldXY(controlledActor->m_position, directionToDestination, wallCheckDistance, controlledActor);

			// If there's a wall close to the AI, find a new random point
			if (raycastResult.m_didImpact)
			{
				m_destination = m_map->GetRandomPointInBounds();
			}
			else
			{
				validDestination = true;
			}
		}

		// Turn towards the valid destination
		Vec3 directionToDestination = (m_destination - controlledActor->m_position);
		directionToDestination.Normalize();
		controlledActor->TurnInDirection(directionToDestination, controlledActor->m_actorDefinition->m_turnSpeed);

		// Move towards the valid destination
		controlledActor->MoveInDirection(directionToDestination, controlledActor->m_actorDefinition->m_walkSpeed);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AI::DamagedBy(Actor* actor)
{
	Actor* controlledActor = m_map->GetActorByUID(m_actorUID);

	if (!controlledActor || !actor || actor->m_actorDefinition->m_faction == controlledActor->m_actorDefinition->m_faction)
	{
		return;
	}

	m_targetUID = actor->m_actorUID;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

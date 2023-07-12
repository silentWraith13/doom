#include "Game/Controller.hpp"
#include "Game/Map.hpp"
#include "Game/ActorUID.hpp"
#include "Game/Actor.hpp"


//--------------------------------------------------------------------------------------------------------------------------------------------------------
Controller::Controller(Map* map)
	:m_map(map)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Controller::Controller()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Controller::Possess(const ActorUID& newActorUID)
{
	// Store the old actor
	Actor* oldActor = GetActor();

	// Update the actor UID
	m_actorUID = newActorUID;

	// Get the new actor
	Actor* newActor = GetActor();

	if (newActor)
	{
		newActor->OnPossessed(this);
		m_map = newActor->m_map;
		newActor->m_isPossessed = true;
	}

	// Unpossess the old actor after possessing the new actor
	if (oldActor)
	{
		oldActor->OnUnpossessed();
		oldActor->m_isPossessed = false;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Controller::Unpossess()
{
	Actor* actor = GetActor();
	if (actor)
	{
		actor->OnUnpossessed();
	}
	m_actorUID = ActorUID::INVALID;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Actor* Controller::GetActor()
{
	return m_map->GetActorByUID(m_actorUID);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------


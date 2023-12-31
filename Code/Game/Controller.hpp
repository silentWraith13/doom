#pragma once
#include "Game/ActorUID.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Map;
class Actor;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Controller
{
public:
	Controller();
	Controller(Map* map);
	void Possess(const ActorUID& newActorUID);
	void Unpossess();
	Actor* GetActor();

	//Member variables
	Map* m_map;
	ActorUID m_actorUID;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
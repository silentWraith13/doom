#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Game/Tile.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Actor;
class ActorUID;
struct ActorDefinition;
struct MapDefinition;
class SpawnInfo;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct RaycastResult :public RaycastResult3D
{
	Actor* m_hitActor = nullptr;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Map
{
public:
	//Map functions
	Map(Game* owner);
	~Map();
	void Update(float deltaSeconds);
	void Render();
	
	//Player functions
	void InitializePlayer();
	void RenderPlayer();
	void UpdatePlayer(float deltaSeconds);
	void DeletePlayer();

	//Tile functions
	void PopulateTiles();
	void AddVertsForWalls(const Tile& tile, const TileDefinition* tileDef);
	void AddVertsForFloor(const Tile& tile, const TileDefinition* tileDef);
	void AddVertsForCeiling(const Tile& tile, const TileDefinition* tileDef);
	void RenderMap();
	int GetTileIndexForTileCoords(int tileX, int tileY);
	bool IsTileOutOfBounds(IntVec2 const& tileCoords);
	bool IsTileSolid(IntVec2 const& tileCoords);
	bool IsTileWalkable(IntVec2 const& tileCoords);
	AABB3 GetTileBoundsForTileCoords(int tileX, int tileY, float tileSize) const;
	bool IsPositionInBounds(Vec3 position, float tolerance = 0.f) const;
	bool AreCoordsInBounds(int x, int y) const;
	Tile* GetTile(int x, int y);
	TileDefinition* const GetTileDefinitionByColor(const Rgba8& color);
	Vec3 GetRandomPointInBounds();
	bool AreNeighboringTilesWalkable(IntVec2 const& tileCoords);
	//World camera
	//void SetWorldCamera();

	//Buffer functions
	void InitializeBuffers();
	void DeleteBuffers();

	//Deletion of actors
	void DeleteAllActors();

	//Enemy actors
	void InitializeEnemyActors();
	void RenderEnemyActors();

	//Actor collision vs world
	void CollideActorsWithMap();
	void CollideActorsWithMap(Actor* actor);

	//Actor vs actor collision
	bool AreOverlappingInZ(Actor* a, Actor* b);
	void CollideActors(Actor* a, Actor* b);
	void CollideActors();

	//Raycast vs tiles
	RaycastResult RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance, Actor* owner = nullptr) const;
	RaycastResult RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance, Actor* owner = nullptr);
	RaycastResult RaycastWorld(const Vec3& start, const Vec3& direction, float distance, Actor* owner = nullptr);

	//Raycast vs cylinder
	RaycastResult RaycastVsCylinderZ3D(const Vec3& start, const Vec3& direction, float distance, const Vec2& center, float minZ, float maxZ, float radius, Actor* owner = nullptr);
	RaycastResult RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance, Actor* owner = nullptr);

	//Raycast all
	RaycastResult RaycastAll(const Vec3& start, const Vec3& direction, float distance, Actor* owner = nullptr);

	Actor* SpawnActor(SpawnInfo spawnInfo);
	void SpawnPlayer();

	Actor* GetActorByUID(const ActorUID& actorUID) const;
	Actor* GetCurrentPlayerActor();
	void DeleteGarbageActors();
	Actor* GetClosestVisibleEnemy(Actor* self, float maxDistance, float fovDegrees);
	std::vector<Actor*> GetActorsWithinRangeAndAngle(Actor* self, float range, float angleDegrees);
	bool IsEnemyVisible(Actor* self, Actor* target, float sightRadius, float sightAngle);
	void DebugPossessNext();
	void RespawnPlayer();
	void UpdateLightingValues();
public:
	Game*						m_game			 = nullptr;
	Camera						m_ScreenCam;
	Player*						m_player		 = nullptr;
	IntVec2						m_dimensions	 = IntVec2(0, 0);
	MapDefinition*				m_mapDefinition  = nullptr;
	SpriteSheet*				m_spriteSheet	 = nullptr;
	TileDefinition*				m_tileDefinition = nullptr;
	VertexBuffer*				m_vertexBuffer	 = nullptr;
	IndexBuffer*				m_indexBuffer	 = nullptr;
	std::vector<Vertex_PNCU>	m_vertexes;
	std::vector<Vertex_PNCU>	m_actorVertexes;
	std::vector<unsigned int>	m_indexes;
	std::vector<Tile>			m_tiles;
	Vec3 m_sunDirection;
	float m_sunIntensity = 0.f;
	float m_ambientIntensity = 0.f;
	std::vector<Actor*> m_actors;
	unsigned int m_actorSalt;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorUID.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include <cstdlib> 
#include <ctime>  
#include <cstdio>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Map::Map(Game* owner)
	:m_game(owner)
	,m_mapDefinition(m_game->m_mapDefinition)
	,m_tileDefinition(m_game->m_tileDefinition)
{
	m_spriteSheet = new SpriteSheet(*m_game->m_mapDefinition->m_spriteSheetTexture, m_game->m_mapDefinition->m_spriteSheetCellCount);
	PopulateTiles();
	InitializeBuffers();
	
 	for (const SpawnInfo& spawnInfo : m_mapDefinition->m_spawnInfos) 
 	{
 		SpawnActor(spawnInfo);
 	}

	InitializePlayer();
	SpawnPlayer();
	
	//Lighting
	m_sunDirection = Vec3(2.f, 1.f, -1.f);
	m_sunIntensity = 0.85f;
	m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
	m_ambientIntensity = 0.35f;
	m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);


	m_ScreenCam.m_mode = Camera::eMode_Orthographic;
	m_ScreenCam.SetOrthographicView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));

	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Map::~Map()
{
	DeletePlayer();
	DeleteBuffers();
	DeleteAllActors();
	DebugRenderClear();
	DebugRenderClear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::Update(float deltaSeconds)
{
	DeleteGarbageActors();
	
	if (m_player)
	{
		m_player->Update(deltaSeconds);
	}

	for (int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i])
		{
			m_actors[i]->Update(deltaSeconds);
		}
	}
	
 	CollideActors();
	CollideActorsWithMap();

	if (g_theInput->WasKeyJustPressed('N'))
	{
		DebugPossessNext();
	}

	UpdateLightingValues();
	
	if (g_theInput->WasKeyJustPressed('K'))
	{
		DebugRenderClear();
	}

	RespawnPlayer();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::Render()
{
	g_theRenderer->BeginCamera(m_player->m_playerCamera);
	
	RenderMap();
	
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i])
		{
			if (!m_actors[i]->m_isPossessed || m_player->m_cameraMode != CameraMode::FIRST_PERSON_CAMERA_MODE)
			{
				m_actors[i]->Render();
			}
		}
	}

	DebugRenderWorld(m_player->m_playerCamera);
	
	RenderPlayer();
	g_theRenderer->EndCamera(m_player->m_playerCamera);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializePlayer()
{
	m_player = new Player(this);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderPlayer()
{
	if (m_player)
	{
		m_player->Render();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::UpdatePlayer(float deltaSeconds)
{
	if (m_player)
	{
		m_player->Update(deltaSeconds);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::DeletePlayer()
{
	delete m_player;
	m_player = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::PopulateTiles()
{
	float tileSize = 1.f;
	m_dimensions = m_mapDefinition->m_image.GetDimensions();
	int numTiles = m_dimensions.x * m_dimensions.y;
	m_tiles.reserve(numTiles);

	for (int tileY = 0; tileY < m_dimensions.y; tileY++)
	{
		
		for (int tileX = 0; tileX < m_dimensions.x; tileX++)
		{
			Rgba8 pixelColor = m_mapDefinition->m_image.GetTexelColor(IntVec2(tileX, tileY));
			TileDefinition* tileDef = GetTileDefinitionByColor(pixelColor);

			if (tileDef)
			{
				AABB3 tileBounds = GetTileBoundsForTileCoords(tileX, tileY, tileSize);
				Tile newTile(tileBounds, tileDef);
				m_tiles.push_back(newTile);

 				if (tileDef->m_floorSpriteCoords != IntVec2(-1, -1))
 				{
 					AddVertsForFloor(newTile, tileDef);
 				}

 				if (tileDef->m_ceilingSpriteCoords != IntVec2(-1, -1))
 				{
 					AddVertsForCeiling(newTile, tileDef);
 				}

 				if (tileDef->m_wallSpriteCoords != IntVec2(-1, -1) && tileDef->m_isSolid)
 				{
 					AddVertsForWalls(newTile, tileDef);
 				}
			}
			else
			{
				ERROR_AND_DIE("Tile definition not found");
			}
			
		}
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::AddVertsForWalls(const Tile& tile, const TileDefinition* tileDef)
{
	AABB3 tileBounds = tile.GetTileBounds();

	// Get the sprite coords for floor from tileDef
	IntVec2 wallSpriteCoords = tileDef->m_wallSpriteCoords;

	// Calculate spritePerRow
	int spritePerRow = m_mapDefinition->m_spriteSheetCellCount.x;

	// Calculate the UV's.
	AABB2 wallSpriteUVs = m_spriteSheet->GetSpriteUVs(wallSpriteCoords.x + (wallSpriteCoords.y * spritePerRow));
	Rgba8 color(255, 255, 255, 255);

	//Front face
	Vec3 bottomLeftFrontFace(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
	Vec3 bottomRightFrontFace(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
	Vec3 topLeftFrontFace(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
	Vec3 topRightFrontFace(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
	AddVertsForIndexQuad3D(m_vertexes, m_indexes, bottomLeftFrontFace, bottomRightFrontFace, topRightFrontFace, topLeftFrontFace, color, wallSpriteUVs);
	
	//Left face
	Vec3 bottomLeftLeftFace(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
	Vec3 bottomRightLeftFace(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
	Vec3 topLeftLeftFace(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
	Vec3 topRightLeftFace(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
	AddVertsForIndexQuad3D(m_vertexes, m_indexes, bottomLeftLeftFace, bottomRightLeftFace, topRightLeftFace, topLeftLeftFace, color, wallSpriteUVs);

	//Back face
	Vec3 bottomLeftBackFace(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
	Vec3 bottomRightBackFace(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
	Vec3 topLeftBackFace(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
	Vec3 topRightBackFace(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
	AddVertsForIndexQuad3D(m_vertexes, m_indexes, bottomLeftBackFace, bottomRightBackFace, topRightBackFace, topLeftBackFace, color, wallSpriteUVs);

	//Right face
	Vec3 bottomLeftRightFace(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
	Vec3 bottomRightRightFace(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
	Vec3 topLeftRightFace(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
	Vec3 topRightRightFace(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
	AddVertsForIndexQuad3D(m_vertexes, m_indexes, bottomLeftRightFace, bottomRightRightFace, topRightRightFace, topLeftRightFace, color, wallSpriteUVs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::AddVertsForFloor(const Tile& tile, const TileDefinition* tileDef)
{
	AABB3 tileBounds = tile.GetTileBounds();
	Vec3 bottomLeft = Vec3(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
	Vec3 bottomRight = Vec3(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
	Vec3 topLeft = Vec3(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
	Vec3 topRight = Vec3(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);

	// Get the sprite coords for floor from tileDef
	IntVec2 floorSpriteCoords = tileDef->m_floorSpriteCoords;

	// Calculate spritePerRow
	int spritePerRow = m_mapDefinition->m_spriteSheetCellCount.x;

	// Calculate the UV's.
	AABB2 floorSpriteUVs = m_spriteSheet->GetSpriteUVs(floorSpriteCoords.x + (floorSpriteCoords.y * spritePerRow));

	// Draw the floor
	Rgba8 color(255, 255, 255, 255);
	AddVertsForIndexQuad3D(m_vertexes, m_indexes, bottomLeft, bottomRight, topRight, topLeft, color, floorSpriteUVs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::AddVertsForCeiling(const Tile& tile, const TileDefinition* tileDef)
{
	AABB3 tileBounds = tile.GetTileBounds();
	Vec3 bottomLeft = Vec3(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
	Vec3 bottomRight = Vec3(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
	Vec3 topLeft = Vec3(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
	Vec3 topRight = Vec3(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
	
	// Get the sprite coords for floor from tileDef
	IntVec2 ceilingSpriteCoords = tileDef->m_ceilingSpriteCoords;

	// Calculate spritePerRow
	int spritePerRow = m_mapDefinition->m_spriteSheetCellCount.x;

	// Calculate the UV's.
	AABB2 ceilingSpriteUVs = m_spriteSheet->GetSpriteUVs(ceilingSpriteCoords.x + (ceilingSpriteCoords.y * spritePerRow));

	Rgba8 color(255, 255, 255, 255);

	//Draw the ceiling
	AddVertsForIndexQuad3D(m_vertexes, m_indexes, bottomLeft, bottomRight, topRight, topLeft, color, ceilingSpriteUVs);
	
	//AddVertsForIndexQuad3D(m_vertexes, m_indexes, Vec3(-2.f, -2.f, 0.f), Vec3(-1.f, -1.f, 0.f), Vec3(-1.f, -1.f, 1.f), Vec3(-2.f, -2.f, 1.f), color, ceilingSpriteUVs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
TileDefinition* const Map::GetTileDefinitionByColor(const Rgba8& color)
{
	for (TileDefinition* tileDef : m_tileDefinition->s_tileDefinitions)
	{
		if (tileDef->m_mapImagePixelColor == color)
		{
			return tileDef;
		}
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Map::GetRandomPointInBounds()
{
	IntVec2 tileCoords;
	do
	{
		int x = rand() % m_dimensions.x;
		int y = rand() % m_dimensions.y;
		tileCoords = IntVec2(x, y);
	} while (!IsTileWalkable(tileCoords) || !AreNeighboringTilesWalkable(tileCoords));

	// Convert tile coordinates to world coordinates
	Vec3 worldCoords = Vec3(static_cast<float>(tileCoords.x) + 0.5f, static_cast<float>(tileCoords.y) + 0.5f, 0.f);
	return worldCoords;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::AreNeighboringTilesWalkable(IntVec2 const& tileCoords)
{
	for (int dx = -1; dx <= 1; ++dx)
	{
		for (int dy = -1; dy <= 1; ++dy)
		{
			if (dx == 0 && dy == 0)
			{
				continue;
			}

			IntVec2 neighborCoords = tileCoords + IntVec2(dx, dy);
			if (!IsTileWalkable(neighborCoords))
			{
				return false;
			}
		}
	}

	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderMap()
{
	g_theRenderer->SetLightConstants(m_sunDirection, m_sunIntensity, m_ambientIntensity);
	g_theRenderer->BindShader(m_mapDefinition->m_shader);
	g_theRenderer->BindTexture(m_mapDefinition->m_spriteSheetTexture);
	g_theRenderer->DrawVertexBuffer((int)m_vertexes.size(), m_vertexBuffer, (int)m_indexes.size(), m_indexBuffer);
	g_theRenderer->BindShader(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int Map::GetTileIndexForTileCoords(int tileX, int tileY)
{
	return tileX + (tileY * m_dimensions.x);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::IsTileSolid(IntVec2 const& tileCoords)
{
	int tileIndex = GetTileIndexForTileCoords(tileCoords.x, tileCoords.y);

	if (tileIndex < 0 || tileIndex >= (m_dimensions.x * m_dimensions.y))
		return false;
	
	m_tileDefinition = m_tiles[tileIndex].m_tileDef;
	
	if (m_tiles[tileIndex].m_tileDef->m_isSolid)
	{
		return true;
	}

	else
		return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::IsTileWalkable(IntVec2 const& tileCoords)
{
	return !IsTileSolid(tileCoords);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::IsTileOutOfBounds(IntVec2 const& tileCoords) 
{
	return (tileCoords.x < 0 || tileCoords.y < 0 || tileCoords.x >= m_dimensions.x || tileCoords.y >= m_dimensions.y);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB3 Map::GetTileBoundsForTileCoords(int tileX, int tileY, float tileSize) const
{
	Vec3 tileMins = Vec3(static_cast<float>(tileX) * tileSize, static_cast<float>(tileY) * tileSize, 0.f);
	Vec3 tileMaxs = Vec3((static_cast<float>(tileX) + 1) * tileSize, (static_cast<float>(tileY) + 1) * tileSize, tileSize);
	return AABB3(tileMins, tileMaxs);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::IsPositionInBounds(Vec3 position, float tolerance /*= 0.f*/) const
{
	tolerance = 0.001f;
	Vec3 mapBoundsMins(0.f, 0.f, 0.f);
	Vec3 mapBoundsMaxs(static_cast<float>(m_dimensions.x), static_cast<float>(m_dimensions.y), 1.f);

	// Add tolerance to the map bounds
	mapBoundsMins -= Vec3(tolerance, tolerance, tolerance);
	mapBoundsMaxs += Vec3(tolerance, tolerance, tolerance);

	return (position.x >= mapBoundsMins.x) && (position.x <= mapBoundsMaxs.x) &&
		(position.y >= mapBoundsMins.y) && (position.y <= mapBoundsMaxs.y) &&
		(position.z >= mapBoundsMins.z) && (position.z <= mapBoundsMaxs.z);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::AreCoordsInBounds(int x, int y) const
{
	if (x >= 0 && (x <= m_dimensions.x - 1) && y >= 0 && (y <= m_dimensions.y - 1))
	{
		return true;
	}
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Tile* Map::GetTile(int x, int y)
{
	if (!AreCoordsInBounds(x, y))
	{
		return nullptr;
	}

	int tileIndex = GetTileIndexForTileCoords(x, y);
	return &m_tiles[tileIndex];
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializeBuffers()
{
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indexes.size() * sizeof(m_indexes[0]));
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), m_indexes.size() * sizeof(m_indexes[0]), m_indexBuffer);

	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(m_vertexes.size() * sizeof(m_vertexes[0]), sizeof(Vertex_PNCU));
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), m_vertexes.size() * sizeof(m_vertexes[0]), m_vertexBuffer);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::DeleteBuffers()
{
	m_vertexes.clear();
	m_indexes.clear();
	m_tiles.clear();

	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_indexBuffer;
	m_indexBuffer = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::DeleteAllActors()
{
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::CollideActorsWithMap()
{
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i])
		{
			if (m_actors[i]->m_actorDefinition->m_collidesWithWorld)
			{
				CollideActorsWithMap(m_actors[i]);
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Map::CollideActorsWithMap(Actor* actor)
 {
	 if (!actor)
		 return;

	 ActorDefinition* actorDef = actor->m_actorDefinition;
	 if (!actorDef)
		 return;

	 bool didCollide = false;
	 Vec3 actorPosition = actor->m_position;
	 float actorRadius = actorDef->m_physicsRadius;
	 float actorHeight = actorDef->m_physicsHeight;

	 int actorTileX = static_cast<int>(actorPosition.x);
	 int actorTileY = static_cast<int>(actorPosition.y);

	 for (int y = actorTileY - 1; y <= actorTileY + 1; y++)
	 {
		 for (int x = actorTileX - 1; x <= actorTileX + 1; x++)
		 {
			 if (!AreCoordsInBounds(x, y))
				 continue;

			 int tileIndex = GetTileIndexForTileCoords(x, y);

			 Tile& tile = m_tiles[tileIndex];

			 if (tile.GetTileDefinition()->m_isSolid)
			 {
				 AABB3 tileBounds = tile.GetTileBounds();
				 AABB2 tileBounds2D(Vec2(tileBounds.m_mins.x, tileBounds.m_mins.y), Vec2(tileBounds.m_maxs.x, tileBounds.m_maxs.y));

				 Vec2 newActorPosition2D(actorPosition.x, actorPosition.y);
				 if (PushDiscOutOfFixedAABB2D(newActorPosition2D, actorRadius, tileBounds2D))
				 {
					 didCollide = true;
					 actorPosition.y = newActorPosition2D.y;
					 actorPosition.x = newActorPosition2D.x;
					 actor->m_position = actorPosition;
				 }
			 }
		 }
	 }

	 // Check for collisions with the floor 
	 if (actorPosition.z < 0.0f)
	 {
		 didCollide = true;
		 actorPosition.z = 0.f;
		 actor->m_position = actorPosition;
	 }
	 // Check for collisions with the ceiling 
	 else if (actorPosition.z + actorHeight > 1.0f)
	 {
		 didCollide = true;
		 actorPosition.z = 1.0f - actorHeight;
		 actor->m_position = actorPosition;
	 }

	 if (didCollide)
	 {
		actor->OnCollision(nullptr);
	 }
 }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 bool Map::AreOverlappingInZ(Actor* a, Actor* b)
 {
	 ActorDefinition* aDef = a->m_actorDefinition;
	 ActorDefinition* bDef = b->m_actorDefinition;
	 if (!aDef || !bDef)
		 return false;

	 float aTop = a->m_position.z + aDef->m_physicsHeight;
	 float aBottom = a->m_position.z;

	 float bTop = b->m_position.z + bDef->m_physicsHeight;
	 float bBottom = b->m_position.z;

	 // Check if the cylinders overlap on the z-axis
	 return !(aTop < bBottom || aBottom > bTop);
 }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
  void Map::CollideActors(Actor* a, Actor* b)
  {
 	 if (!a || !b)
 		 return;
 
 	 if (AreOverlappingInZ(a, b))
 	 {
 		 // Push their 2D discs out of each other
 		 Vec2 aCenter2D(a->m_position.x, a->m_position.y);
 		 Vec2 bCenter2D(b->m_position.x, b->m_position.y);
 
 		 float aPhysicsRadius = a->m_actorDefinition->m_physicsRadius;
 		 float bPhysicsRadius = b->m_actorDefinition->m_physicsRadius;
 
 		 if (PushDiscsOutOfEachOther2D(aCenter2D, aPhysicsRadius, bCenter2D, bPhysicsRadius))
 		 {
 			 if (!a->m_isActorStatic)
 			 {
 				 a->m_position.x = aCenter2D.x;
 				 a->m_position.y = aCenter2D.y;
 			 }
 			 
			 if (!b->m_isActorStatic)
			 {
 				 b->m_position.x = bCenter2D.x;
 				 b->m_position.y = bCenter2D.y;
 			 }
 		 }
 	 }
  }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::CollideActors()
{
	for (int i = 0; i < m_actors.size(); ++i)
	{
		Actor* a = m_actors[i];
		if (!a)
			continue;

		for (int j = i + 1; j < m_actors.size(); ++j)
		{
			Actor* b = m_actors[j];
			if (!b)
				continue;

			if (a->m_actorDefinition->m_collidesWithActors && b->m_actorDefinition->m_collidesWithActors)
			{
				if (a->m_isDead || a->m_isGarbage || b->m_isDead || b->m_isGarbage)
					continue;

				// Skip collision handling if one actor is a projectile and the other is its owner
 				if ((a->m_owner->m_actorUID == b->m_owner->m_actorUID) || (a->m_owner->m_actorUID == b->m_actorUID) || (b->m_owner->m_actorUID == a->m_actorUID))
 				{
 					continue;
 				}
				if (AreOverlappingInZ(a, b))
				{
					// Push their 2D discs out of each other
					Vec2 aCenter2D(a->m_position.x, a->m_position.y);
					Vec2 bCenter2D(b->m_position.x, b->m_position.y);

					float aPhysicsRadius = a->m_actorDefinition->m_physicsRadius;
					float bPhysicsRadius = b->m_actorDefinition->m_physicsRadius;

					if (PushDiscsOutOfEachOther2D(aCenter2D, aPhysicsRadius, bCenter2D, bPhysicsRadius))
					{
						// Update actor positions after pushing them out
						a->m_position.x = aCenter2D.x;
						a->m_position.y = aCenter2D.y;
						b->m_position.x = bCenter2D.x;
						b->m_position.y = bCenter2D.y;
						b->OnCollision(a);
// 						if (b->m_AIController)
// 						{
// 							b->m_AIController->DamagedBy(a->m_owner);
// 						}
						a->OnCollision(b);
// 						if (a->m_AIController)
// 						{
// 							a->m_AIController->DamagedBy(b->m_owner);
// 						}
						
					}

				}

			}
		}
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult Map::RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance, Actor* owner /*= nullptr*/) const
{
	(void)owner;
	RaycastResult result;
	result.m_didImpact = false;
	result.m_rayStartPosition = start;
	result.m_rayDirection = direction;
	result.m_rayLength = distance;
// 	if (owner->m_isDead || owner->m_isGarbage)
// 		return result;
	float ceilZ = 1.f;
	float floorZ = 0.0f;
	if (direction.z == 0.0f)
	{
		return result;
	}

	if (direction.z > 0.0f)
	{
		float tCeil = (ceilZ - start.z) / direction.z;
		if (tCeil >= 0.0f && tCeil <= distance)
		{
			Vec3 impactPos = start + direction * tCeil;
			if (IsPositionInBounds(impactPos))
			{
				result.m_didImpact = true;
				result.m_impactDist = tCeil;
				result.m_impactPos = impactPos;
				result.m_impactNormal = Vec3(0, 0, -1);
			}
		}
	}
	else if (direction.z < 0.0f)
	{
		float tFloor = (floorZ - start.z) / direction.z;
		if (tFloor >= 0.0f && tFloor <= distance)
		{
			Vec3 impactPos = start + direction * tFloor;
			if (IsPositionInBounds(impactPos))
			{
				result.m_didImpact = true;
				result.m_impactDist = tFloor;
				result.m_impactPos = impactPos;
				result.m_impactNormal = Vec3(0, 0, 1);
			}
		}
	}

	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult Map::RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance, Actor* owner /*= nullptr*/)
{
	(void)owner;
	RaycastResult raycastResult;
	raycastResult.m_rayStartPosition = start;
	raycastResult.m_rayDirection = direction;
	raycastResult.m_rayLength = distance;
// 	if (owner->m_isDead || owner->m_isGarbage)
// 		return raycastResult;
	Vec3 forward = direction.GetNormalized();
	// Initialization
	int tileX = static_cast<int>(floor(start.x));
	int tileY = static_cast<int>(floor(start.y));

	Vec3 v = direction * distance;
	Vec2 directXY = Vec2(direction.x, direction.y).GetNormalized();
	float distanceXY = DotProduct3D(v, Vec3(directXY.x, directXY.y, 0.f));

	// Check if starting tile is solid
	if (IsTileSolid(IntVec2(tileX, tileY)))
	{
		Vec3 impactPos = start;
		if (IsPositionInBounds(impactPos))
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = 0.0f;
			raycastResult.m_impactPos = start;
			raycastResult.m_impactNormal = -1 * Vec3(forward);
			return raycastResult;
		}

	}

	// Check for division by zero when calculating tMax values
	float fwdDistPerXCrossing = directXY.x != 0.0f ? 1.0f / fabsf(directXY.x) : FLT_MAX;
	int tileStepDirectionX = (directXY.x < 0) ? -1 : 1;

	float xAtFirstXCrossing = static_cast<float>((tileX)+(tileStepDirectionX + 1) / 2);
	float xDistToFirstXCrossing = xAtFirstXCrossing - start.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

	float fwdDistPerYCrossing = directXY.y != 0.0f ? 1.0f / fabsf(directXY.y) : FLT_MAX;
	int tileStepDirectionY = (directXY.y < 0) ? -1 : 1;

	float yAtFirstYCrossing = static_cast<float>((tileY)+(tileStepDirectionY + 1) / 2);
	float yDistToFirstYCrossing = yAtFirstYCrossing - start.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	// Main Raycast Loop
	while (true)
	{
		// Check if we are outside the tile grid or at the end of the ray


		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)
		{
			if (fwdDistAtNextXCrossing > distanceXY)
			{
				break;
			}

			tileX += tileStepDirectionX;
			if (IsTileSolid(IntVec2(tileX, tileY)))
			{
				float impactDist = fwdDistAtNextXCrossing * distance / distanceXY;
				Vec3 impactPos = start + (forward * impactDist);

				if (IsPositionInBounds(impactPos))
				{
					raycastResult.m_didImpact = true;
					raycastResult.m_impactDist = fwdDistAtNextXCrossing * distance / distanceXY;
					raycastResult.m_impactPos = start + (forward * raycastResult.m_impactDist);
					raycastResult.m_impactNormal = Vec3(static_cast<float>(-tileStepDirectionX), 0, 0);
					return raycastResult;
				}
			}

			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else
		{
			if (fwdDistAtNextYCrossing > distanceXY)
			{
				break;
			}

			tileY += tileStepDirectionY;

			if (IsTileSolid(IntVec2(tileX, tileY)))
			{
				float impactDist = fwdDistAtNextYCrossing * distance / distanceXY;
				Vec3 imapctPos = start + (forward * impactDist);

				if (IsPositionInBounds(imapctPos))
				{
					raycastResult.m_didImpact = true;
					raycastResult.m_impactDist = fwdDistAtNextYCrossing * distance / distanceXY;
					raycastResult.m_impactPos = start + (forward * raycastResult.m_impactDist);
					raycastResult.m_impactNormal = Vec3(0.f, static_cast<float>(-tileStepDirectionY), 0.f);
					return raycastResult;
				}
			}

			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}

	// No impact
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = distance;
	raycastResult.m_impactPos = start + (forward * distance);
	raycastResult.m_impactNormal = Vec3(0, 0, 0);
	return raycastResult;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult Map::RaycastWorld(const Vec3& start, const Vec3& direction, float distance, Actor* owner /*= nullptr*/)
{
	(void)owner;
	RaycastResult raycastResultXY = RaycastWorldXY(start, direction, distance);
	RaycastResult raycastResultZ = RaycastWorldZ(start, direction, distance);
	if (raycastResultXY.m_didImpact && raycastResultZ.m_didImpact)
	{
		return raycastResultXY.m_impactDist <= raycastResultZ.m_impactDist ? raycastResultXY : raycastResultZ;
	}
	else if (raycastResultXY.m_didImpact)
	{
		return raycastResultXY;
	}
	else if (raycastResultZ.m_didImpact)
	{
		return raycastResultZ;
	}

	return RaycastResult();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult Map::RaycastVsCylinderZ3D(const Vec3& start, const Vec3& direction, float distance, const Vec2& center, float minZ, float maxZ, float radius, Actor* owner /*= nullptr*/)
{
	(void)(owner);
	RaycastResult result;
	result.m_rayStartPosition = start;
	result.m_rayDirection = direction;
	result.m_rayLength = distance;

	Vec3 end = start + direction * distance;

	// Check if the ray start is inside the cylinder
	if (IsPointInsideDisc2D(Vec2(start.x, start.y), center, radius) && start.z >= minZ && start.z <= maxZ)
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = start;
		result.m_impactNormal = -1 * direction;
		return result;
	}

	// Check for intersections with top and bottom planes
	float topT = (maxZ - start.z) / direction.z;
	float bottomT = (minZ - start.z) / direction.z;
	Vec2 topIntersection = Vec2(start.x, start.y) + Vec2(direction.x, direction.y) * topT;
	Vec2 bottomIntersection = Vec2(start.x, start.y) + Vec2(direction.x, direction.y) * bottomT;

	// Check if the intersections are within the XY-disc and the t values are within the range [0, distance]
	if (IsPointInsideDisc2D(topIntersection, center, radius) && topT >= 0 && topT <= distance)
	{
		result.m_didImpact = true;
		result.m_impactDist = topT;
		result.m_impactPos = Vec3(topIntersection.x, topIntersection.y, maxZ);
		result.m_impactNormal = Vec3(0, 0, 1);
	}
	else if (IsPointInsideDisc2D(bottomIntersection, center, radius) && bottomT >= 0 && bottomT <= distance)
	{
		result.m_didImpact = true;
		result.m_impactDist = bottomT;
		result.m_impactPos = Vec3(bottomIntersection.x, bottomIntersection.y, minZ);
		result.m_impactNormal = Vec3(0, 0, -1);
	}

	// Check for intersection with the cylinder's side
	Vec2 dir2D = Vec2(direction.x, direction.y);
	Vec2 start2D = Vec2(start.x, start.y);
	Vec2 diff = start2D - center;
	float a = dir2D.GetLengthSquared();
	float b = 2 * DotProduct2D(dir2D, diff);
	float c = diff.GetLengthSquared() - radius * radius;
	float discriminant = b * b - 4 * a * c;

	if (discriminant >= 0)
	{
		float t1 = (-b - sqrtf(discriminant)) / (2 * a);
		float t2 = (-b + sqrtf(discriminant)) / (2 * a);

		// Check both t1 and t2
		for (float t : {t1, t2})
		{
			if (t >= 0 && t <= distance)
			{
				Vec3 intersection = start + direction * t;
				if (intersection.z >= minZ && intersection.z <= maxZ)
				{
					float currentImpactDist = t;
					if (!result.m_didImpact || currentImpactDist < result.m_impactDist)
					{
						result.m_didImpact = true;
						result.m_impactDist = currentImpactDist;
						result.m_impactPos = intersection;
						result.m_impactNormal = Vec3(intersection.x - center.x, intersection.y - center.y, 0).GetNormalized();
					}
				}
			}
		}
	}
	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult Map::RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance, Actor* owner /*= nullptr*/)
{
	(void)(owner);
	RaycastResult closestResult;
	float closestImpactDist = std::numeric_limits<float>::max();

	for (Actor* actor : m_actors)
	{
		if (actor != nullptr && actor != owner) // Add the condition to skip the owner
		{
			ActorDefinition* actorDef = actor->m_actorDefinition;
			if (actorDef != nullptr)
			{
				Vec2 center = Vec2(actor->m_position.x, actor->m_position.y);
				float minZ = actor->m_position.z;
				float maxZ = actor->m_position.z + actorDef->m_physicsHeight;
				float radius = actorDef->m_physicsRadius;

				RaycastResult result = RaycastVsCylinderZ3D(start, direction, distance, center, minZ, maxZ, radius);

				if (result.m_didImpact)
				{
					if (result.m_impactDist < closestImpactDist)
					{
						closestResult = result;
						closestImpactDist = result.m_impactDist;
						closestResult.m_hitActor = actor;
					}
				}
			}
		}
	}

	return closestResult;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
RaycastResult Map::RaycastAll(const Vec3& start, const Vec3& direction, float distance, Actor* owner /*= nullptr*/)
{
	RaycastResult raycastResultWorld = RaycastWorld(start, direction, distance);
	RaycastResult raycastResultActors = RaycastWorldActors(start, direction, distance, owner);

	if (raycastResultWorld.m_didImpact && raycastResultActors.m_didImpact)
	{
		return raycastResultWorld.m_impactDist <= raycastResultActors.m_impactDist ? raycastResultWorld : raycastResultActors;
	}
	else if (raycastResultWorld.m_didImpact)
	{
		return raycastResultWorld;
	}
	else if (raycastResultActors.m_didImpact)
	{
		return raycastResultActors;
	}

	return RaycastResult();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Actor* Map::SpawnActor(SpawnInfo spawnInfo)
{
	// Find an empty slot in the m_actors vector
	unsigned int index = static_cast<unsigned int>(m_actors.size());
	for (unsigned int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i] == nullptr)
		{
			index = i;
			break;
		}
	}

	// Generate a new ActorUID
	unsigned int salt = m_actorSalt++;
	ActorUID actorUID(index, salt);

	// Create a new actor using the spawnInfo and the generated ActorUID
	Actor* newActor = new Actor(this, spawnInfo, actorUID);
	if (newActor->m_actorDefinition->m_aiEnabled)
	{
		newActor->m_AIController->Possess(newActor->m_actorUID);
	}

	// Add the new actor to the m_actors vector
	if (index < m_actors.size())
	{
		m_actors[index] = newActor;
	}
	else
	{
		m_actors.push_back(newActor);
	}

	return newActor;
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::SpawnPlayer()
{
	// Find all spawn points from the existing actors
	const ActorDefinition* spawnActorDef = ActorDefinition::GetActorDef("SpawnPoint");
	std::vector<Actor*> spawnPoints;

	for (Actor* actor : m_actors)
	{
		if (actor != nullptr && actor->m_actorDefinition == spawnActorDef)
		{
			spawnPoints.push_back(actor);
		}
	}

	// If there are spawn points available, choose a random one to spawn the player
	if (!spawnPoints.empty())
	{
		// Seed the random number generator with the current time
		srand(static_cast<unsigned int>(time(nullptr)));

		// Choose a random spawn point from the spawnPoints vector
		int randomIndex = rand() % spawnPoints.size();
		Actor* spawnPoint = spawnPoints[randomIndex];

		// Retrieve the Marine actor definition
		ActorDefinition* marineActorDef = ActorDefinition::GetActorDef("Marine");

		// If the Marine actor definition is found, spawn the player
		if (marineActorDef != nullptr)
		{
			// Create a SpawnInfo for the Marine actor with the spawn point's position, orientation, and velocity
			SpawnInfo marineSpawnInfo(marineActorDef, spawnPoint->m_position, spawnPoint->m_orientation, spawnPoint->m_velocity);

			// Spawn the Marine actor
			Actor* newActor = SpawnActor(marineSpawnInfo);
			m_player->Possess(newActor->m_actorUID);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Actor* Map::GetActorByUID(const ActorUID& actorUID) const
{
	if (!actorUID.IsValid())
	{
		return nullptr;
	}

	unsigned int index = actorUID.GetIndex();

	if (index >= m_actors.size())
	{
		return nullptr;
	}

	Actor* actor = m_actors[index];

	return actor;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Actor* Map::GetCurrentPlayerActor()
{
	return m_player->GetActor();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::DeleteGarbageActors()
{
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i])
		{
			if (m_actors[i]->m_isGarbage)
			{
				delete m_actors[i];
				m_actors[i] = nullptr;
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Actor* Map::GetClosestVisibleEnemy(Actor* self, float maxDistance, float fovDegrees)
{
	Actor* closestEnemy = nullptr;
	float minDistanceSq = maxDistance * maxDistance;
	Vec3 selfPosition = self->m_position;
	Vec3 selfForward = self->m_orientation.GetForwardVector();

	for (Actor* actor : m_actors)
	{
		if(!actor)
			continue;

		if (actor == self || actor->m_actorDefinition->m_faction == self->m_actorDefinition->m_faction || actor->m_actorDefinition->m_faction == "NEUTRAL" || actor->m_isDead || actor->m_isGarbage)
			continue;
	
		Vec3 actorPosition = actor->m_position;

		if (!IsPointInsideDirectedCone3D(actorPosition, selfPosition, selfForward, fovDegrees, maxDistance))
			continue;

		Vec3 toActor = actorPosition - selfPosition;
		float distanceSq = toActor.GetLengthSquared();
		if (distanceSq > minDistanceSq)
			continue;

		Vec3 toActorNormalized = toActor.GetNormalized();
		float distance = sqrtf(distanceSq);
		RaycastResult raycastResultActors = RaycastWorldActors(selfPosition, toActorNormalized, distance, self);
		RaycastResult raycastResultWorld = RaycastWorld(selfPosition, toActorNormalized, distance, self);

		if (raycastResultActors.m_didImpact && raycastResultActors.m_hitActor && (!raycastResultWorld.m_didImpact))
		{
			minDistanceSq = distanceSq;
			closestEnemy = actor;
		}
	}

	return closestEnemy;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::vector<Actor*> Map::GetActorsWithinRangeAndAngle(Actor* self, float range, float angleDegrees)
{
	std::vector<Actor*> actorsInRange;
	Vec3 selfPosition = self->m_position;
	Vec3 selfForward = self->m_orientation.GetForwardVector();

	for (Actor* actor : m_actors)
	{
		if (!actor || actor == self || actor->m_isDead || actor->m_isGarbage)
			continue;

		Vec3 actorPosition = actor->m_position;

		if (!IsPointInsideDirectedCone3D(actorPosition, selfPosition, selfForward, angleDegrees, range))
			continue;

		actorsInRange.push_back(actor);
	}

	return actorsInRange;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::IsEnemyVisible(Actor* self, Actor* target, float sightRadius, float sightAngle)
{
	Vec3 selfPosition = self->m_position;
	Vec3 targetPosition = target->m_position;
	Vec3 toTarget = targetPosition - selfPosition;
	float distanceToTarget = toTarget.GetLength();

	// Check if the target is within sight radius
	if (distanceToTarget > sightRadius)
	{
		return false;
	}

	// Check if the target is within the sight angle
	Vec3 selfForward = self->m_orientation.GetForwardVector();
	if (!IsPointInsideDirectedCone3D(targetPosition, selfPosition, selfForward, sightAngle, sightRadius))
	{
		return false;
	}

	// Check if there are any obstacles between the self and the target
	Vec3 toTargetNormalized = toTarget.GetNormalized();
	RaycastResult raycastResult = RaycastWorld(selfPosition, toTargetNormalized, distanceToTarget, self);

	// If there's no impact, the target is visible
	return !raycastResult.m_didImpact;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::DebugPossessNext()
{
 	unsigned int currentIndex = 0;
 
 	// Get the current actor possessed by the player
 	Actor* currentlyPossessedActor = GetCurrentPlayerActor();
 
 	// Find the index of the currently possessed actor
 	currentIndex = currentlyPossessedActor->m_actorUID.GetIndex();
 
 	for (unsigned int i = currentIndex + 1; i < m_actors.size(); ++i)
 	{
 		Actor* nextActor = m_actors[i];
 		if (nextActor && nextActor->m_actorDefinition->m_canBePossessed)
 		{
 			m_player->Possess(nextActor->m_actorUID);
 			return;
 			
 		}
 	}
 	for (unsigned int i = 0; i < currentIndex; ++i)
 	{
 		Actor* nextActor = m_actors[i];
 		if (nextActor && nextActor->m_actorDefinition->m_canBePossessed)
 		{
 			m_player->Possess(nextActor->m_actorUID);
 			return;
 
 		}
 	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RespawnPlayer()
{
	Actor* currentPlayer = GetCurrentPlayerActor();

	if (currentPlayer->m_actorDefinition->m_name == "Marine")
	{
		if (currentPlayer->m_deathTick <= currentPlayer->m_actorDefinition->m_corpseLifetime)
		{
			currentPlayer->m_camera.m_position.z = 0.f;
			
		}
		else
		{
			SpawnPlayer();
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::UpdateLightingValues()
{
	Vec2 alignment = Vec2(1.f, 1.f);

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		Vec2 textPos(0.f, 700.f);
		std::string text = "Sun Direction x:" + std::to_string(m_sunDirection.x) + "(F2/F3 to change)" + 
			" Sun Direction y:" + std::to_string(m_sunDirection.y) + "(F4/F5 to change)" +
			" Sun Intensity:" + std::to_string(m_sunIntensity) + "(F6/F7 to change)" +
			" Ambient Intensity:" + std::to_string(m_ambientIntensity) + "(F6/F7 to change)";

		
		DebugAddMessage(text, 2.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_sunDirection.x -= 1.f;
		m_sunDirection.Normalize();
		m_sunDirection.Normalize();
		m_sunDirection.z = -1.f;
		Vec2 textPos(0.f, 770.f);
		std::string text = "Sun Direction X Decreased: " + std::to_string(m_sunDirection.x);
		DebugAddMessage(text, 2.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_sunDirection.x += 1.f;
		m_sunDirection.Normalize();
		m_sunDirection.z = -1.f;
		Vec2 textPos(0.f, 770.f);
		std::string text = "Sun Direction X Increased: " + std::to_string(m_sunDirection.x);
		DebugAddMessage(text, 2.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_sunDirection.y -= 1.f;
		m_sunDirection.Normalize();
		m_sunDirection.z = -1.f;
		Vec2 textPos(0.f, 770.f);
		std::string text = "Sun Direction Y Decreased: " + std::to_string(m_sunDirection.y);
		DebugAddMessage(text, 2.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_sunDirection.y += 1.f;
		m_sunDirection.Normalize();
		m_sunDirection.z = -1.f;
		Vec2 textPos(0.f, 770.f);
		std::string text = "Sun Direction Y Increased: " + std::to_string(m_sunDirection.y);
		DebugAddMessage(text, 2.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_sunIntensity -= 0.05f;
		m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
		Vec2 textPos(0.f, 770.f);
		std::string text = "Sun Intensity Decreased: " + std::to_string(m_sunIntensity);
		DebugAddMessage(text, 2.f);
	}


	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_sunIntensity += 0.05f;
		m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
		Vec2 textPos(0.f, 770.f);
		std::string text = "Sun Intensity Increased: " + std::to_string(m_sunIntensity);
		DebugAddMessage(text, 2.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_ambientIntensity -= 0.05f;
		m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
		Vec2 textPos(0.f, 770.f);
		std::string text = "Ambient Intensity Decreased: " + std::to_string(m_ambientIntensity);
		DebugAddMessage(text, 2.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		m_ambientIntensity += 0.05f;
		m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
		Vec2 textPos(0.f, 770.f);
		std::string text = "Ambient Intensity Increased: " + std::to_string(m_ambientIntensity);
		DebugAddMessage(text, 2.f);
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

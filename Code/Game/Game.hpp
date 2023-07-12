#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Map;
struct MapDefinition;
struct TileDefinition;
struct  ActorDefinition;
struct WeaponDefinition;
class  Player;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class GameState
{
	NONE,
	ATTRACT,
	PLAYING,
	COUNT
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();
	
	//Game functions
	void                Startup();
	void                Render() const;
	void                Update();
	void                ShutDown();
	
	// State machine functions
	void				EnterState(GameState state);
	void				ExitState(GameState state);

	//Attract mode functions
	void				EnterAttractMode();
	void				RenderAttractMode() const;
	void				UpdateAttractMode();
	void				PrintControlsInAttractMode() const;
	void				ExitAttractMode();

	//Playing functions
	void				EnterPlayingMode();
	void				RenderPlayingMode() const;
	void				UpdatePlayingMode();
	void				ExitPlayingMode();
	void				PauseGame();
	
	//Map functions
	void				InitializeMap();
	void				RenderMap() const;
	void				UpdateMap();
	void				DeleteMap();
	
	//Set camera values
	void				SetScreenCamera();
	void				SetText2DCamera();
	
	//2d camera text
	void				RenderClockInformationText() const;

	//Initialize definitions
	void				InitializeProjectileActorDefinitions();
	void				InitializeWeaponDefinitions();
	void				InitializeActorDefinitions();
	void				InitializeMapDefinitions();
	void				InitializeTileDefinitions();

	//Load Assets
	void				LoadAssets();
	void				LoadGameConfig();

	SoundPlaybackID PlayMusic(const std::string& musicFilePath, bool loop);

	//Member variables
	Camera				m_screenCamera;
	Camera				m_textCamera;

	Map*				m_currentMap       = nullptr;
	
	GameState			m_currentgameState = GameState::ATTRACT;
	GameState			m_nextGameState	   = GameState::ATTRACT;

	MapDefinition*		m_mapDefinition = nullptr;
	TileDefinition*		m_tileDefinition = nullptr;
	ActorDefinition*	m_actorDefinition = nullptr;
	WeaponDefinition*	m_weaponDefinition = nullptr;
	std::string			m_defaultMapName  = "";
	Player*				m_player;

	//Audio
	std::string m_mainMenuMusic;
	std::string m_gameMusic;
	std::string m_buttonClickSound;
	float m_musicVolume;
	SoundPlaybackID m_mainMenuMusicPlaybackID;
	SoundPlaybackID m_gameMusicPlaybackID;
};
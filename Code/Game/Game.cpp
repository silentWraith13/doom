#include "Game/Game.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Game/WeaponDefinitions.hpp"
#include "Game/Player.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Game::Game()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::Startup()
{
	LoadAssets();
	EnterState(GameState::ATTRACT);
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	Rgba8 clearColor{ 0, 0, 0, 255 };

	g_theRenderer->ClearScreen(clearColor);

	switch (m_currentgameState)
	{
	
	case GameState::ATTRACT:
		 RenderAttractMode();
		break;

	case GameState::PLAYING:
		 RenderPlayingMode();
		break;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::Update()
{
	if (m_currentgameState != m_nextGameState)
	{
		ExitState(m_currentgameState);
		m_currentgameState = m_nextGameState;
		EnterState(m_currentgameState);
	}

	switch (m_currentgameState)
	{
	
	case GameState::ATTRACT:
		 UpdateAttractMode();
		break;
	
	case GameState::PLAYING:
		 UpdatePlayingMode();
		 PauseGame();
		break;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	DeleteMap();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::EnterState(GameState state)
{
	switch (state)
	{
	
	case GameState::ATTRACT:
		 EnterAttractMode();
		break;
	
	case GameState::PLAYING:
		 EnterPlayingMode();
		break;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::ExitState(GameState state)
{
	switch (state)
	{
	
	case GameState::ATTRACT:
		 ExitAttractMode();
		break;
	
	case GameState::PLAYING:
		 ExitPlayingMode();
		break;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::EnterAttractMode()
{
	m_currentgameState = GameState::ATTRACT;
	SetScreenCamera();
	m_mainMenuMusicPlaybackID = PlayMusic(m_mainMenuMusic, true);

	if (g_theAudio->IsPlaying(m_gameMusicPlaybackID)) 
	{
		g_theAudio->StopSound(m_gameMusicPlaybackID);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	std::vector<Vertex_PCU> verts;
	verts.reserve(1000);
	AABB2 attractModeBounds(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
	std::string textureName = "Doom";
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile(("Data/Images/" + textureName + ".png").c_str());
	AddVertsForAABB2D(verts, attractModeBounds, Rgba8(255, 255, 255));
	g_theRenderer->BindTexture(texture);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
	g_theRenderer->EndCamera(m_screenCamera);
 	PrintControlsInAttractMode();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateAttractMode()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		SoundID clickSoundID = g_theAudio->CreateOrGetSound(m_buttonClickSound, false);
		g_theAudio->StartSound(clickSoundID);
		m_nextGameState = GameState::PLAYING;
	}
	else if (m_currentgameState == GameState::ATTRACT && g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		SoundID clickSoundID = g_theAudio->CreateOrGetSound(m_buttonClickSound, false);
		g_theAudio->StartSound(clickSoundID);
		EventArgs args;
		FireEvent("Quit", args);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::PrintControlsInAttractMode() const
{
	std::vector<Vertex_PCU> inputLineVerts;
	BitmapFont* textFont = nullptr;
	std::string fontName = "SquirrelFixedFont";
	textFont = g_theRenderer->CreateOrGetBitmapFontFromFile(("Data/Fonts/" + fontName).c_str());
	AABB2 bounds(Vec2(450.f, 150.f), Vec2(800.f, 400.f));
	std::string text = "Press Space to enter";
	textFont->AddVertsForTextInBox2D(inputLineVerts, bounds, 30.f, text, Rgba8(255, 0, 0, 255), 0.9f, Vec2(0.f, 1.f), TextDrawMode::OVERRUN, 9999);
	g_theRenderer->BindTexture(&textFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)inputLineVerts.size(), inputLineVerts.data());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::ExitAttractMode()
{
	m_nextGameState = GameState::PLAYING;
	g_theAudio->StopSound(m_mainMenuMusicPlaybackID);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::EnterPlayingMode()
{
	m_currentgameState = GameState::PLAYING;
	SetText2DCamera();
	m_gameMusicPlaybackID = PlayMusic(m_gameMusic, true);

	if (g_theAudio->IsPlaying(m_mainMenuMusicPlaybackID)) 
	{
		g_theAudio->StopSound(m_mainMenuMusicPlaybackID);
	}
	g_theAudio->SetNumListeners(1);
	InitializeMap();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::RenderPlayingMode() const
{
	RenderMap();
	RenderClockInformationText();
	DebugRenderScreen(m_textCamera);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::UpdatePlayingMode()
{
	UpdateMap();
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		SoundID clickSoundID = g_theAudio->CreateOrGetSound(m_buttonClickSound, false);
		g_theAudio->StartSound(clickSoundID);
		m_nextGameState = GameState::ATTRACT;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::ExitPlayingMode()
{
	m_nextGameState = GameState::ATTRACT;
	DeleteMap();
	SetScreenCamera();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::PauseGame()
{
	if (g_theInput->WasKeyJustPressed('P'))
	{
		g_theApp->m_clock.TogglePause();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::InitializeMap()
{
	if (m_currentMap == nullptr)
	{
		m_currentMap = new Map(this);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::RenderMap() const
{
	if (m_currentMap)
	{
		m_currentMap->Render();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateMap()
{
	if (m_currentMap)
	{
		m_currentMap->Update(g_theApp->m_clock.GetDeltaSeconds());
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::DeleteMap()
{
	if (m_currentMap)
	{
		delete m_currentMap;
		m_currentMap = nullptr;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::SetScreenCamera()
{
	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::SetText2DCamera()
{
	m_textCamera.m_mode = Camera::eMode_Orthographic;
	m_textCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::RenderClockInformationText() const
{
	//Clock info
	Vec2 textPos(1070.f, 770.f);
	float totalTime = g_theApp->m_clock.GetTotalSeconds();
	float fps = 1.0f / g_theApp->m_clock.GetDeltaSeconds();
	float timeScale = g_theApp->m_clock.GetTimeScale();
	float cellHeight = 12.0f;
	Vec2 alignment(1.f, 1.f);
	std::string text = "Time:" + std::to_string(totalTime) + "s, FPS:" + std::to_string(fps) + ", Scale:" + std::to_string(timeScale);
	DebugAddScreenText(text, textPos, cellHeight, alignment, 0.f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::InitializeProjectileActorDefinitions()
{
	m_actorDefinition = new ActorDefinition();
	std::string path = "Data/Definitions/ProjectileActorDefinitions.xml";
	ActorDefinition::InitializeActorDefinitions(path.c_str());
	ActorDefinition::s_actorDefinitions;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::InitializeWeaponDefinitions()
{
	m_weaponDefinition = new WeaponDefinition();
	std::string path = "Data/Definitions/WeaponDefinitions.xml";
 	WeaponDefinition::InitializeWeaponDefinitions(path.c_str());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::InitializeActorDefinitions()
{
	std::string path = "Data/Definitions/ActorDefinitions.xml";
	ActorDefinition::InitializeActorDefinitions(path.c_str());
	ActorDefinition::s_actorDefinitions;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::InitializeMapDefinitions()
{
	m_mapDefinition = new MapDefinition();
	std::string path = "Data/Definitions/MapDefinitions.xml";
	m_mapDefinition->InitializeMapDefinitions(path.c_str());
	m_mapDefinition = MapDefinition::GetMapDef(m_defaultMapName);

	if (!m_mapDefinition) 
	{
		ERROR_AND_DIE("Failed to find the specified map definition.");
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::InitializeTileDefinitions()
{
	m_tileDefinition = new TileDefinition();
	std::string path = "Data/Definitions/TileDefinitions.xml";
	m_tileDefinition->InitializeTileDefinitions(path.c_str());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	LoadGameConfig();
	InitializeProjectileActorDefinitions();
	InitializeWeaponDefinitions();
	InitializeActorDefinitions();
	InitializeMapDefinitions();
	InitializeTileDefinitions();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::LoadGameConfig()
{
	std::string configPath = "Data/GameConfig.xml";
	XmlDocument configXml;
	configXml.LoadFile(configPath.c_str());
	XmlElement* rootElement = configXml.RootElement();
 	m_defaultMapName = ParseXmlAttribute(*rootElement, "defaultMap", m_defaultMapName);
	m_mainMenuMusic = ParseXmlAttribute(*rootElement, "mainMenuMusic", m_mainMenuMusic);
	m_gameMusic = ParseXmlAttribute(*rootElement, "gameMusic", m_gameMusic);
	m_buttonClickSound = ParseXmlAttribute(*rootElement, "buttonClickSound", m_buttonClickSound);
	m_musicVolume = ParseXmlAttribute(*rootElement, "musicVolume", m_musicVolume);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
SoundPlaybackID Game::PlayMusic(const std::string& musicFilePath, bool loop)
{
	SoundID musicID = g_theAudio->CreateOrGetSound(musicFilePath, false);
	return g_theAudio->StartSound(musicID, loop, m_musicVolume);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/vertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Texture.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class RandomNumberGenerator;
class App;
class Game;
class SpriteSheet;
class Texture;
class EventSystem;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
extern Renderer* g_theRenderer;
extern App* g_theApp;
extern RandomNumberGenerator g_rng;
extern Game* g_theGame;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern SpriteSheet* g_terrainSprites;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr float     WORLD_SIZE_X = 200.0f;
constexpr float     WORLD_SIZE_Y = 100.0f;
constexpr float		SCREEN_SIZE_X = 1600.0f;
constexpr float		SCREEN_SIZE_Y = 800.0f;
constexpr float     WORLD_CENTER_X = WORLD_SIZE_X * 0.5f;
constexpr float     WORLD_CENTER_Y = WORLD_SIZE_Y * 0.5f;
IntVec2   const		TERRAIN_SPRITE_LAYOUT = IntVec2(8, 8);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//Debug functions(also draws with adding verts)
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void DebugDrawLine(Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);

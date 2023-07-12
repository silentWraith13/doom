#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSytem.hpp"
#include "Engine/Core/Clock.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Game;
class InputSystem;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class App
{
public:
	App();
	~App();

	//App functions
	void             Startup();
	void             Shutdown();
	void             RunFrame();
	void             BeginFrame();
	void             Update();
	void             Render() const;
	void             EndFrame();
	void             Run();
	
	//quitting functions
	bool             IsQuitting() const { return m_isQuitting; }
	bool             HandleQuitRequested();
	static bool		 Event_Quit(EventArgs& args);

	//Dev console functions
	void			SetDevConsoleCamera();
	void			PrintDevConsoleCommands();

public:
	//Member variables
	bool            m_isQuitting = false;
	Camera          m_devConsoleCamera;
	Clock			m_clock;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------

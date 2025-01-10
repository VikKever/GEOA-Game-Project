#pragma once
#include "structs.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include <memory>
#include <vector>

class Ball;
class BoundingBox;
class Cue;
class Hole;

class Game
{
public:
	explicit Game( const Window& window );
	Game( const Game& other ) = delete;
	Game& operator=( const Game& other ) = delete;
	Game(Game&& other) = delete;
	Game& operator=(Game&& other) = delete;

	~Game();

	void Run( );

	void Update(float elapsedSec);

	void Draw() const;

	// Event handling
	void ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
	{

	}
	void ProcessKeyUpEvent(const SDL_KeyboardEvent& e)
	{

	}
	void ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e)
	{
		
	}
	void ProcessMouseDownEvent(const SDL_MouseButtonEvent& e)
	{
		
	}
	void ProcessMouseUpEvent(const SDL_MouseButtonEvent& e)
	{
		
	}

	const Rectf& GetViewPort() const
	{
		return m_Viewport;
	}

private:
	// DATA MEMBERS
	// The window properties
	const Window m_Window;
	const Rectf m_Viewport;
	// The window we render to
	SDL_Window* m_pWindow;
	// OpenGL context
	SDL_GLContext m_pContext;
	// Init info
	bool m_Initialized;
	// Prevent timing jumps when debugging
	const float m_MaxElapsedSeconds;
	
	// FUNCTIONS
	void InitializeGameEngine( );
	void CleanupGameEngine( );

	void SetupRedBalls();
	void InitWhiteBall();
	void SetupHoles();
	void CheckBallsRolling();
	bool FallsInHole(const Ball& ball);

	const Rectf m_playArea;
	std::unique_ptr<BoundingBox> m_pBoundingBox;

	std::vector<Hole> m_holes;

	std::vector<Ball> m_redBalls;
	std::unique_ptr<Ball> m_pWhiteBall;

	std::unique_ptr<Cue> m_pCue;

	bool m_ballsRolling;
};

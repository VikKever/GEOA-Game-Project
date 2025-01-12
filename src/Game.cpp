#include <iostream>
#include <algorithm>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <chrono>
#include "Game.h"
#include "utils.h"
#include "structs.h"
#include "FlyFish.h"

#include "Ball.h"
#include "BoundingBox.h"
#include "GAUtils.h"
#include "Cue.h"
#include "Hole.h"

#include "Texture.h"

int Game::m_points{ 0 };

Game::Game(const Window& window)
	: m_Window{ window }
	, m_Viewport{ 0,0,window.width,window.height }
	, m_pWindow{ nullptr }
	, m_pContext{ nullptr }
	, m_Initialized{ false }
	, m_MaxElapsedSeconds{ 0.1f }
	, m_ballsRolling{ false }
	, m_playArea{ 50.f, 50.f, window.width - 100.f, window.height - 100.f}
	, m_isFirstShot{ true }
	, m_pointsOnText{ 0 }
	, m_hasHitBall{ false }
{
	InitializeGameEngine();

	//m_redBalls.reserve(m_numParticles);

	//for (int idx{}; idx < m_numParticles; ++idx)
	//{
	//	ThreeBlade pos1{ float(std::rand() % int(m_Viewport.width)) , float(std::rand() % int(m_Viewport.height)), 0.f, 1.f};
	//	//TwoBlade moveDirection1{ float(std::rand() % 21 - 10), float(std::rand() % 21 - 10), 0, 0, 0, 0};
	//	//Motor velocity1{ Motor::Translation(rand() % 100 + 100, moveDirection1)};
	//	m_redBalls.emplace_back(Ball{ pos1, Motor{} });
	//}
	SetupRedBalls();
	ResetWhiteBall();
	SetupHoles();

	m_pBoundingBox = std::make_unique<BoundingBox>(m_playArea);

	UpdateScoreText();
}

Game::~Game()
{
	CleanupGameEngine();
}

void Game::AddPoints(int amount)
{
	m_points += amount;
}

void Game::InitializeGameEngine()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling SDL_Init: " << SDL_GetError() << std::endl;
		return;
	}

	// Use OpenGL 2.1
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	// Create window
	m_pWindow = SDL_CreateWindow(
		m_Window.title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		int(m_Window.width),
		int(m_Window.height),
		SDL_WINDOW_OPENGL);
	if (m_pWindow == nullptr)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling SDL_CreateWindow: " << SDL_GetError() << std::endl;
		return;
	}

	// Create OpenGL context 
	m_pContext = SDL_GL_CreateContext(m_pWindow);
	if (m_pContext == nullptr)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
		return;
	}

	// Set the swap interval for the current OpenGL context,
	// synchronize it with the vertical retrace
	if (m_Window.isVSyncOn)
	{
		if (SDL_GL_SetSwapInterval(1) < 0)
		{
			std::cerr << "BaseGame::Initialize( ), error when calling SDL_GL_SetSwapInterval: " << SDL_GetError() << std::endl;
			return;
		}
	}
	else
	{
		SDL_GL_SetSwapInterval(0);
	}

	// Set the Projection matrix to the identity matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set up a two-dimensional orthographic viewing region.
	glOrtho(0, m_Window.width, 0, m_Window.height, -1, 1); // y from bottom to top

	// Set the viewport to the client window area
	// The viewport is the rectangular region of the window where the image is drawn.
	glViewport(0, 0, int(m_Window.width), int(m_Window.height));

	// Set the Modelview matrix to the identity matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Enable color blending and use alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling TTF_Init: " << TTF_GetError() << std::endl;
		return;
	}

	m_Initialized = true;
}

void Game::Run()
{
	if (!m_Initialized)
	{
		std::cerr << "BaseGame::Run( ), BaseGame not correctly initialized, unable to run the BaseGame\n";
		std::cin.get();
		return;
	}

	// Main loop flag
	bool quit{ false };

	// Set start time
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

	//The event loop
	SDL_Event e{};
	while (!quit)
	{
		// Poll next event from queue
		while (SDL_PollEvent(&e) != 0)
		{
			// Handle the polled event
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				this->ProcessKeyDownEvent(e.key);
				break;
			case SDL_KEYUP:
				this->ProcessKeyUpEvent(e.key);
				break;
			case SDL_MOUSEMOTION:
				e.motion.y = int(m_Window.height) - e.motion.y;
				this->ProcessMouseMotionEvent(e.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
				e.button.y = int(m_Window.height) - e.button.y;
				this->ProcessMouseDownEvent(e.button);
				break;
			case SDL_MOUSEBUTTONUP:
				e.button.y = int(m_Window.height) - e.button.y;
				this->ProcessMouseUpEvent(e.button);
				break;
			}
		}

		if (!quit)
		{
			// Get current time
			std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

			// Calculate elapsed time
			float elapsedSeconds = std::chrono::duration<float>(t2 - t1).count();

			// Update current time
			t1 = t2;

			// Prevent jumps in time caused by break points
			elapsedSeconds = std::min(elapsedSeconds, m_MaxElapsedSeconds);

			// Call the BaseGame object 's Update function, using time in seconds (!)
			this->Update(elapsedSeconds);

			// Draw in the back buffer
			this->Draw();

			// Update screen: swap back and front buffer
			SDL_GL_SwapWindow(m_pWindow);
		}
	}
}

void Game::CleanupGameEngine()
{
	SDL_GL_DeleteContext(m_pContext);

	SDL_DestroyWindow(m_pWindow);
	m_pWindow = nullptr;

	//Quit SDL subsystems
	TTF_Quit();
	SDL_Quit();

}

void Game::SetupRedBalls()
{
	const Point2f startPos{ m_Viewport.width / 3, m_Viewport.height / 2 };
	const int numColumns{ 5 };

	// Create red balls
	// =========================
	const float horizontalDst{ Ball::SIZE * std::cosf(M_PI / 6) + 0.1f};
	const float verticalDst{ Ball::SIZE + 0.1f };

	// pascals formula for the total amount of balls (= 1 + 2 + 3 + ... + numColumns)
	m_redBalls.reserve((numColumns * (numColumns + 1)) / 2);

	for (int column{}; column < numColumns; ++column)
	{
		for (int row{}; row < column + 1; ++row)
		{
			ThreeBlade pos{
				startPos.x - (column * horizontalDst),
				startPos.y + (row * verticalDst) - (column * Ball::SIZE / 2),
				0.f, 1.f
			};
			m_redBalls.push_back(Ball{ pos, Motor{1, 0, 0, 0, 0, 0, 0, 0} });
		}
	}
}

void Game::ResetWhiteBall()
{
	m_pWhiteBall = std::make_unique<Ball>(ThreeBlade{ 2 * m_Viewport.width / 3, m_Viewport.height / 2, 0.f, 1.f }, Motor{ 1, 0, 0, 0, 0, 0, 0, 0 }/*Motor::Translation(500.f, TwoBlade{ -1, 0, 0, 0, 0, 0 })*/, true);
	
	// make sure the cue still points to the ball
	m_pCue = std::make_unique<Cue>(m_pWhiteBall.get());
}

void Game::SetupHoles()
{
	m_holes.reserve(6);

	// left holes
	m_holes.push_back(Hole{ ThreeBlade{ m_playArea.left + 12.f, m_playArea.bottom + 12.f, 0, 1 } });
	m_holes.push_back(Hole{ ThreeBlade{ m_playArea.left + 12.f, m_playArea.bottom + m_playArea.height - 12.f, 0, 1 } });

	// middle holes
	m_holes.push_back(Hole{ ThreeBlade{ m_playArea.left + m_playArea.width / 2, m_playArea.bottom + 10.f, 0, 1 } });
	m_holes.push_back(Hole{ ThreeBlade{ m_playArea.left + m_playArea.width / 2, m_playArea.bottom + m_playArea.height - 10.f, 0, 1 } });

	// right holes
	m_holes.push_back(Hole{ ThreeBlade{ m_playArea.left + m_playArea.width - 12.f, m_playArea.bottom + 12.f, 0, 1 } });
	m_holes.push_back(Hole{ ThreeBlade{ m_playArea.left + m_playArea.width - 12.f, m_playArea.bottom + m_playArea.height - 12.f, 0, 1 } });
}

void Game::CheckBallsRolling()
{
	m_ballsRolling = false;
	for (const Ball& ball : m_redBalls)
	{
		m_ballsRolling |= ball.IsMoving();
	}
	m_ballsRolling |= m_pWhiteBall->IsMoving();

	if (!m_ballsRolling)
	{
		// executed when balls stop rolling
		m_isFirstShot = false;

		// if no red balls have been hit, lose 20 points
		if (!m_hasHitBall)
		{
			AddPoints(-10);
		}
	}
}

bool Game::FallsInHole(const Ball& ball)
{
	for (const Hole& hole : m_holes)
	{
		if (hole.FallsIn(ball)) return true;
	}
	return false;
}

void Game::UpdateScoreText()
{
	m_pScoreText = std::make_unique<Texture>(std::to_string(m_points), "THEBOLDFONT_FREEVERSION.ttf", 20, Color4f{ 1, 1, 1, 1 });
	if (!m_pScoreText->IsCreationOk())
	{
		std::cout << "ERROR loading score text\n";
	}
	m_pointsOnText = m_points;
}

void Game::Update(float elapsedSec)
{
	if (m_pointsOnText != m_points)
	{
		UpdateScoreText();
	}

	// update white ball
	m_pWhiteBall->Update(elapsedSec, m_pBoundingBox.get(), m_isFirstShot);

	if (m_redBalls.size() <= 0) return;

	// update red balls
	for (Ball& particle : m_redBalls)
	{
		particle.Update(elapsedSec, m_pBoundingBox.get(), m_isFirstShot);
	}

	// handle collisions between red balls
	// only loop over every particle interaction once
	for (int idx1{}; idx1 < m_redBalls.size() - 1; ++idx1)
	{
		for (int idx2{ idx1 + 1 }; idx2 < m_redBalls.size(); ++idx2)
		{
			m_redBalls[idx1].CheckParticleCollision(m_redBalls[idx2], m_isFirstShot);
		}
	}

	// handle collisions between the white ball and red balls
	for (int idx{}; idx < m_redBalls.size(); ++idx)
	{
		if (m_pWhiteBall->CheckParticleCollision(m_redBalls[idx], m_isFirstShot))
		{
			// if there was a collision between the white ball and a red ball, the player doesn't lose points for this
			m_hasHitBall = true;
		}
	}

	// move all red balls that fell into a hole to the back of the list
	auto removeIt{ std::remove_if(m_redBalls.begin(), m_redBalls.end(),
		[&](const Ball& ball) { return FallsInHole(ball); }) };
	// count the points of all removed balls
	for (auto it{ removeIt }; it != m_redBalls.end(); ++it)
	{
		AddPoints(it->GetPoints());
	}
	// remove the balls that fell into a hole
	if (removeIt != m_redBalls.end())
	{
		m_redBalls.erase(removeIt);
	}

	// if the white ball fals into a hole, it gets reset back to its starting position
	if (FallsInHole(*m_pWhiteBall.get()))
	{
		ResetWhiteBall();
		AddPoints(-5);
	}

	if (m_ballsRolling)
	{
		// check if there are no longer balls rolling and the cue can appear again
		CheckBallsRolling();
	}
	else
	{
		// update cue
		int x, y;
		Uint32 mouseState = SDL_GetMouseState(&x, &y);
		bool isShooting{ (mouseState & SDL_BUTTON(1)) != 0 };
		m_pCue->Update(Point2f{ float(x), float(m_Viewport.height - y) }, isShooting);
		if (isShooting)
		{
			if (m_pCue->CheckHitBall())
			{
				// executed on hitting ball
				m_ballsRolling = true;
				m_hasHitBall = false;
			}
		}
	}
}

void Game::Draw() const
{
	glClearColor(0.15f, 0.3f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// draw game area
	utils::SetColor(Color4f{ 0.05f, 0.2f, 0.05f, 1.f });
	utils::FillRect(m_playArea);

	// draw holes
	for (const Hole& hole : m_holes)
	{
		hole.Draw();
	}

	// draw balls
	for (const Ball& particle : m_redBalls)
	{
		particle.Draw();
	}
	m_pWhiteBall->Draw();

	// draw cue
	if (!m_ballsRolling) m_pCue->Draw();

	// draw score
	if (m_pScoreText->IsCreationOk())
	{
		m_pScoreText->Draw(Point2f{ 10.f, m_Viewport.height - 10.f - m_pScoreText->GetHeight() });
	}
}

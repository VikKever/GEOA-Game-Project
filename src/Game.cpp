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
#include "Player.h"

#include "Texture.h"

int Game::m_points{ 0 };

Game::Game(const Window& window)
	: m_Window{ window }
	, m_Viewport{ 0,0,window.width,window.height }
	, m_pWindow{ nullptr }
	, m_pContext{ nullptr }
	, m_Initialized{ false }
	, m_MaxElapsedSeconds{ 0.1f }
	, m_pointsOnText{ 0 }
{
	InitializeGameEngine();

	m_pBalls.reserve(10);

	for (int idx{}; idx < 10; ++idx)
	{
		ThreeBlade pos1{ float(std::rand() % int(m_Viewport.width)) , float(std::rand() % int(m_Viewport.height)), 1.f, 1.f};
		TwoBlade moveDirection1{ float(std::rand() % 21 - 10), float(std::rand() % 21 - 10), 0, 0, 0, 0};
		Motor velocity1{ Motor::Translation(float(rand() % 100), moveDirection1)};
		m_pBalls.emplace_back(std::make_unique<Ball>(pos1, velocity1, false));
	}

	m_pBoundingBox = std::make_unique<BoundingBox>(m_Viewport);

	m_pPlayer = std::make_unique<Player>(
		ThreeBlade{ m_Viewport.width / 3, m_Viewport.height / 2, 1.f, 1 },
		Motor{ 1, 0, 0, 0, 0, 0, 0, 0 },
		ThreeBlade{ m_Viewport.width / 2, m_Viewport.height / 2, 0, 1 }
	);

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

	if (m_pBalls.size() <= 0) return;

	// update balls
	for (const std::unique_ptr<Ball>& particle : m_pBalls)
	{
		particle->Update(elapsedSec, m_pBoundingBox.get());
	}

	// update the player
	m_pPlayer->Update(elapsedSec, m_pBoundingBox.get());

	// handle collisions between balls
	// only loop over every particle interaction once
	for (int idx1{}; idx1 < m_pBalls.size() - 1; ++idx1)
	{
		for (int idx2{ idx1 + 1 }; idx2 < m_pBalls.size(); ++idx2)
		{
			m_pBalls[idx1]->CheckCollision(*m_pBalls[idx2]);
		}
	}

	//// handle collisions between the player and the balls
	//for (Ball& ball : m_pBalls)
	//{
	//	//m_pPlayer->CheckCollisionWithBall(ball);
	//}

	// move all dead balls to the back of the list
	auto removeIt{ std::remove_if(m_pBalls.begin(), m_pBalls.end(),
		[&](const std::unique_ptr<Ball>& pBall) { return pBall->IsDead(); }) };

	// count the points of all (non-projectile) removed balls
	for (auto it{ removeIt }; it != m_pBalls.end(); ++it)
	{
		if (!(*it)->IsProjectile())
		{
			AddPoints(10);
		}
	}
	// actually remove the balls
	if (removeIt != m_pBalls.end())
	{
		m_pBalls.erase(removeIt);
	}
}

void Game::Draw() const
{
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// draw red balls
	for (const std::unique_ptr<Ball>& pBall : m_pBalls)
	{
		pBall->Draw();
	}

	// draw player
	m_pPlayer->Draw();

	// draw score
	if (m_pScoreText->IsCreationOk())
	{
		m_pScoreText->Draw(Point2f{ 10.f, m_Viewport.height - 10.f - m_pScoreText->GetHeight() });
	}
}

void Game::ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
{
}

void Game::ProcessKeyUpEvent(const SDL_KeyboardEvent& e)
{
	if (e.keysym.sym == SDLK_w)
	{
		m_pPlayer->ReflectAroundPillar();
	}
	else if (e.keysym.sym == SDLK_SPACE)
	{
		m_pBalls.push_back(m_pPlayer->ShootBall());
	}
}

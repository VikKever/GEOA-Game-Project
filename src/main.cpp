#include "SDL.h"

#include <ctime>
#include "Game.h"

int main(int argv, char** args)
{
	srand(static_cast<unsigned int>(time(nullptr)));

	Window window{ "GEOA Project Demo", 1280.f , 720.f };
	Game pGame{ window };
	pGame.Run();

	return 0;
}


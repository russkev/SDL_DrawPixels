#include <iostream>
#include <tuple>
#include <algorithm>
#include <SDL.h>
#include <list>

int main(int, char**) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = NULL;  //Good practice to always initialize a pointer variable

	window = SDL_CreateWindow("Title goes here",  //Title of window
		SDL_WINDOWPOS_UNDEFINED, //Lets window manager to choose where the window will go.
		SDL_WINDOWPOS_UNDEFINED,
		400,                     //Width
		200,                     //Height
		SDL_WINDOW_RESIZABLE     //Flag to make the window resizeable
		);

	if (window == NULL) {
		std::cout << "There was an error initializing the window" << std::endl
		<< SDL_GetError << std::endl;
	}

	SDL_Surface *screen = SDL_GetWindowSurface(window);

	Uint32 red = SDL_MapRGB(screen->format, 255, 0, 0);
	SDL_FillRect(screen, NULL, red);
	SDL_UpdateWindowSurface(window);

	int x, y, w, h;

	

	SDL_Event event;
	bool running = true;
	
	// // While program is running, poll event
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
				break;
			}
		}
		SDL_GetWindowPosition(window, &x, &y);
		std::cout << x << ", " << y << std::endl;
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
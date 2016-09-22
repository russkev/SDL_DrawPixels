#include <iostream>
#include <tuple>
#include <algorithm>
#include <SDL.h>
#include <list>

int main(int, char**) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window;

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

	SDL_Delay(6000);

	SDL_DestroyWindow(window);
	SDL_Quit();

	for (int i = 0; i < 100; i++) {
		std::cout << "do something";
	}


	return 0;
}
#include <iostream>
#include <tuple>
#include <algorithm>
#include <SDL.h>
#include <list>


struct coordinate {
	int x, y;
};

struct line {
	coordinate start, end;
};

bool checkInBounds(const coordinate &a, SDL_Surface* surface) {
	if (a.x > 0 && a.y > 0 && a.x <= surface->w && a.y <= surface->h) {
		return true;
	}
	else return false;
}


void drawPixel(const coordinate &coordA, std::uint32_t color, SDL_Surface* surface) {
	if (!checkInBounds(coordA, surface)) {
		std::cout << "Pixel not in bounds" << std::endl;
		return;
	}
	reinterpret_cast<std::uint32_t*> (surface->pixels)[coordA.x + coordA.y * surface->w] = color;
}

void drawLine(const line &lineA, std::uint32_t color, SDL_Surface* surface) {
	if (!checkInBounds(lineA.start, surface) || !checkInBounds(lineA.end, surface)) {
		std::cout << "Line not in bounds" << std::endl;
		return;
	}

	// // INITIALIZE VARIABLES
	coordinate drawCoord;
	drawCoord.x = lineA.start.x;
	drawCoord.y = lineA.start.y;
	int driving, dEnd, passive, pEnd, i, dInc = -1, pInc = -1; // D for Driving, P for Passive
	double m = 0, e, deltaX, deltaY;
	bool flipped = false;

	deltaX = lineA.end.x - lineA.start.x;
	deltaY = lineA.end.y - lineA.start.y;

	
	// // CASE WHERE LINE IS POINT
	if (deltaX == 0 && deltaY == 0) { 
		drawPixel(drawCoord, color, surface);
		return;
	}

	// // CASE WHERE LINE IS STRAIGHT UP DOWN
	if (deltaX == 0) { 
		if (deltaY < 0) {
			drawCoord.y = lineA.end.y;
		}
		for (i = 0; i < abs(deltaY); ++i) {
			drawPixel(drawCoord, color, surface);
			drawCoord.y = drawCoord.y + 1;
		}
		return;
	}
	
	m = deltaY / deltaX; // m is slope of line.

	// // SET DRIVING AND PASSIVE AXIS
	if (abs(deltaX) >= abs(deltaY)) {
		driving = lineA.start.x;
		dEnd = lineA.end.x;
		passive = lineA.start.y;
		pEnd = lineA.end.y;
		if (deltaX >= 0) dInc = 1;
		if (deltaY >= 0) pInc = 1;
	}
	else {
		driving = lineA.start.y;
		dEnd = lineA.end.y;
		passive = lineA.start.x;
		pEnd = lineA.end.x;
		m = 1 / m;
		flipped = true;
		if (deltaY >= 0) dInc = 1;
		if (deltaX >= 0) pInc = 1;
	}
	
	
	e = abs(m) - 1; //e is error margin, when > 0, passive gets incremented.

	// // ITERATE THROUGH EACH DRIVING COORDINATE AND DRAW PIXEL
	while (driving != dEnd) {

		drawCoord.x = passive;
		drawCoord.y = driving;
		if (!flipped) std::swap(drawCoord.x, drawCoord.y);

		drawPixel(drawCoord, color, surface);

		if (e >= 0) {
			passive += pInc;
			--e;
		}
		driving += dInc;
		e += abs(m);
	}
}

int main(int, char**) {
	SDL_Init(SDL_INIT_EVERYTHING);
	std::atexit(&SDL_Quit);




	auto s_window = SDL_CreateWindow("Fuck me", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280u, 720u, 0u);
	auto s_surface = SDL_GetWindowSurface(s_window);
	SDL_FillRect(s_surface, nullptr, 0xFFFFFFFF);

	SDL_Event s_event;
	auto s_last_x = 0;
	auto s_last_y = 0;
	auto s_size = 0;


	// // DEFINE LINE PROPERTIES // //
	int red = 0xFFFF0000;

	coordinate pixelCoord;
	pixelCoord.x = 10; pixelCoord.y = 10;
	drawPixel(pixelCoord, red, s_surface);

	int startX = 300;
	int startY = 300;
	int endX[] = { 300, 300, 380 ,450, 480, 500, 480, 450, 380, 300, 220, 150, 120, 100, 120, 150, 200 };
	int endY[] = { 300, 100, 130 ,150, 220, 300, 380, 450, 480, 500, 480, 450, 390, 300, 220, 150, 130 };
	int numLines = sizeof(endX) / sizeof(endX[0]);


	// // CREATE LINE DATA STRUCTURES // //
	std::list<line> lines;
	for (int i = 0; i < numLines; ++i) {
		coordinate tempStart;
		coordinate tempEnd;
		line tempLine;
		tempStart.x = startX;
		tempStart.y = startY;
		tempEnd.x = endX[i];
		tempEnd.y = endY[i];
		tempLine.start = tempStart;
		tempLine.end = tempEnd;
		lines.push_back(tempLine);
	}

	// // DRAW LINES // //
	std::list<line>::iterator it = lines.begin();
	while (it != lines.end()) {
		drawLine(*it, red, s_surface);
		it++;
	}





	while (!SDL_QuitRequested()) {
		SDL_UpdateWindowSurface(s_window);
	}

	SDL_DestroyWindow(s_window);
	return 0;
}
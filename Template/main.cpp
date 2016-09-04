#include <iostream>
#include <tuple>
#include <algorithm>
#include <list>
#include <cassert>
#include <cmath>
#include <ctime>

#include <SDL.h>

#pragma pack(push, 1)
struct color_type {
	color_type(
		std::uint8_t red = 0u,
		std::uint8_t green = 0u,
		std::uint8_t blue = 0u,
		std::uint8_t alpha = 0u)
		: b(blue),
		g(green),
		r(red),
		a(alpha)
	{}

	std::uint8_t b, g, r, a;

	static const color_type black;
	static const color_type red;
	static const color_type green;
	static const color_type blue;
	static const color_type yellow;
	static const color_type magenta;
	static const color_type cyan;
	static const color_type white;
};
#pragma pack(pop)

const color_type color_type::black = color_type(0, 0, 0);
const color_type color_type::red = color_type(255, 0, 0);
const color_type color_type::green = color_type(0, 255, 0);
const color_type color_type::blue = color_type(0, 0, 255);
const color_type color_type::yellow = color_type(255, 255, 0);
const color_type color_type::magenta = color_type(0, 255, 255);
const color_type color_type::cyan = color_type(255, 0, 255);
const color_type color_type::white = color_type(255, 255, 255);

struct coordinate_type {
	// Note: Added constructor
	coordinate_type(int xx = 0, int yy = 0) : x(xx), y(yy) {}

	int x, y;
};

struct line_type {
	// Note: Added constructor
	line_type(
		const coordinate_type& a = coordinate_type(),
		const coordinate_type& b = coordinate_type(),
		const color_type& c = color_type())
		: start(a),
		end(b),
		color(c)
	{}

	coordinate_type start, end;
	color_type color;
};

struct circle_type {
	circle_type(
		const coordinate_type& a = coordinate_type(),
		int b = 0,
		const color_type& c = color_type())
		: centre(a),
		radius(b),
		color(c)
	{}

	coordinate_type centre;
	int radius;
	color_type color;
};

bool checkInBounds(const coordinate_type& a, const SDL_Surface* surface) {
	if (a.x >= 0 && a.y >= 0 && a.x < surface->w && a.y < surface->h) {
		return true;
	}
	// Note: Doesn't really need an else here
	return false;
}

// Note: clamps x to range [a; b], useful function to have
template <typename T>
inline T clamp(T x, T a, T b) {
	return std::max(std::min(x, b), a);
}

void drawPixel(const coordinate_type& coord, const color_type& color, SDL_Surface* surface) {
	if (!checkInBounds(coord, surface)) {
		return;
	}
	reinterpret_cast<color_type*> (surface->pixels)[coord.x + coord.y * surface->w] = color;
}

void drawHLine(const line_type& lineA, const color_type& color, SDL_Surface* surface) {
	assert(lineA.start.y == lineA.end.y);
	const auto dx = clamp(lineA.end.x - lineA.start.x, -1, 1);
	for (auto x = lineA.start.x; x != lineA.end.x; x += dx) {
		drawPixel(coordinate_type(x, lineA.start.y), color, surface);
	}
}

void drawVLine(const line_type& lineA, const color_type& color, SDL_Surface* surface) {
	assert(lineA.start.x == lineA.end.x);
	const auto dy = clamp(lineA.end.y - lineA.start.y, -1, 1);
	for (auto y = lineA.start.y; y != lineA.end.y; y += dy) {
		drawPixel(coordinate_type(lineA.start.x, y), color, surface);
	}
}

void drawLine(const line_type& lineA, SDL_Surface* surface) {
	if (!checkInBounds(lineA.start, surface) && !checkInBounds(lineA.end, surface)) {
		return;
	}

	auto deltaX = lineA.end.x - lineA.start.x;
	auto deltaY = lineA.end.y - lineA.start.y;

	if (deltaX == 0 && deltaY == 0) {
		return;
	}

	if (deltaX == 0) return drawVLine(lineA, lineA.color, surface);
	if (deltaY == 0) return drawHLine(lineA, lineA.color, surface);


	auto driving = lineA.start.x;
	auto passive = lineA.start.y;
	auto dEnd = lineA.end.x;
	auto pEnd = lineA.end.y;
	auto dInc = clamp(deltaX, -1, 1);
	auto pInc = clamp(deltaY, -1, 1);

	const auto flipped = abs(deltaX) < abs(deltaY);
	if (flipped) {
		std::swap(driving, passive);
		std::swap(dEnd, pEnd);
		std::swap(dInc, pInc);
	}

	auto slope = flipped ? deltaX / (1.0*deltaY)
		: deltaY / (1.0*deltaX);
	auto error = abs(slope) - 1.0;

	while (driving != dEnd) {
		drawPixel(flipped ? coordinate_type(passive, driving)
			: coordinate_type(driving, passive),
			lineA.color, surface);

		if (error >= 0.0) {
			passive += pInc;
			--error;
		}
		driving += dInc;
		error += abs(slope);
	}
}

void drawCircle(const circle_type& circle, SDL_Surface* s_surface) {
	int x_n = circle.radius;
	int y_n = 0;
	int err = 0;

	while (x_n >= y_n) {
		//coordinate_type drawCoord = { circle.centre.x + x_n, circle.centre.y + y_n };
		drawPixel({ circle.centre.x + x_n, circle.centre.y + y_n }, circle.color, s_surface);
		drawPixel({ circle.centre.x + x_n, circle.centre.y - y_n }, circle.color, s_surface);
		drawPixel({ circle.centre.x - x_n, circle.centre.y + y_n }, circle.color, s_surface);
		drawPixel({ circle.centre.x - x_n, circle.centre.y - y_n }, circle.color, s_surface);
		drawPixel({ circle.centre.x + y_n, circle.centre.y + x_n }, circle.color, s_surface);
		drawPixel({ circle.centre.x + y_n, circle.centre.y - x_n }, circle.color, s_surface);
		drawPixel({ circle.centre.x - y_n, circle.centre.y + x_n }, circle.color, s_surface);
		drawPixel({ circle.centre.x - y_n, circle.centre.y - x_n }, circle.color, s_surface);


		++y_n;
		err += (1 + 2 * y_n);
		if (2 * (err - x_n) + 1 > 0)
		{
			--x_n;
			err += 1 - 2 * x_n;
		}
	}
}

void drawClock(SDL_Surface* s_surface) {
	// TODO : Write code to draw a clock here

	// ??? Why static here?
	//static const auto start = coordinate_type(300, 300);
	//static const int rad = 150;
	auto circleA = circle_type({ 640, 360 }, 300, color_type::blue);
	
	drawCircle(circleA, s_surface);




}

int main(int, char**) {
	SDL_Init(SDL_INIT_EVERYTHING);
	std::atexit(&SDL_Quit);

	auto s_window = SDL_CreateWindow("Pretty little lines", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280u, 720u, 0u); // Better than the prev window name I guess
	auto s_surface = SDL_GetWindowSurface(s_window);




	SDL_Event s_event;
	while (!SDL_QuitRequested()) {
		if (SDL_PollEvent(&s_event)) {
			//Note: Need to pull all events before doing anything else
			continue;
		}
		SDL_FillRect(s_surface, nullptr, 0xFFFFFF);
		SDL_LockSurface(s_surface);
		drawClock(s_surface);
		SDL_UnlockSurface(s_surface);
		SDL_UpdateWindowSurface(s_window);
	}

	SDL_DestroyWindow(s_window);
	return 0;
}
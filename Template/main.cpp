#include <iostream>
#include <tuple>
#include <algorithm>
#include <list>
#include <cassert>
#include <cmath>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

const double pi = 3.14159265;
//CPP Reference had '#define PI 3.14159265' but I couldn't get that to work

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

double degToRad(double theta) {
	double result = theta*pi/180;
	return result;
}

void drawRadialLine(const coordinate_type centre, double theta, int radiusStart, int radiusEnd, color_type color, SDL_Surface* surface) {
	theta = degToRad(theta);
	coordinate_type startCoord, endCoord;
	startCoord = {
		(int)round(radiusStart * sin(theta) + centre.x),
		(int)round(radiusStart * -cos(theta) + centre.y)
	};
	endCoord = {
		(int)round(radiusEnd * sin(theta) + centre.x),
		(int)round(radiusEnd * -cos(theta) + centre.y)
	};

	drawLine({ startCoord, endCoord, color }, surface);


	return;
}

void drawCircle(const circle_type& circle, SDL_Surface* s_surface) {
	int x_n = circle.radius;
	int y_n = 0;
	int err = 0;

	while (x_n >= y_n) {
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

void drawMinuteLine(const coordinate_type &centre, int radius, SDL_Surface* s_surface) {
	for (int i = 0; i < 360; i += 6) {
		if (i % 30 != 0) {
			drawRadialLine(centre, i, radius - 20, radius, color_type::green, s_surface);
		}

	}

}

void drawHourLine(const coordinate_type &centre, int radius, SDL_Surface* s_surface) {
	for (int i = 0; i < 360; i += 30) {
			drawRadialLine(centre, i, radius - 40, radius, color_type::red, s_surface);
	}

}

void drawHand(const coordinate_type &centre, double angle, int radius, const color_type &color, SDL_Surface* s_surface) {
	drawRadialLine(centre, angle, 0, radius, color, s_surface);
}

void drawClock(SDL_Surface* s_surface) {
	// TODO : Write code to draw a clock here

	// ??? Why static here?
	static const coordinate_type centreSurface = { 640, 360 };
	static const int clockRadius = 300;
	static const auto circleA = circle_type(centreSurface, clockRadius, color_type::blue);
	
	drawCircle(circleA, s_surface);
	drawMinuteLine(centreSurface, clockRadius, s_surface);
	drawHourLine(centreSurface, clockRadius, s_surface);

	std::time_t clockTime = std::time(NULL);
	tm * localTime = localtime(&clockTime);
	double second = localTime->tm_sec;
	double minute = localTime->tm_min;
	double hour   = localTime->tm_hour;

	second = (second * 6);// -90;
	minute = (minute * 6 + second / 60);// -90;
	hour = (hour * 30 + minute / 12);// -90;


	drawRadialLine(centreSurface, second, 0, clockRadius -  40, color_type::red,     s_surface);
	drawRadialLine(centreSurface, minute, 0, clockRadius -  80, color_type::magenta, s_surface);
	drawRadialLine(centreSurface, hour, 0, clockRadius - 120, color_type::cyan,    s_surface);
	return;

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
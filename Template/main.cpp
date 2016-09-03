#include <iostream>
#include <tuple>
#include <algorithm>
#include <list>
#include <cassert>

#include <SDL.h>

#pragma pack(push, 1)
struct color_type {
    color_type (
        std::uint8_t red    = 0u, 
        std::uint8_t green  = 0u, 
        std::uint8_t blue   = 0u, 
        std::uint8_t alpha  = 0u)
    :   b (blue), 
        g (green), 
        r (red), 
        a (alpha)
    {}

    std::uint8_t b, g, r, a;
};
#pragma pack(pop)

struct coordinate_type {
    // Note: Added constructor
    coordinate_type (int xx = 0, int yy = 0): x (xx), y (yy) {}

    int x, y;
};

struct line_type {
    // Note: Added constructor
    line_type (
        const coordinate_type& a = coordinate_type (), 
        const coordinate_type& b = coordinate_type (),
        const color_type& c = color_type ())
    :   start (a), 
        end (b),
        color (c)
    {}

    coordinate_type start, end;
    color_type color;
};

bool checkInBounds (const coordinate_type& a, const SDL_Surface* surface) {
    if (a.x > 0 && a.y > 0 && a.x <= surface->w && a.y <= surface->h) {
        return true;
    } 
    // Note: Doesn't really need an else here
    return false;
}

// Note: clamps x to range [a; b], useful function to have
template <typename T> 
inline T clamp (T x, T a, T b) { 
    return std::max (std::min (x, b), a); 
}

void drawPixel (const coordinate_type& coord, const color_type& color, SDL_Surface* surface) {
    if (!checkInBounds (coord, surface)) {        
        return;
    }
    reinterpret_cast<color_type*> (surface->pixels) [coord.x + coord.y * surface->w] = color;
}

void drawHLine (const line_type& lineA, const color_type& color, SDL_Surface* surface) {
    assert (lineA.start.y  == lineA.end.y);    
    const auto dx = clamp (lineA.end.x - lineA.start.x, -1, 1);
    for (auto x = lineA.start.x; x != lineA.end.x; x += dx) {
        drawPixel (coordinate_type (x, lineA.start.y), color, surface);
    }
}

void drawVLine (const line_type& lineA, const color_type& color, SDL_Surface* surface) {
    assert (lineA.start.x  == lineA.end.x);
    const auto dy = clamp (lineA.end.y - lineA.start.y, -1, 1);
    for (auto y = lineA.start.y; y != lineA.end.y; y += dy) {
        drawPixel (coordinate_type (lineA.start.x, y), color, surface);
    }
}

void drawLine (const line_type& lineA, SDL_Surface* surface) {
    if (!checkInBounds (lineA.start, surface) && !checkInBounds (lineA.end, surface)) {
        //Todo: Actually you should clip the line here, 
        //      and only ignore the line completely
        //      if both points are out of bounds.
        return;
    }
    
    // Note : should be int, comparing floats and doubles is not a good idea
    auto deltaX = lineA.end.x - lineA.start.x;
    auto deltaY = lineA.end.y - lineA.start.y;

    // // CASE WHERE LINE IS POINT
    if (deltaX == 0 && deltaY == 0) {
        // Todo : actually if both are 0 , the line is of 0 length, means not even one point
        return;
    }

    // Note: can handle horizontal and vertical lines as separate case
    if (deltaX == 0) return drawVLine (lineA, lineA.color, surface);
    if (deltaY == 0) return drawHLine (lineA, lineA.color, surface);
                

    auto driving = lineA.start.x;
    auto passive = lineA.start.y;
    auto dEnd = lineA.end.x;
    auto pEnd = lineA.end.y;
    auto dInc = clamp (deltaX, -1, 1);
    auto pInc = clamp (deltaY, -1, 1);

    const auto flipped = abs (deltaX) < abs (deltaY);
    if (flipped) {
        std::swap (driving, passive);
        std::swap (dEnd, pEnd);        
        std::swap (dInc, pInc);
    }

    // Note : 1.0* , to force one of the variables into a double, and then the whole expression into double
    //        also renamed to a more sensible name
    auto slope = flipped ? deltaX / (1.0*deltaY)  
                         : deltaY / (1.0*deltaX); 
        // when > 0, passive gets incremented.
    auto error = abs (slope) - 1.0; 
    
    while (driving != dEnd) { 
        drawPixel (flipped ? coordinate_type (passive, driving) 
                           : coordinate_type (driving, passive), 
                   lineA.color, surface);

        if (error >= 0.0) {
            passive += pInc;
            --error;
        }
        driving += dInc;
        error += abs (slope);
    }
}

int main (int, char**) {
    SDL_Init (SDL_INIT_EVERYTHING);
    std::atexit (&SDL_Quit);

    auto s_window = SDL_CreateWindow ("Pretty little lines", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280u, 720u, 0u);
    auto s_surface = SDL_GetWindowSurface (s_window);

    SDL_FillRect (s_surface, nullptr, 0xFFFFFFFF);

    static const auto start = coordinate_type (300, 300);
    static const auto lines = std::list<line_type>{
        {start, {300, 100}, {255,   0,   0}},
        {start, {380, 130}, {  0, 255,   0}},
        {start, {450, 150}, {  0,   0, 255}},
        {start, {480, 220}, {255, 255,   0}},
        {start, {500, 300}, {255,   0, 255}},
        {start, {480, 380}, {  0, 255, 255}},
        {start, {450, 450}, {127, 127,   0}},
        {start, {380, 480}, {127,   0, 127}},
        {start, {300, 500}, {  0, 127, 127}},
        {start, {220, 480}, {255, 255, 127}},
        {start, {150, 450}, {255, 127, 255}},
        {start, {120, 390}, {127, 255, 255}},
        {start, {100, 300}, {127,   0,   0}},
        {start, {120, 220}, {  0, 127,   0}},
        {start, {150, 150}, {  0,   0, 127}},
        {start, {200, 130}, {  0,   0,   0}}
        
    };

    for (const auto& l : lines) {       
        drawLine (l, s_surface);
    }

    SDL_Event s_event;
    while (!SDL_QuitRequested ()) {
        if (SDL_PollEvent (&s_event)) {
            //Note: Need to pull all events before doing anything else
            continue;
        }
        SDL_UpdateWindowSurface (s_window);
    }

    SDL_DestroyWindow (s_window);
    return 0;
}

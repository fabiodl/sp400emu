#ifndef _SP400_PLOTTER_H
#define _SP400_PLOTTER_H
#include "board.h"
#include <SDL2/SDL.h>

struct Point {
  int32_t x, y;
  Point(int32_t _x, int32_t _y) : x(_x), y(_y) {}
};

struct Color {
  uint8_t r, g, b;
};

class Window {
public:
  Window(int32_t w, int32_t h);
  ~Window();
  SDL_Window *window;
  SDL_Renderer *renderer;
  void clear();
};

class Texture {
public:
  Texture(SDL_Renderer *renderer, int32_t w, int32_t h);
  ~Texture();
  void draw(const Point &p, const Color &c);
  SDL_Texture *get() { return pointsTexture; }
  void resizeHeight(int32_t h);
  int32_t getHeight() { return height; }

private:
  SDL_Texture *pointsTexture;
  SDL_Renderer *renderer;
  int32_t width, height;
};

class Surface {
public:
  Surface(int32_t w, int32_t h);
  //~Surface();

  void draw(const Point &p, const Color &c);
  void resizeHeight(int32_t h);
  int32_t getHeight() { return height; }
  void drawTo(SDL_Renderer *renderer, const SDL_Rect &src, const SDL_Rect &dst);

private:
  // SDL_Surface* surface;
  std::vector<uint32_t> buffer;
  int32_t width, height;
};

class Plotter {
public:
  Plotter(Board &, int32_t w, int32_t h);
  void run();

private:
  int32_t canvas_width, canvas_height;
  int32_t page_width, page_height;
  Point motOff;
  Point pageOff;
  Point head;
  Window win;
  Surface paper;
  void makePage();
  Board &board;
};

#endif

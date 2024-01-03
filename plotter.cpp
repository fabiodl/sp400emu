
#include "plotter.h"
#include <iostream>
#include <sstream>
#include <stdbool.h>
#include <stdexcept>
#include <stdlib.h>

std::ostream &operator<<(std::ostream &o, const SDL_Rect &r) {
  return o << r.x << " " << r.y << " " << r.w << " " << r.h;
}

Surface::Surface(int32_t w, int32_t h)
    : buffer(w * h, 0xFFFFFF), width(w), height(h) {}

void Surface::draw(const Point &p, const Color &c) {
  buffer[p.y * width + p.x] =
      ((uint32_t)c.r << 16) | (((uint32_t)c.g) << 8) | (((uint32_t)c.b) << 0);
}

void Surface::resizeHeight(int32_t newHeight) {
  buffer.resize(width * newHeight, 0xFFFFFF);
  height = newHeight;
}

void Surface::drawTo(SDL_Renderer *renderer, const SDL_Rect &src,
                     const SDL_Rect &dst) {
  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, dst.w, dst.h);
  uint32_t *texturePixels = nullptr;
  int texturePitch = 0;
  if (SDL_LockTexture(texture, NULL, (void **)&texturePixels, &texturePitch) ==
      0) {

    // std::cout<<"get"<<src<<"->"<<dst<<" from "<<width<<"x
    // "<<height<<std::endl;;
    //  Copy the specified portion of the surface to the texture
    for (int y = 0; y < dst.h; ++y) {
      for (int x = 0; x < dst.w; ++x) {
        texturePixels[y * texturePitch / sizeof(uint32_t) + x] =
            buffer[(src.y + y) * width + (src.x + x)];
      }
    }

    // Unlock the texture
    SDL_UnlockTexture(texture);
  } else {
    std::stringstream ss;
    ss << "Failed to lock texture: " << SDL_GetError() << std::endl;
    throw std::runtime_error(ss.str());
  }
  SDL_Rect all = {0, 0, dst.w, dst.h};
  SDL_SetRenderTarget(renderer, nullptr);
  SDL_RenderCopy(renderer, texture, &all, &dst);
  SDL_DestroyTexture(texture);
}

static const Color penColors[] = {{0x00, 0x00, 0x00},
                                  {0x00, 0x00, 0xFF},
                                  {0x00, 0x80, 0x00},
                                  {0x80, 0x00, 0x00}

};

void drawFilledCircle(SDL_Renderer *renderer, int centerX, int centerY,
                      int radius) {
  for (int y = -radius; y <= radius; ++y) {
    for (int x = -radius; x <= radius; ++x) {
      if (x * x + y * y <= radius * radius) {
        SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
      }
    }
  }
}

Texture::Texture(SDL_Renderer *renderer_, int32_t w, int32_t h)
    : renderer(renderer_), width(w), height(h) {
  pointsTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET, w, h);
  if (pointsTexture == nullptr) {
    std::stringstream ss;
    ss << "Texture could not be created! SDL_Error: " << SDL_GetError()
       << std::endl;
    throw std::runtime_error(ss.str());
  }
  SDL_SetRenderTarget(renderer, pointsTexture);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  std::cout << "creating page" << w << " x " << h << std::endl;
}

Texture::~Texture() { SDL_DestroyTexture(pointsTexture); }

void Texture::draw(const Point &p, const Color &c) {
  SDL_SetRenderTarget(renderer, pointsTexture);
  SDL_RenderDrawPoint(renderer, p.x, p.y);
}

void Texture::resizeHeight(int32_t h) {

  SDL_Texture *newTexture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, h);
  if (pointsTexture == nullptr) {
    std::stringstream ss;
    ss << "Texture could not be created! SDL_Error: " << SDL_GetError()
       << std::endl;
    throw std::runtime_error(ss.str());
  }

  SDL_SetRenderTarget(renderer, newTexture);
  SDL_Rect fillRect = {0, 0, width, height};

  SDL_RenderCopy(renderer, pointsTexture, &fillRect, &fillRect);
  SDL_SetRenderTarget(renderer, nullptr);
  SDL_DestroyTexture(pointsTexture);

  SDL_SetRenderTarget(renderer, newTexture);
  // Fill the newly created space with white
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  fillRect = {0, height, width, h - height};
  SDL_RenderFillRect(renderer, &fillRect);
  pointsTexture = newTexture;
  height = h;
}

Window::Window(int32_t w, int32_t h) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::stringstream ss;
    ss << "SDL could not initialize! SDL_Error: " << SDL_GetError()
       << std::endl;
    throw std::runtime_error(ss.str());
  }

  // Create window
  window = SDL_CreateWindow("SP 400", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::stringstream ss;
    ss << "Window could not be created! SDL_Error: " << SDL_GetError()
       << std::endl;
    throw std::runtime_error(ss.str());
  }

  // Create renderer
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    std::stringstream ss;
    ss << "Renderer could not be created! SDL_Error: " << SDL_GetError()
       << std::endl;
    throw std::runtime_error(ss.str());
  }
}

Window::~Window() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void Window::clear() {
  // Reset the render target to the default window
  SDL_SetRenderTarget(renderer, nullptr);

  SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 255);
  SDL_RenderClear(renderer);
}

Plotter::Plotter(Board &board_, int32_t w, int32_t h)
    : canvas_width(w), canvas_height(h), page_width(570), page_height(480),
      motOff(550, 240),
      pageOff((canvas_width - page_width) / 2, canvas_height / 2), head(0, 0),
      win(canvas_width, canvas_height), paper(page_width, page_height),
      board(board_) {}

void Plotter::makePage() {
  paper.resizeHeight(paper.getHeight() + page_height);
}

void Plotter::run() {
  bool quit = false;
  SDL_Event e;
  const Color *c = penColors;
  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }
    auto states = board.getStates();
    for (auto &s : states) {
      head.x = motOff.x + s.x;
      head.y = motOff.y + s.y;
      c = penColors + s.colorIdx;
      if (s.penDown) {
        paper.draw(head, *c);
      }
    }

    win.clear();
    while (head.y + page_height > paper.getHeight()) {
      makePage();
      std::cout << "made page of size " << paper.getHeight() << std::endl;
    }
    SDL_Rect srcRect = {0, std::max(0, head.y - pageOff.y), page_width,
                        canvas_height}; // Full texture area
    SDL_Rect destRect = {pageOff.x, 0, page_width,
                         canvas_height}; // Half window area
    paper.drawTo(win.renderer, srcRect, destRect);
    SDL_SetRenderDrawColor(win.renderer, c->r, c->g, c->b, 255);
    drawFilledCircle(win.renderer, head.x + pageOff.x, pageOff.y, 6);
    SDL_RenderPresent(win.renderer);
  }
}

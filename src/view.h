#ifndef VIEW_H
#define VIEW_H

#include <string>
#include <unordered_map>
#include "commons.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

namespace gamespace {

class Color {
public:
  const int r, g, b, a;
  const static Color RED, GREEN, BLUE, YELLOW, WHITE, BLACK,
  DARK_RED, DARK_GREEN, DARK_BLUE, DARK_YELLOW;
  // opaque by default
  Color(int red, int green, int blue, int alpha = 255)
      : r(red), g(green), b(blue), a(alpha) {}
  ~Color() {}
};

std::ostream& operator<<(std::ostream& os, const Color& c);
constexpr bool operator==(const Color& c1, const Color& c2);

class WindowManager {
public:
  inline bool isReady() const;
  void setDrawColor(const Color &c) const;
  bool drawLine(int x1, int y1, int x2, int y2, const Color &c) const;
  bool fillRect(int x, int y, int w, int h, const Color &c) const;
  bool drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                    const Color &c) const;
  bool fillBackground(const Color &c) const;
  void render() const;
  bool drawPoint(int x, int y, const Color &c) const;
  bool fillCircle(int x, int y, int r, const Color &c) const;
private:
  std::unordered_map<std::string, SDL_Texture *> textures;
  SDL_Window *window;
  SDL_Renderer *renderer;

public:
  WindowManager();
  ~WindowManager();
  bool startWindow();
  bool drawTexture(const char *imageName, const SDL_FRect *box) const;
  bool loadTexture(const char *c);
};

// forward declarations
class Piece;
class Player;

/**
 * @brief View in the MVC pattern
 */
class View {

private:
  WindowManager windowManager;
public:
  void render();
  void drawBoard();
  void drawPiece(int x, int y, const Color& c, int radius=TS/8);
  void drawConfigBase();
  void drawDice(const Color& c, int value);
  void preparePlayerDice(const Color& c);
  void highLightPosition(int x, int y, const Color& c, int width=TS-2, int height=TS-2);

private:
  bool drawStar(int x, int y, int side, const Color &c) const;

public:
  View();
  ~View();
};
} // namespace gamespace

#endif

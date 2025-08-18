#include <cmath>
#include <iostream>
#include <unordered_map>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <vector>

#include "SDL3/SDL_audio.h"
#include "SDL3/SDL_oldnames.h"
#include "commons.h"
#include "view.h"

using namespace gamespace;

std::ostream &gamespace::operator<<(std::ostream &os, const Color &c) {
  return os << "Color{" << c.r << ',' << c.g << ',' << c.b << ',' << c.a << '}';
}

const Color Color::WHITE(255, 255, 255);
const Color Color::BLACK(0, 0, 0);

const Color Color::RED(255, 110, 110);
const Color Color::GREEN(159, 237, 85);
const Color Color::BLUE(128, 217, 255);
const Color Color::YELLOW(255, 253, 112);

const Color Color::DARK_RED(186, 36, 36);
const Color Color::DARK_GREEN(90, 150, 33);
const Color Color::DARK_BLUE(33, 162, 217);
const Color Color::DARK_YELLOW(245, 208, 65);

WindowManager::WindowManager() : window(nullptr), renderer(nullptr) {
  if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO))
    (std::cerr << "SDL initialization error[" << SDL_GetError() << "]\n")
        .flush();
}

bool WindowManager::drawPoint(int x, int y, const Color &c) const {
  if (!isReady())
    return false;
  setDrawColor(c);
  return SDL_RenderPoint(renderer, x, y);
}

bool WindowManager::drawLine(int x1, int y1, int x2, int y2,
                             const Color &c) const {
  if (!isReady())
    return false;
  setDrawColor(c);
  return SDL_RenderLine(renderer, x1, y1, x2, y2);
}

void WindowManager::setDrawColor(const Color &c) const {
  if (!SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a))
    (std::cerr << "Render draw color setting error [" << SDL_GetError()
               << "]\n")
        .flush();
}

bool WindowManager::fillRect(int x, int y, int w, int h, const Color &c) const {
  static SDL_FRect frect{};
  if (!isReady())
    return false;
  setDrawColor(c);
  frect.x = (float)x, frect.y = (float)y, frect.w = (float)w,
  frect.h = (float)h;
  return SDL_RenderFillRect(renderer, &frect);
}

bool WindowManager::fillBackground(const Color &c) const {
  if (!isReady())
    return false;
  setDrawColor(c);
  if (!SDL_RenderClear(renderer)) {
    (std::cerr << "Background color rendering error [" << SDL_GetError()
               << "]\n")
        .flush();
    return false;
  }
  return true;
}

inline bool WindowManager::isReady() const {
  return window != nullptr && renderer != nullptr;
}

bool WindowManager::drawTexture(const char *imageName,
                                const SDL_FRect *box) const {
  if (!isReady())
    return false;
  if (!textures.contains(imageName))
    return false;
  return SDL_RenderTexture(renderer, textures.at(imageName), nullptr, box);
}

void WindowManager::render() const { SDL_RenderPresent(renderer); }

static inline double toRadians(double theta) {
  return theta * std::numbers::pi / 180;
}

bool View::drawStar(int x, int y, int side, const Color &c) const {
  int rBig{9 * side / 20};
  int rSmall{5 * side / 20}; // adjust later
  int x1, x2, y1, y2, x3, y3;
  for (int i = 0; i < 5; i++) {
    x1 = x + side / 2 +
         static_cast<int>(rBig * std::cos(toRadians(180 + 90 + 72 * i)));
    y1 = y + side / 2 +
         static_cast<int>(rBig * std::sin(toRadians(180 + 90 + 72 * i)));
    x2 = x + side / 2 +
         static_cast<int>(rSmall * std::cos(toRadians(180 + 90 + 36 + 72 * i)));
    y2 = y + side / 2 +
         static_cast<int>(rSmall * std::sin(toRadians(180 + 90 + 36 + 72 * i)));
    x3 = x + side / 2 +
         static_cast<int>(rSmall * std::cos(toRadians(180 + 90 - 36 + 72 * i)));
    y3 = y + side / 2 +
         static_cast<int>(rSmall * std::sin(toRadians(180 + 90 - 36 + 72 * i)));
    windowManager.drawLine(x1, y1, x2, y2, c);
    windowManager.drawLine(x1, y1, x3, y3, c);
  }
  return true;
}
bool WindowManager::loadTexture(const char *filename) {
  if (!isReady())
    return false;
  if (textures.contains(filename))
    return true;
  SDL_Texture *texture = IMG_LoadTexture(renderer, filename);
  if (texture == nullptr) {
    (std::cerr << "Could not load texture [" << filename << "]\n").flush();
    return false;
  }
  textures[filename] = texture;
  return true;
}

bool WindowManager::startWindow() {
  /*
    Creates the window and associated renderer, must only be called once.
    return: bool, wether the whole operation is successfull.
   */
  if (window != nullptr) {
    std::clog << "Window already started\n";
    std::clog.flush();
    return false;
  }
  window = SDL_CreateWindow("LudoCpp", WINDOW_SIZE, WINDOW_SIZE, 0);
  if (window == nullptr) {
    std::cerr << "SDL window creation error[" << SDL_GetError() << "]\n";
    std::cerr.flush();
    return false;
  }
  renderer = SDL_CreateRenderer(window, nullptr);
  if (renderer == nullptr) {
    std::cerr << "SDL renderer creation error[" << SDL_GetError() << "]\n";
    std::cerr.flush();
    return false;
  }

  if (!SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND)) {
    std::cerr << "SDL renderer blend mode error [" << SDL_GetError() << "]\n";
    std::cerr.flush();
    return false;
  }
  return true;
}

WindowManager::~WindowManager() {
  for (const auto &texture : textures)
    SDL_DestroyTexture(texture.second);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

View::View() : windowManager() {
  windowManager.startWindow();
  if (!windowManager.isReady())
    (std::cerr << "Can't draw, exiting\n").flush();
  // windowManager.loadTexture("star.png");
}

View::~View() {}

bool WindowManager::fillCircle(int x, int y, int r, const Color &c) const {
  int xMax{x + r}, xMin{x - r}, yMax{y + r}, yMin{y - r};
  bool result{true};
  double distance;
  setDrawColor(c);
  for (int x1 = xMin; x1 < xMax; x1++) {
    for (int y1 = yMin; y1 < yMax; y1++) {
      distance = std::sqrt(std::pow(x1 - x, 2) + std::pow(y1 - y, 2));
      if (distance < r)
        result &= SDL_RenderPoint(renderer, x1, y1);
    }
  }
  if (!result)
    (std::cerr << "Could not render point" << SDL_GetError()).flush();
  return result;
}

// still no clue why i have to add the gamespace namespace in here
constexpr bool gamespace::operator==(const Color &c1, const Color &c2) {
  return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}

static const std::unordered_map<char, std::pair<int, int>> colorToDiceOffsets{
    {'r', {3, 12}}, {'g', {3, 3}}, {'y', {12, 3}}, {'b', {12, 12}}};

static char colorToChar(const Color &c) {
  if (c == Color::RED)
    return 'r';
  if (c == Color::GREEN)
    return 'g';
  if (c == Color::YELLOW)
    return 'y';
  if (c == Color::BLUE)
    return 'b';
  else {
    std::cerr << "Goofy color" << std::endl;
    exit(0);
  }
}

void View::highLightPosition(int x, int y, const Color &c, int width,
                             int height) {
  const Color cTransparent{c.r, c.g, c.b, 127};
  windowManager.fillRect(x, y, width, height, cTransparent);
}

void View::drawDice(const Color &c, int value) {
  if (value < 1 || value > 6)
    exit(0); // error, TODO: clean later
  auto [x, y] = colorToDiceOffsets.at(colorToChar(c));
  int xCenter{x * TS}, yCenter{y * TS};
  if (value == 1 || value == 5 || value == 3) {
    windowManager.fillCircle(xCenter, yCenter, 2, Color::BLACK);
  }
  if (value == 2 || value == 6) {
    windowManager.fillCircle(xCenter - TS / 8, yCenter, 2, Color::BLACK);
    windowManager.fillCircle(xCenter + TS / 8, yCenter, 2, Color::BLACK);
  }
  if (value >= 3) {
    windowManager.fillCircle(xCenter - TS / 8, yCenter + TS / 8, 2,
                             Color::BLACK);
    windowManager.fillCircle(xCenter + TS / 8, yCenter - TS / 8, 2,
                             Color::BLACK);
  }
  if (value >= 4) {
    windowManager.fillCircle(xCenter + TS / 8, yCenter + TS / 8, 2,
                             Color::BLACK);
    windowManager.fillCircle(xCenter - TS / 8, yCenter - TS / 8, 2,
                             Color::BLACK);
  }
}

void View::preparePlayerDice(const Color &c) {
  // TODO: should probably just draw a square around,
  // optimization to be performed later
  // not DRY ... I don't care.
  auto [x, y] = colorToDiceOffsets.at(colorToChar(c));
  int xCenter{x * TS}, yCenter{y * TS};
  // TODO: should probably just draw a square around,
  // optimization to be performed later
  windowManager.fillRect(xCenter - TS / 4, yCenter - TS / 4, TS / 2, TS / 2,
                         Color::BLACK);
  windowManager.fillRect(xCenter - TS / 4 + 1, yCenter - TS / 4 + 1, TS / 2 - 2,
                         TS / 2 - 2, Color::WHITE);
}

void View::drawBoard() {
  // Bismillah

  // global background
  windowManager.fillBackground(Color::WHITE);

  // the four main player boxes
  windowManager.fillRect(0, 0, 6 * TS, 6 * TS, Color::GREEN);
  windowManager.fillRect(TS, TS, 4 * TS, 4 * TS, Color::WHITE);
  windowManager.fillRect(9 * TS, 0, 6 * TS, 6 * TS, Color::YELLOW);
  windowManager.fillRect(10 * TS, TS, 4 * TS, 4 * TS, Color::WHITE);
  windowManager.fillRect(0, 9 * TS, 6 * TS, 6 * TS, Color::RED);
  windowManager.fillRect(TS, 10 * TS, 4 * TS, 4 * TS, Color::WHITE);
  windowManager.fillRect(9 * TS, 9 * TS, 6 * TS, 6 * TS, Color::BLUE);
  windowManager.fillRect(10 * TS, 10 * TS, 4 * TS, 4 * TS, Color::WHITE);

  // last lines of each player
  for (int i = 1; i <= 5; i++) {
    windowManager.fillRect(7 * TS, i * TS, TS, TS, Color::YELLOW);
    windowManager.fillRect(i * TS, 7 * TS, TS, TS, Color::GREEN);
    windowManager.fillRect((i + 8) * TS, 7 * TS, TS, TS, Color::BLUE);
    windowManager.fillRect(7 * TS, (i + 8) * TS, TS, TS, Color::RED);
  }

  // start position of each player
  windowManager.fillRect(8 * TS, TS, TS, TS, Color::YELLOW);
  drawStar(8 * TS, TS, TS, Color::BLACK);
  windowManager.fillRect(TS, 6 * TS, TS, TS, Color::GREEN);
  drawStar(TS, TS * 6, TS, Color::BLACK);
  windowManager.fillRect(13 * TS, 8 * TS, TS, TS, Color::BLUE);
  drawStar(13 * TS, 8 * TS, TS, Color::BLACK);
  windowManager.fillRect(6 * TS, 13 * TS, TS, TS, Color::RED);
  drawStar(6 * TS, 13 * TS, TS, Color::BLACK);

  // the four other stars on the board
  drawStar(2 * TS, TS * 8, TS, Color::BLACK);
  drawStar(6 * TS, 2 * TS, TS, Color::BLACK);
  drawStar(12 * TS, 6 * TS, TS, Color::BLACK);
  drawStar(8 * TS, 12 * TS, TS, Color::BLACK);

  // central square triangles
  windowManager.drawTriangle(6 * TS, 6 * TS, 9 * TS, 6 * TS, 7.5 * TS, 7.5 * TS,
                             Color::YELLOW);
  windowManager.drawTriangle(6 * TS, 6 * TS, 6 * TS, 9 * TS, 7.5 * TS, 7.5 * TS,
                             Color::GREEN);
  windowManager.drawTriangle(9 * TS, 6 * TS, 9 * TS, 9 * TS, 7.5 * TS, 7.5 * TS,
                             Color::BLUE);
  windowManager.drawTriangle(6 * TS, 9 * TS, 9 * TS, 9 * TS, 7.5 * TS, 7.5 * TS,
                             Color::RED);

  // central square diagonals
  windowManager.drawLine(6 * TS, 6 * TS, 9 * TS, 9 * TS, Color::BLACK);
  windowManager.drawLine(6 * TS, 9 * TS, 9 * TS, 6 * TS, Color::BLACK);

  // square black borders
  for (int i = 6; i <= 9; i++) {
    windowManager.drawLine(i * TS, 9 * TS, i * TS, 15 * TS, Color::BLACK);
    windowManager.drawLine(i * TS, 0, i * TS, 6 * TS, Color::BLACK);
    windowManager.drawLine(9 * TS, i * TS, 15 * TS, i * TS, Color::BLACK);
    windowManager.drawLine(0, i * TS, 6 * TS, i * TS, Color::BLACK);
    windowManager.drawLine(0, i * TS, 6 * TS, i * TS, Color::BLACK);
  }
  for (int i = 1; i <= 6; i++) {
    windowManager.drawLine(i * TS, 6 * TS, i * TS, 9 * TS, Color::BLACK);
    windowManager.drawLine((i + 8) * TS, 6 * TS, (i + 8) * TS, 9 * TS,
                           Color::BLACK);
    windowManager.drawLine(6 * TS, i * TS, 9 * TS, i * TS, Color::BLACK);
    windowManager.drawLine(6 * TS, (i + 8) * TS, 9 * TS, (i + 8) * TS,
                           Color::BLACK);
  }

  // initial position circles
  windowManager.fillCircle(2 * TS, 2 * TS, TS / 2, Color::GREEN);
  windowManager.fillCircle(4 * TS, 4 * TS, TS / 2, Color::GREEN);
  windowManager.fillCircle(2 * TS, 4 * TS, TS / 2, Color::GREEN);
  windowManager.fillCircle(4 * TS, 2 * TS, TS / 2, Color::GREEN);

  windowManager.fillCircle(11 * TS, 11 * TS, TS / 2, Color::BLUE);
  windowManager.fillCircle(13 * TS, 13 * TS, TS / 2, Color::BLUE);
  windowManager.fillCircle(11 * TS, 13 * TS, TS / 2, Color::BLUE);
  windowManager.fillCircle(13 * TS, 11 * TS, TS / 2, Color::BLUE);

  windowManager.fillCircle(2 * TS, 11 * TS, TS / 2, Color::RED);
  windowManager.fillCircle(4 * TS, 13 * TS, TS / 2, Color::RED);
  windowManager.fillCircle(2 * TS, 13 * TS, TS / 2, Color::RED);
  windowManager.fillCircle(4 * TS, 11 * TS, TS / 2, Color::RED);

  windowManager.fillCircle(11 * TS, 2 * TS, TS / 2, Color::YELLOW);
  windowManager.fillCircle(13 * TS, 4 * TS, TS / 2, Color::YELLOW);
  windowManager.fillCircle(11 * TS, 4 * TS, TS / 2, Color::YELLOW);
  windowManager.fillCircle(13 * TS, 2 * TS, TS / 2, Color::YELLOW);

  for (auto [_, pair] : colorToDiceOffsets)
    windowManager.fillRect(pair.first * TS - TS / 4, pair.second * TS - TS / 4,
                           TS / 2, TS / 2, Color::BLACK);
}

static constexpr const Color &toDark(const Color &c) {
  if (c == Color::GREEN)
    return Color::DARK_GREEN;
  if (c == Color::RED)
    return Color::DARK_RED;
  if (c == Color::BLUE)
    return Color::DARK_BLUE;
  if (c == Color::YELLOW)
    return Color::DARK_YELLOW;
  else
    return Color::BLACK;
}

void View::drawPiece(int x, int y, const Color &c, int radius) {
  windowManager.fillCircle(x, y, radius, Color::BLACK);
  windowManager.fillCircle(x, y, radius - 2, toDark(c));
}

// thanks to this reply
// https://www.reddit.com/r/gamedev/comments/1l0tr5b/comment/mvn2y1c/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
bool WindowManager::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                                 const Color &c) const {
  float r{c.r / 255.0f}, g{c.g / 255.0f}, b{c.b / 255.0f}, a{c.a / 255.0f};
  std::vector<SDL_Vertex> vertices{SDL_Vertex{
                                       SDL_FPoint{(float)x1, (float)y1},
                                       {(float)r, g, b, a},
                                       SDL_FPoint{0, 0},
                                   },
                                   SDL_Vertex{
                                       SDL_FPoint{(float)x2, (float)y2},
                                       {r, g, b, a},
                                       SDL_FPoint{0, 0},
                                   },
                                   SDL_Vertex{
                                       SDL_FPoint{(float)x3, (float)y3},
                                       {r, g, b, a},
                                       SDL_FPoint{0, 0},
                                   }};
  // setDrawColor(c);
  if (!SDL_RenderGeometry(renderer, nullptr, vertices.data(), vertices.size(),
                          nullptr, 0)) {
    (std::cerr << "Could not render geometry [" << SDL_GetError() << "]\n")
        .flush();
    return false;
  }
  return true;
}

void View::render() { windowManager.render(); }

AudioManager::AudioManager()
    : diceRollAudio(nullptr), diceRollAudioLength(nullptr),
      diceRollAudiospec(nullptr), deviceId(0){
  if (!SDL_LoadWAV(DICE_ROLL_AUDIO_PATH, diceRollAudiospec, diceRollAudio,
                   diceRollAudioLength)) {
    std::cerr << "Could not load audio file " << DICE_ROLL_AUDIO_PATH << " ["
              << SDL_GetError() << "]" << std::endl;
    exit(0);
  }
  deviceId = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, diceRollAudiospec);
  if(deviceId==0){
    std::cerr << "Could not open default audio device " << SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK << " ["
              << SDL_GetError() << "]" << std::endl;
    exit(0);
  }
}

AudioManager::~AudioManager(){
  SDL_CloseAudioDevice(deviceId);
  SDL_free(diceRollAudio);
}

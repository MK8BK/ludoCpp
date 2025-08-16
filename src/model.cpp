#include <array>
#include <iostream>

#include <SDL3/SDL_events.h>
#include <string>
#include <unordered_map>

#include "SDL3/SDL_error.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_oldnames.h"
#include "commons.h"
#include "model.h"

using namespace gamespace;

// IDK WHY, but this requires the gamespace namespace prefix, otherwise not
// declared properly, see https://stackoverflow.com/a/29067357 for more details
std::ostream &gamespace::operator<<(std::ostream &os, const Player &p) {
  return os << "Player{" << p.type << ',' << p.color << '}';
}
std::ostream &gamespace::operator<<(std::ostream &os,
                                    const Player::PlayerColor &c) {
  os << "GColor{";
  if (c == Player::PlayerColor::RED)
    os << "RED";
  if (c == Player::PlayerColor::GREEN)
    os << "GREEN";
  if (c == Player::PlayerColor::YELLOW)
    os << "YELLOW";
  if (c == Player::PlayerColor::BLUE)
    os << "BLUE";
  return os << '}';
}
std::ostream &gamespace::operator<<(std::ostream &os,
                                    const Player::PlayerType &t) {
  os << "PlayerType{";
  if (t == Player::PlayerType::HUMAN)
    os << "HUMAN";
  if (t == Player::PlayerType::ROBOT)
    os << "ROBOT";
  return os << '}';
}
std::ostream &gamespace::operator<<(std::ostream &os, const BoardPosition &p) {
  return os << "BoardPosition{" << p.pos << '}';
}
std::ostream &gamespace::operator<<(std::ostream &os, Piece const &p) {
  return os << "Piece{" << (p._player) << ',' << p.pos << '}';
}

BoardPosition::BoardPosition(int position) : pos(position) {
  if (pos > 91 || pos < 0)
    (std::cerr << "Invalid position " << pos).flush();
}

constexpr bool BoardPosition::isInitialPosition() const {
  // stuck on square
  return pos >= 76 && pos <= 91;
}

constexpr bool BoardPosition::isFinalPosition(int position) {
  return position == 69 || position == 63 || position == 75 || position == 57;
}

constexpr bool BoardPosition::isFinalPosition() const {
  return BoardPosition::isFinalPosition(pos);
}

// #GoofyEncoding
int BoardPosition::getNext(int pos, const Player::PlayerColor &color) {
  // TODO: handle erroneous positions
  if (pos > 91 || pos < 0) {
    (std::cerr << "Invalid position " << pos).flush();
    return -1;
  }
  if (pos >= 76 && pos <= 79)
    return 0;
  if (pos >= 80 && pos <= 83)
    return 13;
  if (pos >= 84 && pos <= 87)
    return 26;
  if (pos >= 88 && pos <= 91)
    return 39;
  if (pos == 50 && color == Player::PlayerColor::RED)
    return 52;
  if (pos == 11 && color == Player::PlayerColor::GREEN)
    return 58;
  if (pos == 24 && color == Player::PlayerColor::YELLOW)
    return 64;
  if (pos == 37 && color == Player::PlayerColor::BLUE)
    return 70;
  if (pos <= 56 && pos >= 52 && color == Player::PlayerColor::RED)
    return pos + 1;
  if (pos <= 62 && pos >= 58 && color == Player::PlayerColor::GREEN)
    return pos + 1;
  if (pos <= 68 && pos >= 64 && color == Player::PlayerColor::YELLOW)
    return pos + 1;
  if (pos <= 74 && pos >= 70 && color == Player::PlayerColor::BLUE)
    return pos + 1;
  return (pos + 1) % 52;
}

bool Piece::canAdvance(int diceValue) const {
  // brief assuming diceValue \in [1,6]
  // TODO: verify
  if (pos.isFinalPosition())
    return false;
  if (pos.isInitialPosition())
    return diceValue == 6;
  int _pos{pos.pos};
  int nextPos;
  while (diceValue--) { // >0
    nextPos = BoardPosition::getNext(_pos, _player.color);
    if (nextPos == _pos)
      return false;
  }
  return true;
}

void Piece::advance(int diceValue) {
  // TODO check
  if (pos.isInitialPosition()) {
    pos.pos = BoardPosition::getNext(pos.pos, _player.color);
    return;
  }
  while (diceValue--)
    pos.pos = BoardPosition::getNext(pos.pos, _player.color);
}

const Player Piece::defaultPlayer(Player::PlayerType::ROBOT,
                                  Player::PlayerColor::RED);
Game::Game()
    : view(), dice(), players(0), playerIdToPieces(), phase(Phase::CONFIG),
      currentPlayer(0), currentPlayerPlayed(false), currentPlayerRolled(false),
      canAdvance(false), hightLightedPieces(0){
  // change later to use the config phase, for now assume 4 players
  // --------------------------------------------------------------
  players.push_back(
      Player(Player::PlayerType::HUMAN, Player::PlayerColor::RED));
  players.push_back(
      Player(Player::PlayerType::HUMAN, Player::PlayerColor::GREEN));
  players.push_back(
      Player(Player::PlayerType::HUMAN, Player::PlayerColor::YELLOW));
  players.push_back(
      Player(Player::PlayerType::HUMAN, Player::PlayerColor::BLUE));
  phase = Phase::PLAY;
  // --------------------------------------------------------------
  setUpPieces();
}

const std::array<int, 4> &Player::getJailPositions() const {
  static const std::unordered_map<Player::PlayerColor, std::array<int, 4>>
      jailPositions{{{Player::PlayerColor::RED, {76, 77, 78, 79}},
                     {Player::PlayerColor::GREEN, {80, 81, 82, 83}},
                     {Player::PlayerColor::YELLOW, {84, 85, 86, 87}},
                     {Player::PlayerColor::BLUE, {88, 89, 90, 91}}}};
  return jailPositions.at(color);
}

void Game::setUpPieces() {
  for (const Player &player : players) {
    const std::array<int, 4> &jailPositions = player.getJailPositions();
    for (int pos : jailPositions)
      playerIdToPieces[player.color].push_back(Piece(player, pos));
  }
}

// #GoofyEncoding
int BoardPosition::toPositionId(int x, int y) {
  // this code is shit, the circles are not aligned to the tiles
  if ((x == 1 || x == 2) && (y == 1 || y == 2))
    return 82;
  if ((x == 3 || x == 4) && (y == 1 || y == 2))
    return 83;
  if ((x == 1 || x == 2) && (y == 3 || y == 4))
    return 81;
  if ((x == 3 || x == 4) && (y == 3 || y == 4))
    return 80;
  if ((x == 10 || x == 11) && (y == 1 || y == 2))
    return 84;
  if ((x == 12 || x == 13) && (y == 1 || y == 2))
    return 85;
  if ((x == 10 || x == 11) && (y == 3 || y == 4))
    return 86;
  if ((x == 12 || x == 13) && (y == 3 || y == 4))
    return 87;
  if ((x == 3 || x == 4) && (y == 10 || y == 11))
    return 76;
  if ((x == 3 || x == 4) && (y == 13 || y == 12))
    return 77;
  if ((x == 1 || x == 2) && (y == 12 || y == 13))
    return 78;
  if ((x == 1 || x == 2) && (y == 10 || y == 11))
    return 79;

  if ((x == 12 || x == 13) && (y == 10 || y == 11))
    return 88;
  if ((x == 10 || x == 11) && (y == 10 || y == 11))
    return 89;
  if ((x == 10 || x == 11) && (y == 12 || y == 13))
    return 90;
  if ((x == 12 || x == 13) && (y == 12 || y == 13))
    return 91;

  static const std::unordered_map<std::string, int> offsetToPosition{
      {"6-13", 0},   {"6-12", 1},  {"6-11", 2},  {"6-10", 3},   {"6-9", 4},
      {"5-8", 5},    {"4-8", 6},   {"3-8", 7},   {"2-8", 8},    {"1-8", 9},
      {"0-8", 10},   {"0-7", 11},  {"0-6", 12},  {"1-6", 13},   {"2-6", 14},
      {"3-6", 15},   {"4-6", 16},  {"5-6", 17},  {"6-5", 18},   {"6-4", 19},
      {"6-3", 20},   {"6-2", 21},  {"6-1", 22},  {"6-0", 23},   {"7-0", 24},
      {"8-0", 25},   {"8-1", 26},  {"8-2", 27},  {"8-3", 28},   {"8-4", 29},
      {"8-5", 30},   {"9-6", 31},  {"10-6", 32}, {"11-6", 33},  {"12-6", 34},
      {"13-6", 35},  {"14-6", 36}, {"14-7", 37}, {"14-8", 38},  {"13-8", 39},
      {"12-8", 40},  {"11-8", 41}, {"10-8", 42}, {"9-8", 43},   {"8-9", 44},
      {"8-10", 45},  {"8-11", 46}, {"8-12", 47}, {"8-13", 48},  {"8-14", 49},
      {"7-14", 50},  {"6-14", 51}, {"7-13", 52}, {"7-12", 53},  {"7-11", 54},
      {"7-10", 55},  {"7-9", 56},  {"7-8", 57},  {"1-7", 58},   {"2-7", 59},
      {"3-7", 60},   {"4-7", 61},  {"5-7", 62},  {"6-7", 63},   {"7-1", 64},
      {"7-2", 65},   {"7-3", 66},  {"7-4", 67},  {"7-5", 68},   {"7-6", 69},
      {"13-7", 70},  {"12-7", 71}, {"11-7", 72}, {"10-7", 73},  {"9-7", 74},
      {"8-7", 75},   {"4-11", 76}, {"4-13", 77}, {"2-13", 78},  {"2-11", 79},
      {"4-4", 80},   {"2-4", 81},  {"2-2", 82},  {"4-2", 83},   {"11-2", 84},
      {"13-2", 85},  {"11-4", 86}, {"13-4", 87}, {"13-11", 88}, {"11-11", 89},
      {"11-13", 90}, {"13-13", 91}};
  std::string key{std::to_string(x) + '-' + std::to_string(y)};
  if (offsetToPosition.contains(key))
    return offsetToPosition.at(key);
  else
    return -1;
}

BoardPosition BoardPosition::toPosition(int x, int y) {
  return BoardPosition(BoardPosition::toPositionId(x, y));
}

std::pair<int, int> BoardPosition::toXYOffset(int pos) {
  static constexpr std::array<std::pair<int, int>, NUM_POSITIONS>
      positionToXYOffset(
          {{6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9},   {5, 8},   {4, 8},
           {3, 8},  {2, 8},  {1, 8},  {0, 8},  {0, 7},   {0, 6},   {1, 6},
           {2, 6},  {3, 6},  {4, 6},  {5, 6},  {6, 5},   {6, 4},   {6, 3},
           {6, 2},  {6, 1},  {6, 0},  {7, 0},  {8, 0},   {8, 1},   {8, 2},
           {8, 3},  {8, 4},  {8, 5},  {9, 6},  {10, 6},  {11, 6},  {12, 6},
           {13, 6}, {14, 6}, {14, 7}, {14, 8}, {13, 8},  {12, 8},  {11, 8},
           {10, 8}, {9, 8},  {8, 9},  {8, 10}, {8, 11},  {8, 12},  {8, 13},
           {8, 14}, {7, 14}, {6, 14}, {7, 13}, {7, 12},  {7, 11},  {7, 10},
           {7, 9},  {7, 8},  {1, 7},  {2, 7},  {3, 7},   {4, 7},   {5, 7},
           {6, 7},  {7, 1},  {7, 2},  {7, 3},  {7, 4},   {7, 5},   {7, 6},
           {13, 7}, {12, 7}, {11, 7}, {10, 7}, {9, 7},   {8, 7},   {4, 11},
           {4, 13}, {2, 13}, {2, 11}, {4, 4},  {2, 4},   {2, 2},   {4, 2},
           {11, 2}, {13, 2}, {11, 4}, {13, 4}, {13, 11}, {11, 11}, {11, 13},
           {13, 13}});
  return positionToXYOffset.at(pos);
}

std::pair<int, int> BoardPosition::toXYOffset() const {
  return BoardPosition::toXYOffset(pos);
}

Color gamespace::toPhysicalColor(const Player::PlayerColor &c) {
  if (c == Player::PlayerColor::BLUE)
    return Color::BLUE;
  if (c == Player::PlayerColor::GREEN)
    return Color::GREEN;
  if (c == Player::PlayerColor::RED)
    return Color::RED;
  if (c == Player::PlayerColor::YELLOW)
    return Color::YELLOW;
  else
    return Color::BLACK; // just in case more colors are added later
}
/**
 * Assumes the pieces are at the same BoardPosition.
 * At most 4 pieces can be displayed on one tile
 */
void Game::arrangePiecesAtPosition(std::vector<Piece> &pieces) {
  BoardPosition &position = pieces[0].pos;
  auto [x, y] = position.toXYOffset();
  view.drawPiece(x * TS + TS / 4, y * TS + TS / 4,
                 toPhysicalColor(pieces[0].getColor()));
}

void Game::drawPieces() {
  // VADE RETRO SATANA
  // 17 is the smallest prime larger than 16, there will be at most 16
  // pieces in a game
  std::unordered_map<int, std::vector<Piece>> positionToPieces(17);
  for (const auto &[id, pieces] : playerIdToPieces) {
    for (const Piece &p : pieces)
      positionToPieces[p.pos.pos].push_back(p);
  }
  for (auto [position, piecesHere] : positionToPieces) {
    if (piecesHere.empty())
      std::cerr << "Bruh goofed up big time" << std::endl;
    if (position >= 76) { // home square positions
      if (piecesHere.size() != 1) {
        (std::cerr << "Invalid board configuration at position " << position)
            .flush();
        return;
      }
      auto [x, y] = BoardPosition::toXYOffset(position);
      view.drawPiece(x * TS, y * TS, toPhysicalColor(piecesHere[0].getColor()));
      continue;
    } // else if
    arrangePiecesAtPosition(piecesHere);
  }
}

BoardPosition BoardPosition::fromScreenFloats(float x, float y) {
  // ignore inter circle space
  if (((x >= 2.5 && x <= 3.4) || (x >= 11.5 && x <= 12.40)) &&
      ((y >= 2.5 && y <= 3.4) || (y >= 11.5 && y <= 12.40)))
    return BoardPosition(-1);
  x = std::floor(x), y = std::floor(y);
  return BoardPosition::toPosition(x, y);
}

void Game::render() {
  if (phase == Phase::PLAY) {
    view.drawBoard();
    drawPieces();
    view.preparePlayerDice(toPhysicalColor(players.at(currentPlayer).color));
    if (currentPlayerRolled) {
      view.drawDice(toPhysicalColor(players[currentPlayer].color), dice.value);
      if (hightLightedPieces.size() > 0) {
        for (const Piece &p : hightLightedPieces) {
          if (p.pos.isInitialPosition())
            continue;
          auto [x, y] = p.pos.toXYOffset();
          view.highLightPosition(
              x * TS, y * TS,
              toPhysicalColor(
                  players.at(currentPlayer).color)); // TS width and height
        }
      }
    }
  }
  view.render();
}

constexpr bool gamespace::operator==(const BoardPosition &a,
                                     const BoardPosition &b) {
  return a.pos == b.pos;
}

void Game::handleEvent(const SDL_Event &event) {
  if (event.type == SDL_EVENT_KEY_DOWN) {
    SDL_Keycode key = event.key.key;
    if (key == SDLK_SPACE && !currentPlayerRolled) {
      dice.roll();
      currentPlayerRolled = true;
      hightLightedPieces.clear();
      canAdvance = false;
      for (const Piece &p : playerIdToPieces.at(currentPlayer)) {
        if (p.canAdvance(dice.value)){
          hightLightedPieces.push_back(p);
          canAdvance = true;
        }
      }
    }
    if(!canAdvance){
      currentPlayer = (currentPlayer+1)%4;
      canAdvance = currentPlayerPlayed = currentPlayerRolled = false;
    }
  } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    if (currentPlayerPlayed || !currentPlayerRolled)
      return;
    float x, y;
    if (!SDL_GetMouseState(&x, &y)) {
      (std::cerr << "Could not read mouse state [" << SDL_GetError() << "]")
          .flush();
      return;
    }
    x /= TS;
    y /= TS;
    const BoardPosition clickedPosition = BoardPosition::fromScreenFloats(x, y);
    std::vector<Piece> &playerPieces = playerIdToPieces.at(currentPlayer);
    for (Piece &p : playerPieces) {
      if (p.pos == clickedPosition) {
        if (!p.canAdvance(dice.value))
          continue;
        p.advance(dice.value);
        currentPlayer = (currentPlayer + 1) % 4;
        currentPlayerPlayed = currentPlayerRolled = false;
        break;
      }
    }
  }
}

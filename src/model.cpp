#include <array>
#include <iostream>

#include <SDL3/SDL_events.h>
#include <unordered_map>

#include "SDL3/SDL_keycode.h"
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

static inline bool isInitialPosition(int position) {
  // stuck on square
  return position >= 76 && position <= 91;
}

static inline bool isFinalPosition(int position) {
  return position == 69 || position == 63 || position == 75 || position == 57;
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
  if (isFinalPosition(pos.pos))
    return false;
  if (isInitialPosition(pos.pos))
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
  if (isInitialPosition(pos.pos)) {
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
      hightLightedPieces(0) {
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

static const std::unordered_map<Player::PlayerColor, std::array<int, 4>>
    colorToInitialPositions{{{Player::PlayerColor::RED, {76, 77, 78, 79}},
                             {Player::PlayerColor::GREEN, {80, 81, 82, 83}},
                             {Player::PlayerColor::YELLOW, {84, 85, 86, 87}},
                             {Player::PlayerColor::BLUE, {88, 89, 90, 91}}}};

void Game::setUpPieces() {
  for (const Player &player : players) {
    playerIdToPieces[player.color].push_back(
        Piece(player, colorToInitialPositions.at(player.color)[0]));
    playerIdToPieces[player.color].push_back(
        Piece(player, colorToInitialPositions.at(player.color)[1]));
    playerIdToPieces[player.color].push_back(
        Piece(player, colorToInitialPositions.at(player.color)[2]));
    playerIdToPieces[player.color].push_back(
        Piece(player, colorToInitialPositions.at(player.color)[3]));
  }
}

// #GoofyEncoding
static constexpr std::array<std::pair<int, int>, NUM_POSITIONS>
    positionToXYOffset({{6, 13},  {6, 12}, {6, 11}, {6, 10}, {6, 9},   {5, 8},
                        {4, 8},   {3, 8},  {2, 8},  {1, 8},  {0, 8},   {0, 7},
                        {0, 6},   {1, 6},  {2, 6},  {3, 6},  {4, 6},   {5, 6},
                        {6, 5},   {6, 4},  {6, 3},  {6, 2},  {6, 1},   {6, 0},
                        {7, 0},   {8, 0},  {8, 1},  {8, 2},  {8, 3},   {8, 4},
                        {8, 5},   {9, 6},  {10, 6}, {11, 6}, {12, 6},  {13, 6},
                        {14, 6},  {14, 7}, {14, 8}, {13, 8}, {12, 8},  {11, 8},
                        {10, 8},  {9, 8},  {8, 9},  {8, 10}, {8, 11},  {8, 12},
                        {8, 13},  {8, 14}, {7, 14}, {6, 14}, {7, 13},  {7, 12},
                        {7, 11},  {7, 10}, {7, 9},  {7, 8},  {1, 7},   {2, 7},
                        {3, 7},   {4, 7},  {5, 7},  {6, 7},  {7, 1},   {7, 2},
                        {7, 3},   {7, 4},  {7, 5},  {7, 6},  {13, 7},  {12, 7},
                        {11, 7},  {10, 7}, {9, 7},  {8, 7},  {4, 11},  {4, 13},
                        {2, 13},  {2, 11}, {4, 4},  {2, 4},  {2, 2},   {4, 2},
                        {11, 2},  {13, 2}, {11, 4}, {13, 4}, {13, 11}, {11, 11},
                        {11, 13}, {13, 13}});

static Color toPhysicalColor(const Player::PlayerColor &c) {
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
  int position = pieces[0].pos.pos;
  auto [x, y] = positionToXYOffset.at(position);
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
      auto [x, y] = positionToXYOffset[position];
      view.drawPiece(x * TS, y * TS, toPhysicalColor(piecesHere[0].getColor()));
      continue;
    } // else if
    arrangePiecesAtPosition(piecesHere);
  }
}

void Game::render() {
  if (phase == Phase::PLAY) {
    view.drawBoard();
    drawPieces();
    view.preparePlayerDice(toPhysicalColor(players.at(currentPlayer).color));
    if (currentPlayerRolled) {
     view.drawDice(toPhysicalColor(players[currentPlayer].color), dice.value);
     if(hightLightedPieces.size()>0){
       for(const Piece& p : hightLightedPieces){
        if(isInitialPosition(p.pos.pos)) continue;
         auto [x, y] = positionToXYOffset[p.pos.pos];
         view.highLightPosition(x*TS,y*TS, toPhysicalColor(players.at(currentPlayer).color)); // TS width and height
       }
     }
    }
  }
  view.render();
}

void Game::handleEvent(const SDL_Event &event) {
  if (event.type == SDL_EVENT_KEY_DOWN) {
    SDL_Keycode key = event.key.key;
    if (key == SDLK_SPACE && !currentPlayerRolled) {
      dice.roll();
      currentPlayerRolled = true;
      hightLightedPieces.clear();
      for(const Piece& p : playerIdToPieces.at(currentPlayer)){
        if(p.canAdvance(dice.value)) hightLightedPieces.push_back(p);
      }
    } else if (key == SDLK_UP && !currentPlayerPlayed &&
               currentPlayerRolled) { // replace later with mouse
      Piece &p = playerIdToPieces.at(currentPlayer)[0];
      if (p.canAdvance(dice.value)) {
        p.advance(dice.value);
      }
      currentPlayer = (currentPlayer + 1) % 4;
      currentPlayerPlayed = currentPlayerRolled = false;
    }
  }
}

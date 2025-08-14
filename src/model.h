#ifndef MODEL_H
#define MODEL_H

#include "view.h"
#include <ostream>
#include <random>
#include <unordered_map>
#include <vector>

namespace gamespace {

class BoardPosition;

class Player {
  /**
   * @brief Player representation class ... done
   *
   */
public:
  enum PlayerType { HUMAN, ROBOT };
  enum PlayerColor { RED=0, GREEN=1, YELLOW=2, BLUE=3}; // order of play on the board
  PlayerType type;
  PlayerColor color; // used as an id field
  // just used to satisfy defaultConstructible in 0 length
  // std::vector::constructor--(4)
  Player(PlayerType type = ROBOT, PlayerColor color = RED)
      : type(type), color(color) {}
  ~Player() = default;
  friend std::ostream& operator<<(std::ostream& os, const Player& p);
};

std::ostream& operator<<(std::ostream& os, const Player::PlayerColor& c);
std::ostream& operator<<(std::ostream& os, const Player::PlayerType& t);

class BoardPosition {
  /**
   * @brief Board position representation class ... done
   *
   */
public:
  int pos;
  static int getNext(int pos, const Player::PlayerColor &color);
  BoardPosition(int position = 0);
  static int defaultPosition(const Player::PlayerColor &color);
  friend std::ostream& operator<<(std::ostream& os, const BoardPosition& p);
};

class Piece {
  /**
   * @brief  ... done
   *
   */
private:
  Player _player;

public:
  BoardPosition pos;
  Player::PlayerColor getColor() const{return _player.color;};
  bool canAdvance(int diceValue) const;
  void advance(int diceValue);
  /**
   * @brief TODO: set position later depending on other similar colored pieces
   * @param p
   */

  // just used to satisfy defaultConstructible in 0 length
  // std::vector::constructor--(4)
  Piece(const Player &player = defaultPlayer, 
    const BoardPosition& position=BoardPosition(0)):_player(player), pos(position){}
  static const Player defaultPlayer;
  friend std::ostream& operator<<(std::ostream& os, Piece const& p);
};


class Dice {
public:
  int roll() { return value = dist(rd); }

private:
  std::random_device rd;
  std::uniform_int_distribution<int> dist;

public:
  int value;
  Dice() : rd(), dist(1, 6), value(6){} // TODO: change to -1 later
  ~Dice() = default;
};

class Game {
  enum Phase { CONFIG, PLAY };

public:
  void render();
  void handleEvent(const SDL_Event &event);
  Game();

private:
  View view;
  Dice dice;
  std::vector<Player> players;
  std::unordered_map<int,std::vector<Piece>> playerIdToPieces;
  Phase phase;
  int currentPlayer;
  bool currentPlayerPlayed;
  bool currentPlayerRolled;
  std::vector<Piece> hightLightedPieces;
  void drawPieces();
  void setUpPieces();
  void arrangePiecesAtPosition(std::vector<Piece>& pieces);
};
} // namespace gamespace
#endif

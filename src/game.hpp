#pragma once

#include "chesspiece.hpp"

/*
Game class starts the game: initialize and keep all shared data
and give all control to chess pieces.
*/
class Game {
 public:
  Game(int field_size, int chess_pieces_count);

  void Start();

  void Print() const;

 private:
  Field field;
  sys_clock::time_point last_move;
  std::vector<ChessPiece> chess_pieces;
  std::mutex m;
  std::condition_variable cv;
};

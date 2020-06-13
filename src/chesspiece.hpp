#pragma once

#include <condition_variable>
#include <vector>

using Field = std::vector<std::vector<bool>>;
using sys_clock = std::chrono::system_clock;

class Game;

struct Position {
  int row;
  int column;

  static Position GetRandomStartPosition(const Field& field);
  static Position GetRandomMovePosition(const Position& current_pos,
                                        int field_size);
};

/*
ChessPiece classes contain all game logic
and make changes in shared data.
*/
class ChessPiece {
 private:
  const int id;
  Position pos;
  int moves_count{0};

  // Shared data:
  Field& field;
  sys_clock::time_point& last_move;
  std::mutex& m;
  std::condition_variable& cv;
  //

  ChessPiece(int id, Field& field, sys_clock::time_point& last_move,
             std::mutex& m, std::condition_variable& cv);

  bool IsMoveBlocked(const Position& move_position) const;

  bool Run(const sys_clock::time_point& start_time, int& total_moves_count);

  // ChessPiece can be created only in Game.
  friend class Game;
};

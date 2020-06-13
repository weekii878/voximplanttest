#include "chesspiece.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>

namespace {

using namespace std::chrono;

constexpr seconds kMaxTimeToWaitMove{60};
constexpr seconds kTimeToWaitPathAvailability{5};
constexpr int kNeededMoves{50};

// Get num from [min, max] range.
int GetRandomInt(int min, int max) {
  static thread_local std::random_device rd;
  static thread_local std::mt19937 generator(rd());
  std::uniform_int_distribution<int> distribution(min, max);
  return distribution(generator);
}

std::string meta(int id, const sys_clock::time_point& start_time) {
  const duration<float> fs = sys_clock::now() - start_time;
  std::stringstream ss;
  ss << std::fixed << std::setprecision(3);
  ss << "[id=" << id << ", time_from_start=" << fs.count() << "s]";
  return ss.str();
}

}  // namespace

Position Position::GetRandomStartPosition(const Field& field) {
  Position pos;
  do {
    pos = {GetRandomInt(0, field.size() - 1),
           GetRandomInt(0, field.size() - 1)};
  } while (field[pos.row][pos.column]);
  return pos;
}

Position Position::GetRandomMovePosition(const Position& current_pos,
                                         int field_size) {
  const auto GetRandomIntExcept = [&field_size](int except) {
    int num;
    do {
      num = GetRandomInt(0, field_size - 1);
    } while (num == except);
    return num;
  };

  // Choose one of the direction: vertical or horizontal.
  // Then generate random position.
  return static_cast<bool>(GetRandomInt(0, 1))
             ? Position{GetRandomIntExcept(current_pos.row), current_pos.column}
             : Position{current_pos.row,
                        GetRandomIntExcept(current_pos.column)};
}

ChessPiece::ChessPiece(int id, Field& field, sys_clock::time_point& last_move,
                       std::mutex& m, std::condition_variable& cv)
    : id(id), field(field), last_move(last_move), m(m), cv(cv) {
  // Initialize start position.
  pos = Position::GetRandomStartPosition(field);
  field[pos.row][pos.column] = true;
}

bool ChessPiece::IsMoveBlocked(const Position& move_position) const {
  const auto GetMinMax = [](int first, int second) {
    return first <= second ? std::pair{first, second}
                           : std::pair{second, first};
  };

  // Check every cell of path that it is not busy.
  const auto [min_vertical, max_vertical] =
      GetMinMax(pos.row, move_position.row);
  const auto [min_horizontal, max_horizontal] =
      GetMinMax(pos.column, move_position.column);
  for (int i = min_vertical; i <= max_vertical; ++i) {
    for (int j = min_horizontal; j <= max_horizontal; ++j) {
      if (field[i][j] && !(i == pos.row && j == pos.column)) {
        return true;
      }
    }
  }
  return false;
}

bool ChessPiece::Run(const sys_clock::time_point& start_time,
                     int& total_moves_count) {
  while (moves_count < kNeededMoves) {
    const Position move_position{
        Position::GetRandomMovePosition(pos, field.size())};

    std::unique_lock<std::mutex> lock(m);
    // Check path availability every time chess piece moves.
    if (!cv.wait_for(lock, kTimeToWaitPathAvailability,
                     [&move_position, this]() {
                       return !this->IsMoveBlocked(move_position);
                     })) {
      // If kMaxTimeToWaitMove exceeded,
      // it means that chess piece blocked and can not move
      // or available paths count is too low.
      const auto now{sys_clock::now()};
      const auto diff = duration_cast<seconds>(now - last_move);
      if (diff > kMaxTimeToWaitMove) {
        return false;
      }

      // Use printf for comfortable formatting.
      printf(
          "%s path from (%d, %d) to (%d, %d) blocked. Try to find another "
          "path\n",
          meta(id, start_time).c_str(), pos.row, pos.column, move_position.row,
          move_position.column);

      // Try to find another path.
      continue;
    }

    std::swap(field[pos.row][pos.column],
              field[move_position.row][move_position.column]);
    last_move = sys_clock::now();
    printf("%s moved from (%d, %d) to (%d, %d). Total moves count= %d\n",
           meta(id, start_time).c_str(), pos.row, pos.column, move_position.row,
           move_position.column, ++total_moves_count);
    lock.unlock();

    // Notify that piece moved.
    cv.notify_all();
    pos = std::move(move_position);
    ++moves_count;
    std::this_thread::sleep_for(milliseconds(GetRandomInt(200, 300)));
  }

  return true;
}

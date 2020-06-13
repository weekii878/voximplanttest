#include "game.hpp"

#include <future>
#include <iostream>

Game::Game(int field_size, int chess_pieces_count)
    : field(field_size, std::vector(field_size, false)) {
  chess_pieces.reserve(chess_pieces_count);
  for (int i = 0; i != chess_pieces_count; ++i) {
    chess_pieces.push_back({i + 1, field, last_move, m, cv});
  }
}

void Game::Start() {
  last_move = sys_clock::now();

  std::vector<std::future<bool>> futures;
  futures.reserve(chess_pieces.size());
  // Use "now" and "total_moves_count" only for logging
  const auto now{sys_clock::now()};
  int total_moves_count{0};
  for (auto& piece : chess_pieces) {
    auto future = std::async(&ChessPiece::Run, piece, std::ref(now),
                             std::ref(total_moves_count));
    futures.push_back(std::move(future));
  }

  bool game_completed{true};
  for (auto& future : futures) {
    game_completed &= future.get();
  }

  if (!game_completed) {
    std::cout << "Game did not complete. Move waiting timed out." << std::endl;
  } else {
    std::cout << "Game completed successfully." << std::endl;
  }
}

void Game::Print() const {
  for (const auto& row : field) {
    for (auto cell : row) {
      std::cout << cell << "\t";
    }
    std::cout << std::endl;
  }
}

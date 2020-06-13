#include <iostream>

#include "game.hpp"

constexpr int kFieldSize{8};
constexpr int kChessPiecesCount{6};

int main() {
  static_assert(kFieldSize * kFieldSize >= kChessPiecesCount,
                "Field is too small");

  Game game(kFieldSize, kChessPiecesCount);
  game.Print();
  game.Start();
  game.Print();

  return 0;
}

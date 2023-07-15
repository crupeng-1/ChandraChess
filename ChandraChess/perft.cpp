#include <chrono>
#include <vector>
#include <iostream>
#include "perft.h"
#include "moveGenerator.h"
#include "state.h"
#include "updateState.h"
#include "board.h"
#include "bits.h"
int perft(int depth) {
  if (depth == 0) return 1;
  int result = 0;
  movesContainer moves;
  moves.numberOfMoves = 0;
  generateMoves(currentBoard, moves, false);
  if (depth == 1) return moves.numberOfMoves;
  for (int i = 0; i < moves.numberOfMoves; i++) {
    int move = moves.moveList[i].move;          
    makeMove(currentBoard, move);
    result += perft(depth - 1);
    takeMove(currentBoard, move);
  }
  return result;
}
void divide(int depth) {
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  std::cout << "Perft to depth " << depth << ":" << "\n";
  int result = 0;
  movesContainer moves;
  moves.numberOfMoves = 0;
  generateMoves(currentBoard, moves, false);
  for (int i = 0; i < moves.numberOfMoves; i++) {
    int move = moves.moveList[i].move;          
    makeMove(currentBoard, move);
    int rootResult = perft(depth - 1);
    std::cout << moveToLongAlgebraic(move) << ": " << rootResult << std::endl;
    takeMove(currentBoard, move);
    result += rootResult;
  }
  std::cout << "Total leaf nodes: " << result << "\n";
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  double timeElapsed = (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
  std::cout << (int)(((double)result / (timeElapsed / 1000.0)) / 1000.0) << " kN/s" << "\n\n";
}

#pragma once
#include <vector>
#include "state.h"
extern int hashMove;
extern int winningCaptures;
extern int equalCaptures;
extern int killerMoves;
extern int nonCaptures;
extern int losingCaptures;
struct moveEntry {
  int move;
  int score;
};
struct movesContainer {
  int numberOfMoves;
  moveEntry moveList[256];
};
void generateMoves(board& inputBoard, movesContainer& moves, bool isQuiescentGenerator);
void orderMoves(board& inputBoard, movesContainer& moves);

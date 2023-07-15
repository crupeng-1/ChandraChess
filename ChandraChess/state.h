#pragma once
#include <math.h>
#include <vector>
#include <string>
struct historyEntryRequiredForNullMove {
  int enPassantSquare;
  uint64_t currentKey;
  int halfMoveClock;
};
struct historyEntryNotRequiredForNullMove {
  int castlingPermission;
  int pieceCapturedOnNextMove;
};
struct board {
  int nodes;
  bool isSearchStopped;
  uint64_t bitboards[15];
  int sideToPlay;
  int castlingPermission;
  int enPassantSquare;
  int halfMoveClock;
  int halfMovesDone;
  uint64_t currentKey;
  int pieceCounts[12];
  historyEntryRequiredForNullMove historyRequiredForNullMove[512];
  historyEntryNotRequiredForNullMove historyNotRequiredForNullMove[512];
  int numberOfHistoryRequiredForNullMove;
  int numberOfHistoryNotRequiredForNullMove;
  int distanceToRoot;
  int historyTable[2][64][64];
  int killersTable[64][3];
  int middleGameEvaluation;
  int endGameEvaluation;
  int gamePhase;
};
// Stripped version of board for texel tuning malloc()
struct strippedBoard {
  uint64_t bitboards[15];
  int sideToPlay;
  int pieceCounts[12];
  int middleGameEvaluation;
  int endGameEvaluation;
  int gamePhase;
  int result;
};
extern board currentBoard;
extern int numberOfEntries;

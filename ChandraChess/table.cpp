#include <math.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include "board.h"
#include "state.h"
#include "table.h"
#include "updateState.h"
#include "moveGenerator.h"
int tableEntrySize = sizeof(tableEntry);
int lowerBound = 0;
int upperBound = 1;
int exact = 2;
tableEntry* table;
uint64_t syncronisationTable[65536];
void allocateTable(int size) {
  free(table);
  numberOfEntries = size / tableEntrySize;
  table = (tableEntry*)malloc(numberOfEntries * tableEntrySize);
}
void insertToTable(board& inputBoard, int move, int depth, int score, int type) {
  tableEntry& currentTableEntry = table[inputBoard.currentKey % numberOfEntries];
  int age = inputBoard.halfMovesDone - inputBoard.distanceToRoot;
  if (depth >= currentTableEntry.depth || currentTableEntry.age != age) {
    if (score >= 19937) {
      if (type == exact) {
        score = 19937;
        type = upperBound; // Atleast mate in 63 so we can get mate in 64 (19936).
      } else if (type == lowerBound) {  // Atmost mate
        return;
      }
    } else if (score <= -19937) {
      if (type == exact) {
        score = -19937;
        type = lowerBound; // Atleast mate in 63 so we can get mate in 64 (-19936).
      } else if (type == upperBound) { // Atmost mate
        return;
      }
    }
    currentTableEntry.key = inputBoard.currentKey;
    currentTableEntry.move = move;
    currentTableEntry.depth = depth;
    currentTableEntry.score = score;
    currentTableEntry.type = type;
    currentTableEntry.age = age;
  } 
} 
void startingSearch(board& inputBoard, int depth) {
  if (depth < 3) return;
  uint64_t& currentSyncronisationTableEntry = syncronisationTable[inputBoard.currentKey & 65535];
  if (currentSyncronisationTableEntry == 0ull) currentSyncronisationTableEntry = inputBoard.currentKey;
}
void endingSearch(board& inputBoard, int depth) {
  if (depth < 3) return;
  uint64_t& currentSyncronisationTableEntry = syncronisationTable[inputBoard.currentKey & 65535];
  if (currentSyncronisationTableEntry == inputBoard.currentKey) currentSyncronisationTableEntry = 0ull;
}
bool probeTableEntry(board& inputBoard, int depth, int alpha, int beta, int& score) {
  tableEntry& probedTableEntry = table[inputBoard.currentKey % numberOfEntries];
  if (probedTableEntry.key == inputBoard.currentKey && probedTableEntry.depth >= depth) {
    int value = probedTableEntry.score;
    int type = probedTableEntry.type;
    if (type == exact) {
      score = value;
      return true;
    }
    if ((type == upperBound && value <= alpha)) {
      score = alpha;
      return true;
    }
    if (type == lowerBound && value >= beta) {
      score = beta;
      return true;
    }
  }
  return false;
}
void retrievePrincipalVariation(board& inputBoard, std::vector<int>& principalVariation) {
  std::vector<uint64_t> probedHashKeys;
  while (true) {
    tableEntry& probedTableEntry = table[inputBoard.currentKey % numberOfEntries];
    if (probedTableEntry.key != inputBoard.currentKey) break;
    if (std::find(probedHashKeys.begin(), probedHashKeys.end(), inputBoard.currentKey) != probedHashKeys.end()) break;
    movesContainer moves;
    moves.numberOfMoves = 0;
    generateMoves(inputBoard, moves, false);
    bool isLegalMove = false;
    for (int i = 0; i < moves.numberOfMoves; i++) {
      if (moves.moveList[i].move == probedTableEntry.move) {
        isLegalMove = true;
        break;
      }
    }
    if (!isLegalMove) break;
    probedHashKeys.push_back(inputBoard.currentKey);
    principalVariation.push_back(probedTableEntry.move);
    makeMove(inputBoard, probedTableEntry.move);
  }
  for (int i = principalVariation.size() - 1; i >= 0; i--) takeMove(inputBoard, principalVariation[i]);
}
void clearTable() {
  for (int i = 0; i < numberOfEntries; i++) {
    tableEntry& currentTableEntry = table[i];
    currentTableEntry.key = 0ull;
    currentTableEntry.move = 0;
    currentTableEntry.depth = 0;
    currentTableEntry.score = 0;
    currentTableEntry.type = 0;
    currentTableEntry.age = 0;
  }
}

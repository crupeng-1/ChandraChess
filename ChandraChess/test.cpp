#include <algorithm>
#include <iostream>
#include <thread>
#include "bits.h"
#include "board.h"
#include "evaluation.h"
#include "hashKey.h"
#include "perft.h"
#include "search.h"
#include "state.h"
#include "state.h"
#include "table.h"
#include "moveGenerator.h"
#include "updateState.h"
int main() {
  numberOfThreads = 1;
  initializeMasks();
  initializeKeys();
  initializeEvaluation();
  allocateTable(128000000);
  setBoard(currentBoard, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
  setStartingEvaluationValues(currentBoard);
  timeToSearch = 10000;
  prepareForSearch(currentBoard);
  searchPosition(currentBoard);
  return 0;
}

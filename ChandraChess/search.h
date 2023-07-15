#pragma once
#include <string>
#include <vector>
#include "state.h"
extern double timeManagementTable[300][2];
extern int numberOfThreads;
extern int timeToSearch;
extern int depthLimit;
extern int minimumTimeRemaining;
extern volatile bool isInterruptedByGui;
void searchPosition(board& inputBoard);
void prepareForSearch(board& inputBoard);
extern volatile bool isCurrentlySearching;

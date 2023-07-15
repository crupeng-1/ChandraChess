#define numberOfWorkers 16
#define numberOfDatasetEntries 1428000
#include <algorithm>   
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include "math.h"
#include "bits.h"
#include "board.h"
#include "evaluation.h"
#include "moveGenerator.h"
#include "search.h"
#include "state.h"
#include "table.h"
#include "texelsTuning.h"
#include "updateState.h"
#include <algorithm>
#include <random>
int strippedBoardSize = sizeof(strippedBoard);
strippedBoard* loadedDataset;
double oneOverNumberOfDatasetEntries = 1.0 / double(numberOfDatasetEntries);
int numberOfDatasetEntriesPerThread = numberOfDatasetEntries / numberOfWorkers;
volatile bool shouldTerminateThreads = false;
volatile bool isWorkerRunning[numberOfWorkers];
double workerErrors[numberOfWorkers];
std::thread workerThreads[numberOfWorkers];
int numberOfParameters = 0;
std::vector<evaluationParameter> evaluationParameters;
std::string mobilityNames[2][4] = {
  {"queenMiddleGameMobilityBonuses", "rookMiddleGameMobilityBonuses", "bishopMiddleGameMobilityBonuses", "knightMiddleGameMobilityBonuses"},
  {"queenEndGameMobilityBonuses", "rookEndGameMobilityBonuses", "bishopEndGameMobilityBonuses", "knightEndGameMobilityBonuses"},
};
int mobilityCounts[4] = {28, 15, 14, 9};
double error() {
  initializeEvaluation();
  for (int i = 0; i < numberOfWorkers; i++) isWorkerRunning[i] = true;
  while (true) {
    bool shouldReturnResult = true;
    for (int i = 0; i < numberOfWorkers; i++) {
      if (isWorkerRunning[i]) {
        shouldReturnResult = false;
        break;
      }
    }
    if (shouldReturnResult) {
      double result = 0.0;
      for (int i = 0; i < numberOfWorkers; i++) result += workerErrors[i];
      return result;
    }
  }
}
void updateParameters(int index, bool& improved, double& bestError) {
  evaluationParameter& currentEvaluationParameter = evaluationParameters[index];
  int increment = currentEvaluationParameter.key < 10 ? 5 : 1;
  *currentEvaluationParameter.parameter += increment;
  double currentError = error();
  if (currentError < bestError) {
    bestError = currentError;
    improved = true;
  } else {
    *currentEvaluationParameter.parameter -= (2 * increment);
    currentError = error();
    if (currentError < bestError) {
      bestError = currentError;
      improved = true;
    } else {
      *currentEvaluationParameter.parameter += increment;
      return;
    }
  }
}
void computeError(int threadId) {
  while (true) {
    if (shouldTerminateThreads) return;
    if (isWorkerRunning[threadId]) {
      workerErrors[threadId] = 0.0;
      int start = numberOfDatasetEntriesPerThread * threadId;
      int end = start + numberOfDatasetEntriesPerThread;
      for (int i = start; i < end; i++) {
        strippedBoard& currentEntryBoard = loadedDataset[i];
        setStartingEvaluationValues(currentEntryBoard);
        double evaluationScore = 1.0 / (1.0 + std::pow(10.0, double(-(2 * currentEntryBoard.sideToPlay - 1) * evaluate(currentEntryBoard)) / 400.0));
        double targetScore = double(currentEntryBoard.result) / 2.0;
        workerErrors[threadId] += std::pow(targetScore - evaluationScore, 2.0);
      }
      workerErrors[threadId] *= oneOverNumberOfDatasetEntries;
      isWorkerRunning[threadId] = false;
    }
  }
}
void setEvaluationParameters() {
  for (int piece = 1; piece < 6; piece++) {
    evaluationParameter newEvaluationParameter;
    newEvaluationParameter.parameter = &middleGamePieceValues[piece];
    newEvaluationParameter.key = numberOfParameters;
    evaluationParameters.push_back(newEvaluationParameter);
    numberOfParameters++;
  }
  for (int piece = 1; piece < 6; piece++) {
    evaluationParameter newEvaluationParameter;
    newEvaluationParameter.parameter = &endGamePieceValues[piece];
    newEvaluationParameter.key = numberOfParameters;
    evaluationParameters.push_back(newEvaluationParameter);
    numberOfParameters++;
  }
  for (int piece = 0; piece < 6; piece++) {
    int start = piece == 5 ? 8 : 0;
    int end = piece == 5 ? 56 : 64;
    for (int square = start; square < end; square++) {
      evaluationParameter newEvaluationParameter;
      newEvaluationParameter.parameter = &middleGamePieceSquareTables[piece][square];
      newEvaluationParameter.key = numberOfParameters;
      evaluationParameters.push_back(newEvaluationParameter);
      numberOfParameters++;
    }
  }
  for (int piece = 0; piece < 6; piece++) {
    int start = piece == 5 ? 8 : 0;
    int end = piece == 5 ? 56 : 64;
    for (int square = start; square < end; square++) {
      evaluationParameter newEvaluationParameter;
      newEvaluationParameter.parameter = &endGamePieceSquareTables[piece][square];
      newEvaluationParameter.key = numberOfParameters;
      evaluationParameters.push_back(newEvaluationParameter);
      numberOfParameters++;
    }
  }
  for (int rank = 1; rank < 7; rank++) {
    evaluationParameter newEvaluationParameter;
    newEvaluationParameter.parameter = &passedPawnBonuses[rank];
    newEvaluationParameter.key = numberOfParameters;
    evaluationParameters.push_back(newEvaluationParameter);
    numberOfParameters++;
  }
  // for (int piece = 0; piece < 4; piece++) {
  //   evaluationParameter newEvaluationParameter;
  //   newEvaluationParameter.parameter = &attackerWeightsByPiece[piece];
  //   newEvaluationParameter.key = numberOfParameters;
  //   evaluationParameters.push_back(newEvaluationParameter);
  //   numberOfParameters++;
  // }
  // for (int weight = 1; weight < 8; weight++) {
  //   evaluationParameter newEvaluationParameter;
  //   newEvaluationParameter.parameter = &attackerWeightsByNumber[weight];
  //   newEvaluationParameter.key = numberOfParameters;
  //   evaluationParameters.push_back(newEvaluationParameter);
  //   numberOfParameters++;
  // }

  evaluationParameter isolatedPawnPenaltyParameter;
  isolatedPawnPenaltyParameter.parameter = &isolatedPawnPenalty;
  isolatedPawnPenaltyParameter.key = numberOfParameters;
  evaluationParameters.push_back(isolatedPawnPenaltyParameter);
  numberOfParameters++;

  evaluationParameter backwardPawnPenaltyParameter;
  backwardPawnPenaltyParameter.parameter = &backwardPawnPenalty;
  backwardPawnPenaltyParameter.key = numberOfParameters;
  evaluationParameters.push_back(backwardPawnPenaltyParameter);
  numberOfParameters++;

  evaluationParameter bishopPairBonusParameter;
  bishopPairBonusParameter.parameter = &bishopPairBonus;
  bishopPairBonusParameter.key = numberOfParameters;
  evaluationParameters.push_back(bishopPairBonusParameter);
  numberOfParameters++;

  for (int i = 0; i < 7; i++) {
    evaluationParameter farFromOpponentKingParameter;
    farFromOpponentKingParameter.parameter = &farFromOpponentKing[i];
    farFromOpponentKingParameter.key = numberOfParameters;
    evaluationParameters.push_back(farFromOpponentKingParameter);
    numberOfParameters++;
  }

  for (int i = 0; i < 7; i++) {
    evaluationParameter farFromOurKingParameter;
    farFromOurKingParameter.parameter = &farFromOurKing[i];
    farFromOurKingParameter.key = numberOfParameters;
    evaluationParameters.push_back(farFromOurKingParameter);
    numberOfParameters++;
  }

  for (int side = 0; side < 2; side++) {
    for (int piece = 0; piece < 4; piece++) {
      int mobilityCount = mobilityCounts[piece];
      for (int i = 0; i < mobilityCount; i++) {
        evaluationParameter newEvaluationParameter;
        newEvaluationParameter.parameter = &mobilityBonuses[side][piece][i];
        newEvaluationParameter.key = numberOfParameters;
        evaluationParameters.push_back(newEvaluationParameter);
        numberOfParameters++;
      }
    }
  }
}
void writeEvaluationParameters() {
  std::ofstream output;
  output.open("parameters.txt", std::ofstream::out | std::ofstream::trunc);

  output << "int middleGamePieceValues[6] = {";
  for (int i = 0; i < 6; i++) {
    output << middleGamePieceValues[i];
    if (i != 5) output << ", ";
  }
  output << "};\n";

  output << "int endGamePieceValues[6] = {";
  for (int i = 0; i < 6; i++) {
    output << endGamePieceValues[i];
    if (i != 5) output << ", ";
  }
  output << "};\n";

  output << "int middleGamePieceSquareTables[6][64] = {\n";
  for (int i = 0; i < 6; i++) {
    output << "  {\n";
    for (int y = 0; y < 8; y++) {
      output << "    ";
      for (int x = 0; x < 8; x++) {
        int square = 8 * y + x;
        output << middleGamePieceSquareTables[i][square];
        if (square != 64) output << ", ";
      }
      output << "\n";
    }
    output << "  }";
    if (i != 5) output << ",";
    output << "\n";
  }
  output << "};\n";

  output << "int endGamePieceSquareTables[6][64] = {\n";
  for (int i = 0; i < 6; i++) {
    output << "  {\n";
    for (int y = 0; y < 8; y++) {
      output << "    ";
      for (int x = 0; x < 8; x++) {
        int square = 8 * y + x;
        output << endGamePieceSquareTables[i][square];
        if (square != 64) output << ", ";
      }
      output << "\n";
    }
    output << "  }";
    if (i != 5) output << ",";
    output << "\n";
  }
  output << "};\n";

  output << "int passedPawnBonuses[8] = {";
  for (int i = 0; i < 8; i++) {
    output << passedPawnBonuses[i];
    if (i != 7) output << ", ";
  }
  output << "};\n";

  output << "int isolatedPawnPenalty = " << isolatedPawnPenalty << ";\n";
  output << "int backwardPawnPenalty = " << backwardPawnPenalty << ";\n";

  output << "int farFromOpponentKing[7] = {";
  for (int i = 0; i < 7; i++) {
    output << farFromOpponentKing[i];
    if (i != 6) output << ", ";
  }
  output << "};\n";

  output << "int farFromOurKing[7] = {";
  for (int i = 0; i < 7; i++) {
    output << farFromOurKing[i];
    if (i != 6) output << ", ";
  }
  output << "};\n";
  
  output << "int bishopPairBonus = " << bishopPairBonus << ";\n";

  // output << "int attackerWeightsByPiece[4] = {";
  // for (int i = 0; i < 4; i++) {
  //   output << attackerWeightsByPiece[i];
  //   if (i != 3) output << ", ";
  // }
  // output << "};\n";

  // output << "int attackerWeightsByNumber[8] = {";
  // for (int i = 0; i < 8; i++) {
  //   output << attackerWeightsByNumber[i];
  //   if (i != 7) output << ", ";
  // }

  // output << "};\n";
  
  for (int side = 0; side < 2; side++) {
    for (int piece = 0; piece < 4; piece++) {
      int mobilityCount = mobilityCounts[piece];
      int mobilityCountMinusOne = mobilityCount - 1;
      std::string mobilityName = mobilityNames[side][piece];
      output << "int " << mobilityName << "[" << mobilityCount << "] = {";
      for (int i = 0; i < mobilityCount; i++) {
        output << mobilityBonuses[side][piece][i];
        if (i != mobilityCountMinusOne) output << ", ";
      }
      output << "};\n";
    }
  }
  output.close();
}
void normalizeParameters() {
  for (int piece = 0; piece < 6; piece++) {
    int middleGamePieceSquareTableMean = 0;
    int endGamePieceSquareTableMean = 0;
    int start = piece == 5 ? 8 : 0;
    int end = piece == 5 ? 56 : 64;
    int squareCount = piece == 5 ? 48 : 64;
    for (int square = 0; square < 64; square++) {
      middleGamePieceSquareTableMean += middleGamePieceSquareTables[piece][square];
      endGamePieceSquareTableMean += endGamePieceSquareTables[piece][square];
    }
    middleGamePieceSquareTableMean = round(double(middleGamePieceSquareTableMean) / double(squareCount));
    endGamePieceSquareTableMean = round(double(endGamePieceSquareTableMean) / double(squareCount));
    for (int square = start; square < end; square++) {
      middleGamePieceSquareTables[piece][square] -= middleGamePieceSquareTableMean;
      endGamePieceSquareTables[piece][square] -= endGamePieceSquareTableMean;
    }
    if (piece == 0) continue;
    middleGamePieceValues[piece] += middleGamePieceSquareTableMean;
    endGamePieceValues[piece] += endGamePieceSquareTableMean;
    if (piece == 5) continue;
    int middleGameMobilityMean = 0;
    int endGameMobilityMean = 0;
    int relativePiece = piece - 1;
    int mobilityCount = mobilityCounts[relativePiece];
    for (int i = 0; i < mobilityCount; i++) {
      middleGameMobilityMean += mobilityBonuses[0][relativePiece][i];
      endGameMobilityMean += mobilityBonuses[1][relativePiece][i];
    }
    middleGameMobilityMean = round(double(middleGameMobilityMean) / double(mobilityCount));
    endGameMobilityMean = round(double(endGameMobilityMean) / double(mobilityCount));
    for (int i = 0; i < mobilityCount; i++) {
      mobilityBonuses[0][relativePiece][i] -= middleGameMobilityMean;
      mobilityBonuses[1][relativePiece][i] -= endGameMobilityMean;
    }
    middleGamePieceValues[piece] += middleGameMobilityMean;
    endGamePieceValues[piece] += endGameMobilityMean;
  }
}
int main() {
  initializeMasks();
  loadedDataset = (strippedBoard*)malloc(numberOfDatasetEntries * strippedBoardSize);
  for (int i = 0; i < numberOfWorkers; i++) workerThreads[i] = std::thread(computeError, i);
  std::ifstream file("dataset.txt");
  std::string line;
  int datasetIndex = 0;
  while (std::getline(file, line)) {
    std::vector<std::string> tuple;
    int firstQuotesIndex = line.find('"');
    std::string fen = line.substr(0, firstQuotesIndex - 3);
    int secondQuotesIndex = line.find('"', firstQuotesIndex + 1);
    std::string score = line.substr(firstQuotesIndex + 1, secondQuotesIndex - firstQuotesIndex - 1);
    strippedBoard emptyNewBoard;
    setBoard(emptyNewBoard, fen);
    if (score == "0-1") {
      emptyNewBoard.result = 0;
    } else if (score == "1/2-1/2") {
      emptyNewBoard.result = 1;
    } else {
      emptyNewBoard.result = 2;
    }
    loadedDataset[datasetIndex] = emptyNewBoard;
    datasetIndex++;
  }
  file.close();
  setEvaluationParameters();
  std::random_device rd;
  double bestError = error();
  writeEvaluationParameters();
  while (true) {
    bool improved = false;
    // Shuffled to prevent local minimums and predictable part across error curve.
    std::shuffle(evaluationParameters.begin(), evaluationParameters.end(), rd);
    for (int j = 0; j < numberOfParameters; j++) {
      std::cout << "Error: " << bestError << " " << j << "/" << numberOfParameters << std::endl;
      updateParameters(j, improved, bestError);
    }
    normalizeParameters();
    writeEvaluationParameters();
    if (!improved) break;
  }
  shouldTerminateThreads = true;
  for (int i = 0; i < numberOfWorkers; i++) {
    if (workerThreads[i].joinable()) {
      workerThreads[i].join();
    }
  } 
  return 0;
}

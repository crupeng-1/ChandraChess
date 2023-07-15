#pragma once
#include <string>
#include <math.h>
#include "bits.h"
#include "hashKey.h"
#include "state.h"
template <typename boardType> void setBoard(boardType& inputBoard, std::string fen) {
  std::vector<std::string> splitFen;
  int currentTokenIndex = 0;
  for (int i = 0; i < fen.size(); i++) {
    if (currentTokenIndex == splitFen.size()) {
      std::string emptyString;
      splitFen.push_back(emptyString);
    }
    char token = fen[i];
    if (token != ' ') {
      splitFen[currentTokenIndex] += token;
    } else {
      currentTokenIndex++;
    }
  }
  std::string boardString = splitFen[0];
  std::string side = splitFen[1];
  std::string castling = splitFen[2];
  std::string enPassant = splitFen[3];
  int boardSize = boardString.size();
  int index = 0;
  for (int i = 0; i < 15; i++) inputBoard.bitboards[i] = 0ull;
  for (int i = 0; i < 12; i++) inputBoard.pieceCounts[i] = 0;
  for (int i = 0; i < boardSize; i++) {
    char character = boardString[i];
    if (character == '/') {
      continue;
    }
    if (isdigit(character)) {
      index += (character - '0');
      continue;
    }
    uint64_t bit = bits[index];
    switch (character) {
      case 'k':
        inputBoard.bitboards[0] |= bit;
        inputBoard.bitboards[12] |= bit;
        inputBoard.pieceCounts[0]++;
        break;
      case 'q':
        inputBoard.bitboards[1] |= bit;
        inputBoard.bitboards[12] |= bit;
        inputBoard.pieceCounts[1]++;
        break;
      case 'r':
        inputBoard.bitboards[2] |= bit;
        inputBoard.bitboards[12] |= bit;
        inputBoard.pieceCounts[2]++;
        break;
      case 'b':
        inputBoard.bitboards[3] |= bit;
        inputBoard.bitboards[12] |= bit;
        inputBoard.pieceCounts[3]++;
        break;
      case 'n':
        inputBoard.bitboards[4] |= bit;
        inputBoard.bitboards[12] |= bit;
        inputBoard.pieceCounts[4]++;
        break;
      case 'p':
        inputBoard.bitboards[5] |= bit;
        inputBoard.bitboards[12] |= bit;
        inputBoard.pieceCounts[5]++;
        break;
      case 'K':
        inputBoard.bitboards[6] |= bit;
        inputBoard.bitboards[13] |= bit;
        inputBoard.pieceCounts[6]++;
        break;
      case 'Q':
        inputBoard.bitboards[7] |= bit;
        inputBoard.bitboards[13] |= bit;
        inputBoard.pieceCounts[7]++;
        break;
      case 'R':
        inputBoard.bitboards[8] |= bit;
        inputBoard.bitboards[13] |= bit;
        inputBoard.pieceCounts[8]++;
        break;
      case 'B':
        inputBoard.bitboards[9] |= bit;
        inputBoard.bitboards[13] |= bit;
        inputBoard.pieceCounts[9]++;
        break;
      case 'N':
        inputBoard.bitboards[10] |= bit;
        inputBoard.bitboards[13] |= bit;
        inputBoard.pieceCounts[10]++;
        break;
      case 'P':
        inputBoard.bitboards[11] |= bit;
        inputBoard.bitboards[13] |= bit;
        inputBoard.pieceCounts[11]++;
        break;
    }
    inputBoard.bitboards[14] |= bit;
    index++;
  }
  inputBoard.sideToPlay = side == "b" ? 0 : 1;
  if constexpr (std::is_same_v<boardType, board>) {
    inputBoard.numberOfHistoryNotRequiredForNullMove = 0;
    inputBoard.numberOfHistoryRequiredForNullMove = 0;
    if (enPassant == "-") {
      inputBoard.enPassantSquare = 0;
    } else {
      char rank = enPassant[0];
      char file = enPassant[1];
      inputBoard.enPassantSquare = 8 * (8 - (file - '0')) + rank - 'a';
    }
    inputBoard.castlingPermission = 0;
    int castlingSize = castling.size();
    for (int i = 0; i < castlingSize; i++) {
      char character = castling[i];
      switch (character) {
        case 'k':
          inputBoard.castlingPermission |= castlingPermissions[0];
          break;
        case 'q':
          inputBoard.castlingPermission |= castlingPermissions[1];
          break;
        case 'K':
          inputBoard.castlingPermission |= castlingPermissions[2];
          break;
        case 'Q':
          inputBoard.castlingPermission |= castlingPermissions[3];
          break;
      }
    }
    if (splitFen.size() > 4) {
      inputBoard.halfMoveClock = stoi(splitFen[4]);
      inputBoard.halfMovesDone = 2 * (stoi(splitFen[5]) - 1) + (inputBoard.sideToPlay ^ 1);
    } else {
      inputBoard.halfMoveClock = 0;
      inputBoard.halfMovesDone = 0;
    }
    inputBoard.currentKey = getKey(inputBoard);
  }
}
void printBoard(board& inputBoard);
template <typename boardType> uint64_t msbRayAttacks(boardType& inputBoard, int square, int ray) {
  uint64_t attacks = rayAttacks[ray][square];
  uint64_t blockers = attacks & inputBoard.bitboards[14];
  int blockerSquare = msbPosition(blockers | 1ull);
  return attacks ^ rayAttacks[ray][blockerSquare];
};
template <typename boardType> uint64_t lsbRayAttacks(boardType& inputBoard, int square, int ray) {
  uint64_t attacks = rayAttacks[ray][square];
  uint64_t blockers = attacks & inputBoard.bitboards[14];
  int blockerSquare = lsbPosition(blockers | 0x8000000000000000ull);
  return attacks ^ rayAttacks[ray][blockerSquare];
};
// We have lots of cases where we don't know the type of ray beforehand.
template <typename boardType> uint64_t generalRayAttacks(boardType& inputBoard, int square, int ray) { 
  if (ray < 4) return msbRayAttacks(inputBoard, square, ray);
  return lsbRayAttacks(inputBoard, square, ray);
}
bool isSquareOfSideToPlayAttacked(board& inputBoard, int square);
uint64_t piecesAttackingSquareOfSideToPlay(board& inputBoard, int square);
uint64_t piecesAttackingSquare(board& inputBoard, int square);
bool isInsufficientMaterial(board& inputBoard);
int getPiece(board& inputBoard, int square);
void removePiece(board& inputBoard, int square, bool isUpdatingKey);
void addPiece(board& inputBoard, int piece, int square, bool isUpdatingKey);
void movePiece(board& inputBoard, int from, int to, bool isUpdatingKey);
int longAlgebraicToMove(std::string longAlgebraic, board& inputBoard);
std::string moveToLongAlgebraic(int move);
bool isRepetition(board& inputBoard);

#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <math.h>
#include "board.h"
#include "bits.h"
#include "evaluation.h"
#include "hashKey.h"
#include "moveGenerator.h"
#include "table.h"
#include "state.h"
#include "updateState.h"
char pieces[12] = {'k', 'q', 'r', 'b', 'n', 'p', 'K', 'Q', 'R', 'B', 'N', 'P'};
char castlingStrings[4] = {'k', 'q', 'K', 'Q'};
char promotionPieces[4] = {'q', 'r', 'b', 'n'};
bool isSquareOfSideToPlayAttacked(board& inputBoard, int square) {
  int theirOffset = 6 * (inputBoard.sideToPlay ^ 1);
  uint64_t theirKingBitboard = inputBoard.bitboards[theirOffset];
  uint64_t theirQueenBitboard = inputBoard.bitboards[theirOffset + 1];
  uint64_t theirRookBitboard = inputBoard.bitboards[theirOffset + 2];
  uint64_t theirBishopBitboard = inputBoard.bitboards[theirOffset + 3];
  if ((kingAttacks[square] & theirKingBitboard) != 0ull) return true;
  if (((msbRayAttacks(inputBoard, square, 0) | msbRayAttacks(inputBoard, square, 2) | lsbRayAttacks(inputBoard, square, 4) | lsbRayAttacks(inputBoard, square, 6)) & (theirQueenBitboard | theirRookBitboard)) != 0ull) return true;
  if (((msbRayAttacks(inputBoard, square, 1) | msbRayAttacks(inputBoard, square, 3) | lsbRayAttacks(inputBoard, square, 5) | lsbRayAttacks(inputBoard, square, 7)) & (theirQueenBitboard | theirBishopBitboard)) != 0ull) return true;
  if ((knightAttacks[square] & inputBoard.bitboards[theirOffset + 4]) != 0ull) return true;
  if ((pawnAttacks[inputBoard.sideToPlay][square] & inputBoard.bitboards[theirOffset + 5]) != 0ull) return true;
  return false;
}
uint64_t piecesAttackingSquareOfSideToPlay(board& inputBoard, int square) {
  uint64_t attackers = 0ull;
  int theirOffset = 6 * (inputBoard.sideToPlay ^ 1);
  uint64_t theirKingBitboard = inputBoard.bitboards[theirOffset];
  uint64_t theirQueenBitboard = inputBoard.bitboards[theirOffset + 1];
  uint64_t theirRookBitboard = inputBoard.bitboards[theirOffset + 2];
  uint64_t theirBishopBitboard = inputBoard.bitboards[theirOffset + 3];
  attackers |= kingAttacks[square] & theirKingBitboard;
  attackers |= (msbRayAttacks(inputBoard, square, 0) | msbRayAttacks(inputBoard, square, 2) | lsbRayAttacks(inputBoard, square, 4) | lsbRayAttacks(inputBoard, square, 6)) & (theirQueenBitboard | theirRookBitboard);
  attackers |= (msbRayAttacks(inputBoard, square, 1) | msbRayAttacks(inputBoard, square, 3) | lsbRayAttacks(inputBoard, square, 5) | lsbRayAttacks(inputBoard, square, 7)) & (theirQueenBitboard | theirBishopBitboard);
  attackers |= knightAttacks[square] & inputBoard.bitboards[theirOffset + 4];
  attackers |= pawnAttacks[inputBoard.sideToPlay][square] & inputBoard.bitboards[theirOffset + 5];
  return attackers;
}
uint64_t piecesAttackingSquare(board& inputBoard, int square) {
  uint64_t attackers = 0ull;
  uint64_t queenBitboard = inputBoard.bitboards[1] | inputBoard.bitboards[7];
  uint64_t rookBitboard = inputBoard.bitboards[2] | inputBoard.bitboards[8];
  uint64_t bishopBitboard = inputBoard.bitboards[3] | inputBoard.bitboards[9];
  attackers |= kingAttacks[square] & (inputBoard.bitboards[0] | inputBoard.bitboards[6]);
  attackers |= (msbRayAttacks(inputBoard, square, 0) | msbRayAttacks(inputBoard, square, 2) | lsbRayAttacks(inputBoard, square, 4) | lsbRayAttacks(inputBoard, square, 6)) & (queenBitboard | rookBitboard);
  attackers |= (msbRayAttacks(inputBoard, square, 1) | msbRayAttacks(inputBoard, square, 3) | lsbRayAttacks(inputBoard, square, 5) | lsbRayAttacks(inputBoard, square, 7)) & (queenBitboard | bishopBitboard);
  attackers |= knightAttacks[square] & (inputBoard.bitboards[4] | inputBoard.bitboards[10]);
  attackers |= pawnAttacks[1][square] & inputBoard.bitboards[5];
  attackers |= pawnAttacks[0][square] & inputBoard.bitboards[11];
  return attackers;
}
bool isInsufficientMaterial(board& inputBoard) {
  if (inputBoard.pieceCounts[5] + inputBoard.pieceCounts[11] != 0) return false;
  if (std::abs(
    9 * (inputBoard.pieceCounts[7] - inputBoard.pieceCounts[1]) + 
    5 * (inputBoard.pieceCounts[8] - inputBoard.pieceCounts[2]) + 
    3 * (inputBoard.pieceCounts[9] + inputBoard.pieceCounts[10] - inputBoard.pieceCounts[3] - inputBoard.pieceCounts[4])
  ) >= 4) {
    return false;
  }
  return true;
}
int getPiece(board& inputBoard, int square) {
  uint64_t squareBit = bits[square];
  int gottenPiece;
  for (int piece = 0; piece < 12; piece++) {
    if ((inputBoard.bitboards[piece] & squareBit) != 0ull) {
      gottenPiece = piece;
      break;
    }
  }
  return gottenPiece;
}
void removePiece(board& inputBoard, int square, bool isUpdatingKey) {
  uint64_t squareBit = bits[square];
  int pieceToRemove = getPiece(inputBoard, square);
  int sideOfPieceToRemove = pieceToRemove < 6 ? 0 : 1;
  int relativePiece = pieceToRemove - 6 * sideOfPieceToRemove;
  inputBoard.bitboards[pieceToRemove] ^= squareBit;
  inputBoard.bitboards[12 + sideOfPieceToRemove] ^= squareBit;
  inputBoard.bitboards[14] ^= squareBit;
  inputBoard.pieceCounts[pieceToRemove]--;
  inputBoard.middleGameEvaluation -= middleGamePieceBonuses[pieceToRemove][square];
  inputBoard.endGameEvaluation -= endGamePieceBonuses[pieceToRemove][square];
  inputBoard.gamePhase -= piecePhases[pieceToRemove];
  if (isUpdatingKey) {
    inputBoard.currentKey ^= positionKeys[pieceToRemove][square];
    inputBoard.currentKey ^= positionKeys[12][square];
  }
}
void addPiece(board& inputBoard, int piece, int square, bool isUpdatingKey) {
  uint64_t squareBit = bits[square];
  int sideOfPieceToAdd = piece < 6 ? 0 : 1;
  int relativePiece = piece - 6 * sideOfPieceToAdd;
  inputBoard.bitboards[piece] |= squareBit;
  inputBoard.bitboards[12 + sideOfPieceToAdd] |= squareBit;
  inputBoard.bitboards[14] |= squareBit;
  inputBoard.pieceCounts[piece]++;
  inputBoard.middleGameEvaluation += middleGamePieceBonuses[piece][square];
  inputBoard.endGameEvaluation += endGamePieceBonuses[piece][square];
  inputBoard.gamePhase += piecePhases[piece];
  if (isUpdatingKey) {
    inputBoard.currentKey ^= positionKeys[12][square];
    inputBoard.currentKey ^= positionKeys[piece][square];
  }
}
void movePiece(board& inputBoard, int from, int to, bool isUpdatingKey) {
  int pieceToMove = getPiece(inputBoard, from);
  int sideOfPieceToMove = pieceToMove < 6 ? 0 : 1;
  int relativePiece = pieceToMove - 6 * sideOfPieceToMove;
  uint64_t fromBit = bits[from];
  uint64_t toBit = bits[to];
  int ourSideBitboardIndex = 12 + sideOfPieceToMove;
  inputBoard.bitboards[pieceToMove] ^= fromBit;
  inputBoard.bitboards[ourSideBitboardIndex] ^= fromBit;
  inputBoard.bitboards[14] ^= fromBit;
  inputBoard.bitboards[pieceToMove] |= toBit;
  inputBoard.bitboards[ourSideBitboardIndex] |= toBit;
  inputBoard.bitboards[14] |= toBit;
  inputBoard.middleGameEvaluation -= middleGamePieceBonuses[pieceToMove][from];
  inputBoard.endGameEvaluation -= endGamePieceBonuses[pieceToMove][from];
  inputBoard.middleGameEvaluation += middleGamePieceBonuses[pieceToMove][to];
  inputBoard.endGameEvaluation += endGamePieceBonuses[pieceToMove][to];
  if (isUpdatingKey) {
    inputBoard.currentKey ^= positionKeys[pieceToMove][from];
    inputBoard.currentKey ^= positionKeys[12][from];
    inputBoard.currentKey ^= positionKeys[12][to];
    inputBoard.currentKey ^= positionKeys[pieceToMove][to];
  }
}
int longAlgebraicToMove(std::string longAlgebraic, board& inputBoard) {
  int move = 0;
  char* lastFilesToCharactersPointer = filesToCharacters + 8;
  char* lastRanksToCharactersPointer = ranksToCharacters + 8;
  int fromX = std::find(filesToCharacters, lastFilesToCharactersPointer, longAlgebraic[0]) - filesToCharacters;
  int fromY = std::find(ranksToCharacters, lastRanksToCharactersPointer, longAlgebraic[1]) - ranksToCharacters;
  int toX = std::find(filesToCharacters, lastFilesToCharactersPointer, longAlgebraic[2]) - filesToCharacters;
  int toY = std::find(ranksToCharacters, lastRanksToCharactersPointer, longAlgebraic[3]) - ranksToCharacters;
  int from = 8 * fromY + fromX;
  int to = 8 * toY + toX;
  move |= from | (to << 6);
  int type = 0; 
  int fromPiece = getPiece(inputBoard, from) % 6;
  if (longAlgebraic.size() == 5) {
    int promotionPiece = std::find(promotionPieces, promotionPieces + 4, longAlgebraic[4]) - promotionPieces;
    move |= promotionPiece << 12;
    type = 1;
  } else if (fromX != toX && (inputBoard.bitboards[14] & bits[to]) == 0ull && fromPiece == 5) {
    type = 2;
  } else if (fromPiece == 0 && std::abs(fromX - toX) > 1) {
    type = 3;
  }
  move |= type << 14;
  return move;
}
std::string moveToLongAlgebraic(int move) {
  int from = move & 0x3f;
  int to = (move & 0xfc0) >> 6;
  int fromX = oneDimensionalToTwoDimensional[from][0];
  int fromY = oneDimensionalToTwoDimensional[from][1];
  int toX = oneDimensionalToTwoDimensional[to][0];
  int toY = oneDimensionalToTwoDimensional[to][1];
  int promotion = (move & 0x3000) >> 12;
  int type = (move & 0xc000) >> 14;
  std::string longAlgebraicMove = "";
  longAlgebraicMove += filesToCharacters[fromX];
  longAlgebraicMove += ranksToCharacters[fromY];
  longAlgebraicMove += filesToCharacters[toX];
  longAlgebraicMove += ranksToCharacters[toY];
  if (type == 1) longAlgebraicMove += promotionPieces[promotion];
  return longAlgebraicMove;
}
void printBoard(board& inputBoard) {
  std::cout << "Game Board: " << std::endl;
  std::cout << std::endl;
  for (int y = 0; y < 8; y++) {
    std::cout << (8 - y) << "  ";
    for (int x = 0; x < 8; x++) {
      int square = 8 * y + x;
      if ((inputBoard.bitboards[14] & bits[square]) == 0ull) {
        std::cout << ". ";
      } else {
        int piece = getPiece(inputBoard, square);
        std::cout << pieces[piece] << " ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  std::cout << "   ";
  for (int x = 0; x < 8; x++) std::cout << char('a' + x) << " ";
  std::cout << std::endl;
  std::cout << "side:" << (inputBoard.sideToPlay == 0 ? 'b' : 'w') << std::endl;
  std::cout << "enPas:" << inputBoard.enPassantSquare << std::endl;
  std::cout << "castle:";
  for (int i = 0; i < 4; i++) {
    if ((inputBoard.castlingPermission & castlingPermissions[i]) != 0) {
      std::cout << castlingStrings[i];
    }
  }
  std::cout << std::endl;
  std::cout << "PosKey:" << inputBoard.currentKey << std::endl;
  std::cout << std::endl;
}
bool isRepetition(board& inputBoard) {
  int numberOfRepetitions = 0;
  for (int i = inputBoard.numberOfHistoryRequiredForNullMove - 4; i >= inputBoard.numberOfHistoryRequiredForNullMove - inputBoard.halfMoveClock; i -= 2) {
    if (inputBoard.historyRequiredForNullMove[i].currentKey == inputBoard.currentKey) {
      numberOfRepetitions++;
    }
  }
  return (inputBoard.distanceToRoot == 1 && numberOfRepetitions == 2) || (inputBoard.distanceToRoot >= 2 && numberOfRepetitions == 1);
}

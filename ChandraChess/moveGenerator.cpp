#include <math.h>
#include <algorithm>
#include <bit>
#include <vector>
#include <iostream>
#include "moveGenerator.h"
#include "bits.h"
#include "state.h"
#include "table.h"
#include "board.h"
#include "staticExchangeEvaluation.h"
int hashMove = 0;
int winningCaptures = 1;
int equalCaptures = 2;
int killerMoves = 3;
int nonCaptures = 4;
int losingCaptures = 5;
void saveMove(int from, int to, int promotionPiece, int type, movesContainer& moves) {
  moves.moveList[moves.numberOfMoves++].move = from | (to << 6) | (promotionPiece << 12) | (type << 14);
}
bool compareScoredMoves(moveEntry& firstScoredMove, moveEntry& secondScoredMove) {
  return firstScoredMove.score > secondScoredMove.score;
}
void orderMoves(board& inputBoard, movesContainer& moves) {
  tableEntry& probedTableEntry = table[inputBoard.currentKey % numberOfEntries];
  for (int i = 0; i < moves.numberOfMoves; i++) {
    moveEntry& currentMove = moves.moveList[i];
    int move = currentMove.move;
    int from = move & 0x3f;
    int to = (move & 0xfc0) >> 6;
    int promotion = (move & 0x3000) >> 12;
    int type = (move & 0xc000) >> 14;
    if (probedTableEntry.key == inputBoard.currentKey && probedTableEntry.move == move) {
      currentMove.score = 2000000;
    } else if (type == 1 || type == 2 || (inputBoard.bitboards[14] & bits[to]) != 0ull) {
      int score = runStaticExchangeEvaluation(inputBoard, move);
      if (score > 0) {
        // Winning capture.
        // If capture score is 200 it will be 1999999.
        // If capture score is 1 it will be 1999800.
        currentMove.score = 1999799 + score;
      } else if (score == 0) {
        // Equal capture.
        currentMove.score = 1999799;
      } else {
        // Losing captures.
        // If capture score is -200 score is 0.
        // If capture score is -1 score is 199.
        currentMove.score = 200 + score;
      }
    } else {
      if (inputBoard.killersTable[inputBoard.distanceToRoot][0] == move || inputBoard.killersTable[inputBoard.distanceToRoot][1] == move || inputBoard.killersTable[inputBoard.distanceToRoot][2] == move) {
        currentMove.score = 1999798;
      } else {
        // If history is 0 score is 200.
        currentMove.score = 200 + inputBoard.historyTable[inputBoard.sideToPlay][from][to];
      }
    }
  }
  std::sort(moves.moveList, moves.moveList + moves.numberOfMoves, compareScoredMoves);
}
void addKingMoves(board& inputBoard, bool isQuiescentGenerator, movesContainer& moves) {
  int theirOffset = 6 * (inputBoard.sideToPlay ^ 1);
  int kingSquare = msbPosition(inputBoard.bitboards[6 * inputBoard.sideToPlay]);
  uint64_t kingSquareBit = bits[kingSquare];
  uint64_t ourBitboard = inputBoard.bitboards[12 + inputBoard.sideToPlay];
  uint64_t ourKingMoves = kingAttacks[kingSquare] & ~ourBitboard;
  uint64_t squaresAttackedByOpponent = 0ull;
  uint64_t theirQueenBitboard = inputBoard.bitboards[theirOffset + 1];
  uint64_t theirRookBitboard = inputBoard.bitboards[theirOffset + 2];
  uint64_t theirBishopBitboard = inputBoard.bitboards[theirOffset + 3];
  uint64_t theirKnightBitboard = inputBoard.bitboards[theirOffset + 4];
  uint64_t theirPawnBitboard = inputBoard.bitboards[theirOffset + 5];
  int theirKingSquare = msbPosition(inputBoard.bitboards[theirOffset]);
  inputBoard.bitboards[14] ^= kingSquareBit;
  squaresAttackedByOpponent |= kingAttacks[theirKingSquare];
  while (theirQueenBitboard != 0ull) {
    int attackerSquare = lsbPosition(theirQueenBitboard);
    squaresAttackedByOpponent |= msbRayAttacks(inputBoard, attackerSquare, 0) | msbRayAttacks(inputBoard, attackerSquare, 1) | msbRayAttacks(inputBoard, attackerSquare, 2) | msbRayAttacks(inputBoard, attackerSquare, 3) | lsbRayAttacks(inputBoard, attackerSquare, 4) | lsbRayAttacks(inputBoard, attackerSquare, 5) | lsbRayAttacks(inputBoard, attackerSquare, 6) | lsbRayAttacks(inputBoard, attackerSquare, 7);
    theirQueenBitboard &= theirQueenBitboard - 1ull;
  }
  while (theirRookBitboard != 0ull) {
    int attackerSquare = lsbPosition(theirRookBitboard);
    squaresAttackedByOpponent |= msbRayAttacks(inputBoard, attackerSquare, 0) | msbRayAttacks(inputBoard, attackerSquare, 2) | lsbRayAttacks(inputBoard, attackerSquare, 4) | lsbRayAttacks(inputBoard, attackerSquare, 6);
    theirRookBitboard &= theirRookBitboard - 1ull;
  }
  while (theirBishopBitboard != 0ull) {
    int attackerSquare = lsbPosition(theirBishopBitboard);
    squaresAttackedByOpponent |= msbRayAttacks(inputBoard, attackerSquare, 1) | msbRayAttacks(inputBoard, attackerSquare, 3) | lsbRayAttacks(inputBoard, attackerSquare, 5) | lsbRayAttacks(inputBoard, attackerSquare, 7);
    theirBishopBitboard &= theirBishopBitboard - 1ull;
  }
  while (theirKnightBitboard != 0ull) {
    int attackerSquare = lsbPosition(theirKnightBitboard);
    squaresAttackedByOpponent |= knightAttacks[attackerSquare];
    theirKnightBitboard &= theirKnightBitboard - 1ull;
  }
  if (inputBoard.sideToPlay == 1) {
    squaresAttackedByOpponent |= ((theirPawnBitboard & ~files[0]) >> 7ull);
    squaresAttackedByOpponent |= ((theirPawnBitboard & ~files[7]) >> 9ull);
  } else {
    squaresAttackedByOpponent |= ((theirPawnBitboard & ~files[7]) << 7ull);
    squaresAttackedByOpponent |= ((theirPawnBitboard & ~files[0]) << 9ull);
  }
  inputBoard.bitboards[14] ^= kingSquareBit;
  ourKingMoves &= ~squaresAttackedByOpponent;
  if (isQuiescentGenerator) ourKingMoves &= inputBoard.bitboards[14];
  while (ourKingMoves != 0ull) {
    int to = lsbPosition(ourKingMoves);
    saveMove(kingSquare, to, 0, 0, moves);
    ourKingMoves &= ourKingMoves - 1ull;
  }
}
void addQueenMoves(board& inputBoard, bool isQuiescentGenerator, uint64_t pinnedOnHorizontal, uint64_t pinnedOnVertical, uint64_t pinnedOnPositiveDiagonal, uint64_t pinnedOnNegativeDiagonal, uint64_t notPinned, uint64_t pseudolegalMask, movesContainer& moves) {
  uint64_t ourQueenBitboard = inputBoard.bitboards[6 * inputBoard.sideToPlay + 1];
  uint64_t horizontallyPinnedQueens = pinnedOnHorizontal & ourQueenBitboard;
  uint64_t verticallyPinnedQueens = pinnedOnVertical & ourQueenBitboard;
  uint64_t positiveDiagonallyPinnedQueens = pinnedOnPositiveDiagonal & ourQueenBitboard;
  uint64_t negativeDiagonallyPinnedQueens = pinnedOnNegativeDiagonal & ourQueenBitboard;
  uint64_t unPinnedQueens = notPinned & ourQueenBitboard;
  while (horizontallyPinnedQueens != 0ull) {
    int from = lsbPosition(horizontallyPinnedQueens);
    uint64_t ourQueenMoves = (msbRayAttacks(inputBoard, from, 0) | lsbRayAttacks(inputBoard, from, 4)) & pseudolegalMask;
    if (isQuiescentGenerator) ourQueenMoves &= inputBoard.bitboards[14];
    while (ourQueenMoves != 0ull) {
      int to = lsbPosition(ourQueenMoves);
      saveMove(from, to, 0, 0, moves);
      ourQueenMoves &= ourQueenMoves - 1ull;
    }
    horizontallyPinnedQueens &= horizontallyPinnedQueens - 1ull;
  }
  while (verticallyPinnedQueens != 0ull) {
    int from = lsbPosition(verticallyPinnedQueens);
    uint64_t ourQueenMoves = (msbRayAttacks(inputBoard, from, 2) | lsbRayAttacks(inputBoard, from, 6)) & pseudolegalMask; 
    if (isQuiescentGenerator) ourQueenMoves &= inputBoard.bitboards[14];   
    while (ourQueenMoves != 0ull) {
      int to = lsbPosition(ourQueenMoves);
      saveMove(from, to, 0, 0, moves);
      ourQueenMoves &= ourQueenMoves - 1ull;
    }
    verticallyPinnedQueens &= verticallyPinnedQueens - 1ull;
  }
  while (positiveDiagonallyPinnedQueens != 0ull) {
    int from = lsbPosition(positiveDiagonallyPinnedQueens);
    uint64_t ourQueenMoves = (msbRayAttacks(inputBoard, from, 3) | lsbRayAttacks(inputBoard, from, 7)) & pseudolegalMask;
    if (isQuiescentGenerator) ourQueenMoves &= inputBoard.bitboards[14];   
    while (ourQueenMoves != 0ull) {
      int to = lsbPosition(ourQueenMoves);
      saveMove(from, to, 0, 0, moves);
      ourQueenMoves &= ourQueenMoves - 1ull;
    }
    positiveDiagonallyPinnedQueens &= positiveDiagonallyPinnedQueens - 1ull;
  }
  while (negativeDiagonallyPinnedQueens != 0ull) {
    int from = lsbPosition(negativeDiagonallyPinnedQueens);
    uint64_t ourQueenMoves = (msbRayAttacks(inputBoard, from, 1) | lsbRayAttacks(inputBoard, from, 5)) & pseudolegalMask;
    if (isQuiescentGenerator) ourQueenMoves &= inputBoard.bitboards[14];   
    while (ourQueenMoves != 0ull) {
      int to = lsbPosition(ourQueenMoves);
      saveMove(from, to, 0, 0, moves);
      ourQueenMoves &= ourQueenMoves - 1ull;
    }
    negativeDiagonallyPinnedQueens &= negativeDiagonallyPinnedQueens - 1ull;
  }
  while (unPinnedQueens != 0ull) {
    int from = lsbPosition(unPinnedQueens);
    uint64_t ourQueenMoves = (msbRayAttacks(inputBoard, from, 0) | msbRayAttacks(inputBoard, from, 1) | msbRayAttacks(inputBoard, from, 2) | msbRayAttacks(inputBoard, from, 3) | lsbRayAttacks(inputBoard, from, 4) | lsbRayAttacks(inputBoard, from, 5) | lsbRayAttacks(inputBoard, from, 6) | lsbRayAttacks(inputBoard, from, 7)) & pseudolegalMask;
    if (isQuiescentGenerator) ourQueenMoves &= inputBoard.bitboards[14];   
    while (ourQueenMoves != 0ull) {
      int to = lsbPosition(ourQueenMoves);
      saveMove(from, to, 0, 0, moves);
      ourQueenMoves &= ourQueenMoves - 1ull;
    }
    unPinnedQueens &= unPinnedQueens - 1ull;
  }
}
void addRookMoves(board& inputBoard, bool isQuiescentGenerator, uint64_t pinnedOnHorizontal, uint64_t pinnedOnVertical, uint64_t notPinned, uint64_t pseudolegalMask, movesContainer& moves) {
  uint64_t ourRookBitboard = inputBoard.bitboards[6 * inputBoard.sideToPlay + 2];
  uint64_t horizontallyPinnedRooks = pinnedOnHorizontal & ourRookBitboard;
  uint64_t verticallyPinnedRooks = pinnedOnVertical & ourRookBitboard;
  uint64_t unPinnedRooks = notPinned & ourRookBitboard;
  while (horizontallyPinnedRooks != 0ull) {
    int from = lsbPosition(horizontallyPinnedRooks);
    uint64_t ourRookMoves = (msbRayAttacks(inputBoard, from, 0) | lsbRayAttacks(inputBoard, from, 4)) & pseudolegalMask;
    if (isQuiescentGenerator) ourRookMoves &= inputBoard.bitboards[14];
    while (ourRookMoves != 0ull) {
      int to = lsbPosition(ourRookMoves);
      saveMove(from, to, 0, 0, moves);
      ourRookMoves &= ourRookMoves - 1ull;
    }
    horizontallyPinnedRooks &= horizontallyPinnedRooks - 1ull;
  }
  while (verticallyPinnedRooks != 0ull) {
    int from = lsbPosition(verticallyPinnedRooks);
    uint64_t ourRookMoves = (msbRayAttacks(inputBoard, from, 2) | lsbRayAttacks(inputBoard, from, 6)) & pseudolegalMask;
    if (isQuiescentGenerator) ourRookMoves &= inputBoard.bitboards[14];
    while (ourRookMoves != 0ull) {
      int to = lsbPosition(ourRookMoves);
      saveMove(from, to, 0, 0, moves);
      ourRookMoves &= ourRookMoves - 1ull;
    }
    verticallyPinnedRooks &= verticallyPinnedRooks - 1ull;
  }
  while (unPinnedRooks != 0ull) {
    int from = lsbPosition(unPinnedRooks);
    uint64_t ourRookMoves = (msbRayAttacks(inputBoard, from, 0) | msbRayAttacks(inputBoard, from, 2) | lsbRayAttacks(inputBoard, from, 4) | lsbRayAttacks(inputBoard, from, 6)) & pseudolegalMask;
    if (isQuiescentGenerator) ourRookMoves &= inputBoard.bitboards[14];
    while (ourRookMoves != 0ull) {
      int to = lsbPosition(ourRookMoves);
      saveMove(from, to, 0, 0, moves);
      ourRookMoves &= ourRookMoves - 1ull;
    }
    unPinnedRooks &= unPinnedRooks - 1ull;
  }
}
void addBishopMoves(board& inputBoard, bool isQuiescentGenerator, uint64_t pinnedOnPositiveDiagonal, uint64_t pinnedOnNegativeDiagonal, uint64_t notPinned, uint64_t pseudolegalMask, movesContainer& moves) {
  uint64_t ourBishopBitboard = inputBoard.bitboards[6 * inputBoard.sideToPlay + 3];
  uint64_t positiveDiagonallyPinnedBishops = pinnedOnPositiveDiagonal & ourBishopBitboard;
  uint64_t negativeDiagonallyPinnedBishops = pinnedOnNegativeDiagonal & ourBishopBitboard;
  uint64_t unPinnedBishops = notPinned & ourBishopBitboard;
  while (positiveDiagonallyPinnedBishops != 0ull) {
    int from = lsbPosition(positiveDiagonallyPinnedBishops);
    uint64_t ourBishopMoves = (msbRayAttacks(inputBoard, from, 3) | lsbRayAttacks(inputBoard, from, 7)) & pseudolegalMask;
    if (isQuiescentGenerator) ourBishopMoves &= inputBoard.bitboards[14];
    while (ourBishopMoves != 0ull) {
      int to = lsbPosition(ourBishopMoves);
      saveMove(from, to, 0, 0, moves);
      ourBishopMoves &= ourBishopMoves - 1ull;
    }
    positiveDiagonallyPinnedBishops &= positiveDiagonallyPinnedBishops - 1ull;
  }
  while (negativeDiagonallyPinnedBishops != 0ull) {
    int from = lsbPosition(negativeDiagonallyPinnedBishops);
    uint64_t ourBishopMoves = (msbRayAttacks(inputBoard, from, 1) | lsbRayAttacks(inputBoard, from, 5)) & pseudolegalMask;
    if (isQuiescentGenerator) ourBishopMoves &= inputBoard.bitboards[14];
    while (ourBishopMoves != 0ull) {
      int to = lsbPosition(ourBishopMoves);
      saveMove(from, to, 0, 0, moves);
      ourBishopMoves &= ourBishopMoves - 1ull;
    }
    negativeDiagonallyPinnedBishops &= negativeDiagonallyPinnedBishops - 1ull;
  }
  while (unPinnedBishops != 0ull) {
    int from = lsbPosition(unPinnedBishops);
    uint64_t ourBishopMoves = (msbRayAttacks(inputBoard, from, 1) | msbRayAttacks(inputBoard, from, 3) | lsbRayAttacks(inputBoard, from, 5) | lsbRayAttacks(inputBoard, from, 7)) & pseudolegalMask;
    if (isQuiescentGenerator) ourBishopMoves &= inputBoard.bitboards[14];
    while (ourBishopMoves != 0ull) {
      int to = lsbPosition(ourBishopMoves);
      saveMove(from, to, 0, 0, moves);
      ourBishopMoves &= ourBishopMoves - 1ull;
    }
    unPinnedBishops &= unPinnedBishops - 1ull;
  }
}
void addKnightMoves(board& inputBoard, bool isQuiescentGenerator, uint64_t notPinned, uint64_t pseudolegalMask, movesContainer& moves) {
  uint64_t unPinnedKnights = notPinned & inputBoard.bitboards[6 * inputBoard.sideToPlay + 4];
  while (unPinnedKnights != 0ull) {
    int from = lsbPosition(unPinnedKnights);
    uint64_t ourKnightMoves = knightAttacks[from] & pseudolegalMask;
    if (isQuiescentGenerator) ourKnightMoves &= inputBoard.bitboards[14];
    while (ourKnightMoves != 0ull) {
      int to = lsbPosition(ourKnightMoves);
      saveMove(from, to, 0, 0, moves);
      ourKnightMoves &= ourKnightMoves - 1ull;
    }
    unPinnedKnights &= unPinnedKnights - 1ull;
  }
}
void addPawnMoves(board& inputBoard, bool isQuiescentGenerator, uint64_t pinnedOnVertical, uint64_t pinnedOnPositiveDiagonal, uint64_t pinnedOnNegativeDiagonal, uint64_t notPinned, uint64_t pseudolegalMask, movesContainer& moves) {
  uint64_t theirBitboard = inputBoard.bitboards[12 + inputBoard.sideToPlay ^ 1];
  uint64_t ourPawnBitboard = inputBoard.bitboards[6 * inputBoard.sideToPlay + 5];
  uint64_t emptySquares = ~inputBoard.bitboards[14];
  uint64_t pawnPushMask = ~theirBitboard & pseudolegalMask;
  uint64_t pawnCaptureMask = theirBitboard & pseudolegalMask;
  uint64_t verticallyPinnedPawns = pinnedOnVertical & ourPawnBitboard;
  uint64_t positiveDiagonallyPinnedPawns = pinnedOnPositiveDiagonal & ourPawnBitboard;
  uint64_t negativeDiagonallyPinnedPawns = pinnedOnNegativeDiagonal & ourPawnBitboard;
  uint64_t unPinnedPawns = notPinned & ourPawnBitboard; 
  uint64_t notLeftFile = ~files[0];
  uint64_t notRightFile = ~files[7];
  uint64_t singlePushes;
  uint64_t doublePushes;
  uint64_t positiveDiagonalCaptures;
  uint64_t negativeDiagonalCaptures;
  uint64_t verticallyPushablePawns = verticallyPinnedPawns | unPinnedPawns;
  int promotionY;
  int sign;
  if (inputBoard.sideToPlay == 0) {
    singlePushes = (verticallyPushablePawns >> 8ull) & pawnPushMask;
    doublePushes = ((((verticallyPushablePawns & ranks[1]) >> 8ull) & emptySquares) >> 8ull) & pawnPushMask;
    positiveDiagonalCaptures = (((positiveDiagonallyPinnedPawns | unPinnedPawns) & notLeftFile) >> 7ull) & pawnCaptureMask;
    negativeDiagonalCaptures = (((negativeDiagonallyPinnedPawns | unPinnedPawns) & notRightFile) >> 9ull) & pawnCaptureMask;
    promotionY = 7;
    sign = -1;
  } else {
    singlePushes = (verticallyPushablePawns << 8ull) & pawnPushMask;
    doublePushes = ((((verticallyPushablePawns & ranks[6]) << 8ull) & emptySquares) << 8ull) & pawnPushMask;
    positiveDiagonalCaptures = (((positiveDiagonallyPinnedPawns | unPinnedPawns) & notRightFile) << 7ull) & pawnCaptureMask;
    negativeDiagonalCaptures = (((negativeDiagonallyPinnedPawns | unPinnedPawns) & notLeftFile) << 9ull) & pawnCaptureMask;
    promotionY = 0;
    sign = 1;
  }
  uint64_t promotionRank = ranks[promotionY];
  uint64_t notPromotionRank = ~promotionRank;
  uint64_t singlePushesNonPromotion = singlePushes & notPromotionRank;
  uint64_t singlePushesPromotion = singlePushes & promotionRank;
  if (!isQuiescentGenerator) {
    while (doublePushes != 0ull) {
      int to = lsbPosition(doublePushes);
      int from = to + sign * 16;
      saveMove(from, to, 0, 0, moves);
      doublePushes &= doublePushes - 1ull;
    }
    while (singlePushesNonPromotion != 0ull) {
      int to = lsbPosition(singlePushesNonPromotion);
      int from = to + sign * 8;
      saveMove(from, to, 0, 0, moves);
      singlePushesNonPromotion &= singlePushesNonPromotion - 1ull;
    }
  }
  while (singlePushesPromotion != 0ull) {
    int to = lsbPosition(singlePushesPromotion);
    int from = to + sign * 8;
    saveMove(from, to, 0, 1, moves);
    saveMove(from, to, 1, 1, moves);
    saveMove(from, to, 2, 1, moves);
    saveMove(from, to, 3, 1, moves);
    singlePushesPromotion &= singlePushesPromotion - 1ull;
  }
  while (positiveDiagonalCaptures != 0ull) {
    int to = lsbPosition(positiveDiagonalCaptures);
    int toY = oneDimensionalToTwoDimensional[to][1];
    int from = to + sign * 7;
    if (toY == promotionY) {
      saveMove(from, to, 0, 1, moves);
      saveMove(from, to, 1, 1, moves);
      saveMove(from, to, 2, 1, moves);
      saveMove(from, to, 3, 1, moves);
    } else {
      saveMove(from, to, 0, 0, moves);
    }
    positiveDiagonalCaptures &= positiveDiagonalCaptures - 1ull;
  }
  while (negativeDiagonalCaptures != 0ull) {
    int to = lsbPosition(negativeDiagonalCaptures);
    int toY = oneDimensionalToTwoDimensional[to][1];
    int from = to + sign * 9;
    if (toY == promotionY) {
      saveMove(from, to, 0, 1, moves);
      saveMove(from, to, 1, 1, moves);
      saveMove(from, to, 2, 1, moves);
      saveMove(from, to, 3, 1, moves);
    } else {
      saveMove(from, to, 0, 0, moves);
    }
    negativeDiagonalCaptures &= negativeDiagonalCaptures - 1ull;
  }
}
void addEnPassantMoves(board& inputBoard, movesContainer& moves) {
  int enPassantX = oneDimensionalToTwoDimensional[inputBoard.enPassantSquare][0];
  int ourOffset = 6 * inputBoard.sideToPlay;
  int kingSquare = msbPosition(inputBoard.bitboards[ourOffset]);
  uint64_t ourPawnBitboard = inputBoard.bitboards[ourOffset + 5];
  int enPassantSquareToTheirPawnSquare = inputBoard.sideToPlay == 0 ? -8 : 8;
  int theirPawnSquare = inputBoard.enPassantSquare + enPassantSquareToTheirPawnSquare;
  int theirPawnIndex = 6 * (inputBoard.sideToPlay ^ 1) + 5;
  if (enPassantX > 0) {
    int from = theirPawnSquare - 1;
    if ((bits[from] & ourPawnBitboard) != 0ull) {
      inputBoard.bitboards[theirPawnIndex] ^= bits[theirPawnSquare];
      inputBoard.bitboards[14] ^= bits[theirPawnSquare];
      inputBoard.bitboards[14] ^= bits[from];
      inputBoard.bitboards[14] |= bits[inputBoard.enPassantSquare];
      bool isInCheck = isSquareOfSideToPlayAttacked(inputBoard, kingSquare);
      inputBoard.bitboards[theirPawnIndex] |= bits[theirPawnSquare];
      inputBoard.bitboards[14] |= bits[theirPawnSquare];
      inputBoard.bitboards[14] |= bits[from];
      inputBoard.bitboards[14] ^= bits[inputBoard.enPassantSquare];
      if (!isInCheck) saveMove(from, inputBoard.enPassantSquare, 0, 2, moves);
    }
  }
  if (enPassantX < 7) {
    int from = theirPawnSquare + 1;
    if ((bits[from] & ourPawnBitboard) != 0ull) {
      inputBoard.bitboards[theirPawnIndex] ^= bits[theirPawnSquare];
      inputBoard.bitboards[14] ^= bits[theirPawnSquare];
      inputBoard.bitboards[14] ^= bits[from];
      inputBoard.bitboards[14] |= bits[inputBoard.enPassantSquare];
      bool isInCheck = isSquareOfSideToPlayAttacked(inputBoard, kingSquare);
      inputBoard.bitboards[theirPawnIndex] |= bits[theirPawnSquare];
      inputBoard.bitboards[14] |= bits[theirPawnSquare];
      inputBoard.bitboards[14] |= bits[from];
      inputBoard.bitboards[14] ^= bits[inputBoard.enPassantSquare];
      if (!isInCheck) saveMove(from, inputBoard.enPassantSquare, 0, 2, moves);
    }
  }
}
void addCastlingMoves(board& inputBoard, movesContainer& moves) {
  int castlingPermissionsOffset = 2 * inputBoard.sideToPlay;
  int kingSquare = inputBoard.sideToPlay == 0 ? 4 : 60;
  if ((inputBoard.castlingPermission & castlingPermissions[castlingPermissionsOffset]) != 0) {
    if ((inputBoard.bitboards[14] & bits[kingSquare + 1]) == 0ull && (inputBoard.bitboards[14] & bits[kingSquare + 2]) == 0ull) {
      if (!isSquareOfSideToPlayAttacked(inputBoard, kingSquare + 1) && !isSquareOfSideToPlayAttacked(inputBoard, kingSquare + 2)) saveMove(kingSquare, kingSquare + 2, 0, 3, moves);
    }
  }
  if ((inputBoard.castlingPermission & castlingPermissions[castlingPermissionsOffset + 1]) != 0) {
    if ((inputBoard.bitboards[14] & bits[kingSquare - 1]) == 0ull && (inputBoard.bitboards[14] & bits[kingSquare - 2]) == 0ull && (inputBoard.bitboards[14] & bits[kingSquare - 3]) == 0ull) {
      if (!isSquareOfSideToPlayAttacked(inputBoard, kingSquare - 1) && !isSquareOfSideToPlayAttacked(inputBoard, kingSquare - 2)) saveMove(kingSquare, kingSquare - 2, 0, 3, moves);
    }
  }
}
void generateMoves(board& inputBoard, movesContainer& moves, bool isQuiescentGenerator) {
  int theirSide = inputBoard.sideToPlay ^ 1;
  int theirOffset = 6 * theirSide;
  int kingSquare = msbPosition(inputBoard.bitboards[6 * inputBoard.sideToPlay]);
  uint64_t theirQueenBitboard = inputBoard.bitboards[theirOffset + 1];
  uint64_t theirRookBitboard = inputBoard.bitboards[theirOffset + 2];
  uint64_t theirBishopBitboard = inputBoard.bitboards[theirOffset + 3];
  uint64_t checkers = piecesAttackingSquareOfSideToPlay(inputBoard, kingSquare);
  uint64_t slidingCheckers = checkers & (theirQueenBitboard | theirRookBitboard | theirBishopBitboard);
  addKingMoves(inputBoard, isQuiescentGenerator, moves);
  int numberOfOurCheckers = std::popcount(checkers);
  if (numberOfOurCheckers == 2) return;
  uint64_t kingEastRay = msbRayAttacks(inputBoard, kingSquare, 0);
  uint64_t kingSouthEastRay = msbRayAttacks(inputBoard, kingSquare, 1);
  uint64_t kingSouthRay = msbRayAttacks(inputBoard, kingSquare, 2);
  uint64_t kingSouthWestRay = msbRayAttacks(inputBoard, kingSquare, 3);
  uint64_t kingWestRay = lsbRayAttacks(inputBoard, kingSquare, 4);
  uint64_t kingNorthWestRay = lsbRayAttacks(inputBoard, kingSquare, 5);
  uint64_t kingNorthRay = lsbRayAttacks(inputBoard, kingSquare, 6);
  uint64_t kingNorthEastRay = lsbRayAttacks(inputBoard, kingSquare, 7);
  uint64_t kingRays[8] = {kingEastRay, kingSouthEastRay, kingSouthRay, kingSouthWestRay, kingWestRay, kingNorthWestRay, kingNorthRay, kingNorthEastRay};
  uint64_t pushMask = 0xffffffffffffffffull;
  uint64_t captureMask = 0xffffffffffffffffull;
  uint64_t pseudolegalMask = ~inputBoard.bitboards[12 + inputBoard.sideToPlay];
  if (numberOfOurCheckers == 1) {
    if (slidingCheckers != 0ull) {
      int slidingCheckerSquare = msbPosition(slidingCheckers);
      int rayFromKing = fromToToRays[kingSquare][slidingCheckerSquare];
      int oppositeRay = oppositeRays[rayFromKing];
      pushMask = kingRays[rayFromKing] & generalRayAttacks(inputBoard, slidingCheckerSquare, oppositeRay);
    } else {
      pushMask = 0ull;
    }
    captureMask = checkers;
    pseudolegalMask &= pushMask | captureMask;
  }
  uint64_t pinnedPieces[4] = {0ull, 0ull, 0ull, 0ull};
  while (theirQueenBitboard != 0ull) {
    int from = lsbPosition(theirQueenBitboard);
    int rayFromKing = fromToToRays[kingSquare][from];
    if (rayFromKing != 8) {
      int rayType = rayFromKing & 3;
      int oppositeRay = oppositeRays[rayFromKing];
      pinnedPieces[rayType] |= kingRays[rayFromKing] & generalRayAttacks(inputBoard, from, oppositeRay);
    }
    theirQueenBitboard &= theirQueenBitboard - 1ull;
  }
  while (theirRookBitboard != 0ull) {
    int from = lsbPosition(theirRookBitboard);
    int rayFromKing = fromToToRays[kingSquare][from];
    int rayType = rayFromKing & 3;
    if (rayFromKing != 8 && (rayType & 1) == 0) {
      int oppositeRay = oppositeRays[rayFromKing];
      pinnedPieces[rayType] |= kingRays[rayFromKing] & generalRayAttacks(inputBoard, from, oppositeRay);
    }
    theirRookBitboard &= theirRookBitboard - 1ull;
  }
  while (theirBishopBitboard != 0ull) {
    int from = lsbPosition(theirBishopBitboard);
    int rayFromKing = fromToToRays[kingSquare][from];
    int rayType = rayFromKing & 3;
    if (rayFromKing != 8 && (rayType & 1) == 1) {
      int oppositeRay = oppositeRays[rayFromKing];
      pinnedPieces[rayType] |= kingRays[rayFromKing] & generalRayAttacks(inputBoard, from, oppositeRay);
    }
    theirBishopBitboard &= theirBishopBitboard - 1ull;
  }
  uint64_t pinnedOnHorizontal = pinnedPieces[0];
  uint64_t pinnedOnNegativeDiagonal = pinnedPieces[1];
  uint64_t pinnedOnVertical = pinnedPieces[2];
  uint64_t pinnedOnPositiveDiagonal = pinnedPieces[3];
  uint64_t notPinned = ~(pinnedOnHorizontal | pinnedOnNegativeDiagonal | pinnedOnVertical | pinnedOnPositiveDiagonal);
  addQueenMoves(inputBoard, isQuiescentGenerator, pinnedOnHorizontal, pinnedOnVertical, pinnedOnPositiveDiagonal, pinnedOnNegativeDiagonal, notPinned, pseudolegalMask, moves);
  addRookMoves(inputBoard, isQuiescentGenerator, pinnedOnHorizontal, pinnedOnVertical, notPinned, pseudolegalMask, moves);
  addBishopMoves(inputBoard, isQuiescentGenerator, pinnedOnPositiveDiagonal, pinnedOnNegativeDiagonal, notPinned, pseudolegalMask, moves);
  addKnightMoves(inputBoard, isQuiescentGenerator, notPinned, pseudolegalMask, moves);
  addPawnMoves(inputBoard, isQuiescentGenerator, pinnedOnVertical, pinnedOnPositiveDiagonal, pinnedOnNegativeDiagonal, notPinned, pseudolegalMask, moves);
  if (inputBoard.enPassantSquare != 0) addEnPassantMoves(inputBoard, moves);
  if (numberOfOurCheckers == 0 && !isQuiescentGenerator) addCastlingMoves(inputBoard, moves);
}

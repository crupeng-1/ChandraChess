#pragma once
#include <bit>
#include "state.h"
#include "board.h"
extern int middleGamePieceValues[6];
extern int endGamePieceValues[6];
extern int middleGamePieceSquareTables[6][64];
extern int endGamePieceSquareTables[6][64];
extern int middleGamePieceBonuses[12][64];
extern int endGamePieceBonuses[12][64];
extern int piecePhases[12];
extern int passedPawnBonuses[8];
extern int isolatedPawnPenalty;
extern int backwardPawnPenalty;
extern int farFromOpponentKing[7];
extern int farFromOurKing[7];
extern int bishopPairBonus;
extern int attackerWeightsByPiece[4];
extern int attackerWeightsByNumber[8];
extern int* mobilityBonuses[2][4];
extern int rayStarts[3];
extern int rayIncrements[3];
void initializeEvaluation();
template <typename boardType> void setStartingEvaluationValues(boardType& inputBoard) {
  inputBoard.middleGameEvaluation = 0;
  inputBoard.endGameEvaluation = 0;
  inputBoard.gamePhase = 0;
  for (int piece = 0; piece < 12; piece++) {
    uint64_t pieceBitboard = inputBoard.bitboards[piece];
    while (pieceBitboard != 0ull) {
      int from = lsbPosition(pieceBitboard);
      int pieceSide = piece < 6 ? 0 : 1;
      inputBoard.middleGameEvaluation += middleGamePieceBonuses[piece][from];
      inputBoard.endGameEvaluation += endGamePieceBonuses[piece][from];
      pieceBitboard &= pieceBitboard - 1ull;
    }
    inputBoard.gamePhase += piecePhases[piece] * inputBoard.pieceCounts[piece];
  }
}
template <typename boardType> int evaluate(boardType& inputBoard) {
  if (isInsufficientMaterial(inputBoard)) return 0;
  int score = 0;
  int middleGamePhase = std::min(inputBoard.gamePhase, 24);
  int middleGameScore = inputBoard.middleGameEvaluation;
  int endGameScore = inputBoard.endGameEvaluation;
  uint64_t notOccupied = ~inputBoard.bitboards[14];
  uint64_t notLeftFile = ~files[0];
  uint64_t notRightFile = ~files[7];
  uint64_t notAttackedByPawns[2] = {
    ~(((inputBoard.bitboards[5] & notLeftFile) >> 7ull) | ((inputBoard.bitboards[5] & notRightFile) >> 9ull)),
    ~(((inputBoard.bitboards[11] & notRightFile) << 7ull) | ((inputBoard.bitboards[11] & notLeftFile) << 9ull))
  };
  for (int side = 0; side < 2; side++) {
    int theirSide = side ^ 1;
    int ourKingSquare = msbPosition(inputBoard.bitboards[6 * side]);
    int theirKingSquare = msbPosition(inputBoard.bitboards[6 * theirSide]);
    uint64_t kingZone = kingAttacks[ourKingSquare];
    uint64_t extendedKingZone = extendedKingAttacks[ourKingSquare];
    int kingAttacksValue = 0;
    int numberOfAttackers = 0;
    int bonusSign = 2 * side - 1;
    int ourOffset = 6 * side;
    int theirOffset = 6 * theirSide;
    int ourPawnBitboardIndex = ourOffset + 5;
    int theirPawnBitboardIndex = theirOffset + 5;
    int sideScore = 0;
    int sideMiddleGameScore = 0;
    int sideEndGameScore = 0;
    uint64_t ourPawnBitboard = inputBoard.bitboards[ourPawnBitboardIndex];
    // Bishop pair bonus.
    if (inputBoard.pieceCounts[ourOffset + 3] == 2) sideScore += bishopPairBonus;
    // Evaluate pawn structure.
    while (ourPawnBitboard) {
      int from = lsbPosition(ourPawnBitboard);
      int forwardSquare = from - bonusSign * 8;
      int file = oneDimensionalToTwoDimensional[from][0];
      int rank = oneDimensionalToTwoDimensional[from][1];
      if ((inputBoard.bitboards[ourPawnBitboardIndex] & isolatedPawnMasks[file]) == 0ull) sideScore -= isolatedPawnPenalty;
      if ((inputBoard.bitboards[theirPawnBitboardIndex] & passedPawnMasks[side][from]) == 0ull && (inputBoard.bitboards[ourPawnBitboardIndex] & forwardMasks[side][from]) == 0ull) {
        int passedPawnRank = side == 0 ? 7 - rank : rank;
        sideEndGameScore += farFromOpponentKing[kingDistanceBetween[forwardSquare][theirKingSquare]]; 
        sideEndGameScore -= farFromOurKing[kingDistanceBetween[forwardSquare][ourKingSquare]]; 
        sideScore += passedPawnBonuses[passedPawnRank];
      }
      if ((inputBoard.bitboards[ourPawnBitboardIndex] & pawnBehindMasks[side][from]) == 0ull && (pawnAttacks[side][forwardSquare] & inputBoard.bitboards[theirPawnBitboardIndex]) != 0ull) sideScore -= backwardPawnPenalty;
      ourPawnBitboard &= ourPawnBitboard - 1ull;
    }
    // Evaluate king safety penalty for us and mobility for opponent.
    for (int piece = 0; piece < 3; piece++) {
      uint64_t pieces = inputBoard.bitboards[theirOffset + piece + 1];
      while (pieces != 0ull) {
        int pieceSquare = lsbPosition(pieces);
        uint64_t pieceAttacks = 0ull;
        uint64_t attacksOnKingZone = 0ull;
        for (int ray = rayStarts[piece]; ray < 8; ray += rayIncrements[piece]) {
          pieceAttacks |= generalRayAttacks(inputBoard, pieceSquare, ray);
          attacksOnKingZone |= rayAttacks[ray][pieceSquare];
        }
        pieceAttacks &= notOccupied;
        int numberOfValidPieceAttacks = std::popcount(pieceAttacks & notAttackedByPawns[side]);
        sideMiddleGameScore -= mobilityBonuses[0][piece][numberOfValidPieceAttacks];
        sideEndGameScore -= mobilityBonuses[1][piece][numberOfValidPieceAttacks];
        uint64_t attacksOnExtendedKingZone = pieceAttacks & extendedKingZone;
        attacksOnKingZone &= kingZone;
        if (attacksOnExtendedKingZone != 0ull && attacksOnKingZone != 0ull) {
          kingAttacksValue += attackerWeightsByPiece[piece];
          numberOfAttackers++;
        }
        pieces &= pieces - 1ull;
      }  
    }
    uint64_t knights = inputBoard.bitboards[theirOffset + 4];
    while (knights != 0ull) {
      int knightSquare = lsbPosition(knights);
      uint64_t knightPieceAttacks = knightAttacks[knightSquare];
      uint64_t knightAttacksOnKing = kingZone & knightPieceAttacks;
      int numberOfValidPieceAttacks = std::popcount(knightPieceAttacks & notOccupied & notAttackedByPawns[side]);
      sideMiddleGameScore -= mobilityBonuses[0][3][numberOfValidPieceAttacks];
      sideEndGameScore -= mobilityBonuses[1][3][numberOfValidPieceAttacks];
      if (knightAttacksOnKing != 0ull) {
        kingAttacksValue += attackerWeightsByPiece[3];
        numberOfAttackers++;
      }
      knights &= knights - 1ull;
    }
    sideMiddleGameScore -= kingAttacksValue * attackerWeightsByNumber[numberOfAttackers] / 100;
    score += bonusSign * sideScore;
    middleGameScore += bonusSign * sideMiddleGameScore;
    endGameScore += bonusSign * sideEndGameScore;
  }
  score += (middleGameScore * middleGamePhase + endGameScore * (24 - middleGamePhase)) / 24;
  return (2 * inputBoard.sideToPlay - 1) * score;
}

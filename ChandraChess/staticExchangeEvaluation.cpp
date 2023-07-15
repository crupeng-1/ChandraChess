#include <csignal>
#include <iostream>
#include "bits.h"
#include "board.h"
#include "state.h"
#include "staticExchangeEvaluation.h"
// Does it in perspective of sideToPlay.
// Assumes move is a capture or a promotion.
int staticPieceValues[6] = {200, 9, 5, 3, 3, 1};
int runStaticExchangeEvaluation(board& inputBoard, int move) {
  int score = 0;
  int from = move & 0x3f;
  int to = (move & 0xfc0) >> 6;
  int promotion = (move & 0x3000) >> 12;
  int type = (move & 0xc000) >> 14;
  int ourOffset = 6 * inputBoard.sideToPlay;
  int theirOffset = 6 * (inputBoard.sideToPlay ^ 1);
  int relativePiece = getPiece(inputBoard, from) - ourOffset;
  uint64_t attackers = piecesAttackingSquare(inputBoard, to);
  uint64_t queens = inputBoard.bitboards[1] | inputBoard.bitboards[7];
  uint64_t rooks = inputBoard.bitboards[2] | inputBoard.bitboards[8];
  uint64_t bishops = inputBoard.bitboards[3] | inputBoard.bitboards[9];
  // Remove piece that just attacked.
  attackers ^= bits[from];
  // Add any attackers that are now unblocked from piece moving.
  if (relativePiece != 4) {
    int scanRay = fromToToRays[to][from];
    uint64_t relavantSliders = (scanRay & 1) == 0 ? queens | rooks : queens | bishops;
    attackers |= relavantSliders & generalRayAttacks(inputBoard, from, scanRay);
  }
  if (type == 0) {
    score += staticPieceValues[getPiece(inputBoard, to) - theirOffset];
  } else if (type == 1) {
    if ((inputBoard.bitboards[14] & bits[to]) != 0ull) score += staticPieceValues[getPiece(inputBoard, to) - theirOffset];
    int promotionPiece = ourOffset + promotion + 1;
    score += staticPieceValues[promotionPiece - ourOffset] - staticPieceValues[5];
  } else if (type == 2) {
    score += staticPieceValues[5];
    // Add any attackers that are now unblocked from En Passant target getting removed.
    uint64_t rookTypePieces = queens | rooks;
    uint64_t uncoveredAttackers = inputBoard.sideToPlay == 0 ? rookTypePieces & lsbRayAttacks(inputBoard, to - 8, 6) : rookTypePieces & msbRayAttacks(inputBoard, to + 8, 2);
    attackers |= uncoveredAttackers;
  }
  inputBoard.sideToPlay ^= 1;
  score -= staticExchangeEvaluation(inputBoard, to, attackers, relativePiece);
  inputBoard.sideToPlay ^= 1;
  return score; 
}
int staticExchangeEvaluation(board& inputBoard, int square, uint64_t attackers, int lastPieceToMove) {
  int score = 0;
  uint64_t dangerousAttackers = attackers & inputBoard.bitboards[12 + inputBoard.sideToPlay];
  if (dangerousAttackers != 0ull) {
    // Get lightest attacker.
    int pieceOffset = 6 * inputBoard.sideToPlay;
    int attackerSquare;
    int attackingPiece;
    for (int piece = 5; piece >= 0; piece--) {
      uint64_t attackingPieces = dangerousAttackers & inputBoard.bitboards[pieceOffset + piece];
      if (attackingPieces != 0ull) {
        attackerSquare = msbPosition(attackingPieces);
        attackingPiece = piece;
        break;
      }
    }
    // Remove piece that just attacked.
    attackers ^= bits[attackerSquare];
    // Add any attackers that are now unblocked from piece moving.
    if (attackingPiece != 4) {
      uint64_t queens = inputBoard.bitboards[1] | inputBoard.bitboards[7];
      uint64_t rooks = inputBoard.bitboards[2] | inputBoard.bitboards[8];
      uint64_t bishops = inputBoard.bitboards[3] | inputBoard.bitboards[9];
      int scanRay = fromToToRays[square][attackerSquare];
      uint64_t relavantSliders = (scanRay & 1) == 0 ? queens | rooks : queens | bishops;
      attackers |= relavantSliders & generalRayAttacks(inputBoard, attackerSquare, scanRay);
    }
    inputBoard.sideToPlay ^= 1;
    score = std::max(0, staticPieceValues[lastPieceToMove] - staticExchangeEvaluation(inputBoard, square, attackers, attackingPiece));
    inputBoard.sideToPlay ^= 1;
  }
  return score;
}

#include <math.h>
#include <iostream>
#include "bits.h"
uint64_t bits[64];
uint64_t kingAttacks[64];
uint64_t extendedKingAttacks[64];
uint64_t rayAttacks[8][64];
uint64_t knightAttacks[64];
uint64_t pawnAttacks[2][64];
uint64_t files[8];
uint64_t ranks[8];
uint64_t passedPawnMasks[2][64];
uint64_t isolatedPawnMasks[8];
uint64_t pawnBehindMasks[2][64];
uint64_t forwardMasks[2][64];
int kingDistanceBetween[64][64];
int oneDimensionalToTwoDimensional[64][2];
int fromToToRays[64][64];
char filesToCharacters[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
char ranksToCharacters[8] = {'8', '7', '6', '5', '4', '3', '2', '1'};
int oppositeRays[8] = {4, 5, 6, 7, 0, 1, 2, 3};
int castlingPermissions[4] = {0b1000, 0b0100, 0b0010, 0b0001};
void printBitboard(uint64_t bitboard) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if ((bitboard & bits[8 * y + x]) != 0ull) {
        std::cout << "1 ";
      } else {
        std::cout << "0 ";
      }
    }
    std::cout << "\n\n";
  }
  std::cout << "\n\n";
}
void initializeMasks() {
  uint64_t currentBit = 0x8000000000000000ull;
  for (int i = 0; i < 64; i++) {
    bits[i] = currentBit;
    currentBit >>= 1ull;
  }
  int square = 0;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      uint64_t kingAttack = 0ull;
      for (int squareY = y - 1; squareY <= y + 1; squareY++) {
        for (int squareX = x - 1; squareX <= x + 1; squareX++) {
          if ((squareX != x || squareY != y) && squareX >= 0 && squareX < 8 && squareY >= 0 && squareY < 8) kingAttack |= bits[8 * squareY + squareX];
        }
      }
      kingAttacks[square] = kingAttack;
      uint64_t extendedKingAttack = 0ull;
      for (int squareY = y - 2; squareY <= y + 2; squareY++) {
        for (int squareX = x - 2; squareX <= x + 2; squareX++) {
          if ((squareX != x || squareY != y) && squareX >= 0 && squareX < 8 && squareY >= 0 && squareY < 8) extendedKingAttack |= bits[8 * squareY + squareX];
        }
      }
      extendedKingAttacks[square] = extendedKingAttack;
      int rayX, rayY;
      rayX = x + 1;
      uint64_t eastRay = 0ull;
      while (rayX < 8) {
        eastRay |= bits[8 * y + rayX];
        rayX++;
      }
      rayAttacks[0][square] = eastRay;
      rayX = x + 1;
      rayY = y + 1;
      uint64_t southEastRay = 0ull;
      while (rayX < 8 && rayY < 8) {
        southEastRay |= bits[8 * rayY + rayX];
        rayX++;
        rayY++;
      }
      rayAttacks[1][square] = southEastRay;
      rayY = y + 1;
      uint64_t southRay = 0ull;
      while (rayY < 8) {
        southRay |= bits[8 * rayY + x];
        rayY++;
      }
      rayAttacks[2][square] = southRay;
      rayX = x - 1;
      rayY = y + 1;
      uint64_t southWestRay = 0ull;
      while (rayX > -1 && rayY < 8) {
        southWestRay |= bits[8 * rayY + rayX];
        rayX--;
        rayY++;
      }
      rayAttacks[3][square] = southWestRay;
      rayX = x - 1;
      uint64_t westRay = 0ull;
      while (rayX > -1) {
        westRay |= bits[8 * y + rayX];
        rayX--;
      }
      rayAttacks[4][square] = westRay;
      rayX = x - 1;
      rayY = y - 1;
      uint64_t northWestRay = 0ull;
      while (rayX > -1 && rayY > -1) {
        northWestRay |= bits[8 * rayY + rayX];
        rayX--;
        rayY--;
      }
      rayAttacks[5][square] = northWestRay;
      rayY = y - 1;
      uint64_t northRay = 0ull;
      while (rayY > -1) {
        northRay |= bits[8 * rayY + x];
        rayY--;
      }
      rayAttacks[6][square] = northRay;
      rayX = x + 1;
      rayY = y - 1;
      uint64_t northEastRay = 0ull;
      while (rayX < 8 && rayY > -1) {
        northEastRay |= bits[8 * rayY + rayX];
        rayX++;
        rayY--;
      }
      rayAttacks[7][square] = northEastRay;
      uint64_t knightAttack = 0ull;
      if (x - 2 > -1 && y - 1 > -1) knightAttack |= bits[8 * (y - 1) + (x - 2)];
      if (x - 1 > -1 && y - 2 > -1) knightAttack |= bits[8 * (y - 2) + (x - 1)];
      if (x + 1 < 8 && y - 2 > -1) knightAttack |= bits[8 * (y - 2) + (x + 1)];
      if (x + 2 < 8 && y - 1 > -1) knightAttack |= bits[8 * (y - 1) + (x + 2)];
      if (x + 2 < 8 && y + 1 < 8) knightAttack |= bits[8 * (y + 1) + (x + 2)];
      if (x + 1 < 8 && y + 2 < 8) knightAttack |= bits[8 * (y + 2) + (x + 1)];
      if (x - 1 > -1 && y + 2 < 8) knightAttack |= bits[8 * (y + 2) + (x - 1)];
      if (x - 2 > -1 && y + 1 < 8) knightAttack |= bits[8 * (y + 1) + (x - 2)];
      knightAttacks[square] = knightAttack;
      uint64_t blackPawnAttack = 0ull;
      uint64_t whitePawnAttack = 0ull;
      if (x - 1 > -1 && y + 1 < 8) blackPawnAttack |= bits[8 * (y + 1) + (x - 1)];
      if (x + 1 < 8 && y + 1 < 8) blackPawnAttack |= bits[8 * (y + 1) + (x + 1)];
      if (x - 1 > -1 && y - 1 > -1) whitePawnAttack |= bits[8 * (y - 1) + (x - 1)];
      if (x + 1 < 8 && y - 1 > -1) whitePawnAttack |= bits[8 * (y - 1) + (x + 1)];
      pawnAttacks[0][square] = blackPawnAttack;
      pawnAttacks[1][square] = whitePawnAttack;
      oneDimensionalToTwoDimensional[square][0] = x;
      oneDimensionalToTwoDimensional[square][1] = y;
      uint64_t whitePassedPawnMask = 0ull;
      uint64_t blackPassedPawnMask = 0ull;
      uint64_t upwardDirectionMask = 0ull;
      uint64_t downwardDirectionMask = 0ull;
      for (int rank = y - 1; rank > -1; rank--) {
        if (x - 1 > -1) whitePassedPawnMask |= bits[8 * rank + (x - 1)];
        whitePassedPawnMask |= bits[8 * rank + x];
        if (x + 1 < 8) whitePassedPawnMask |= bits[8 * rank + (x + 1)];
        upwardDirectionMask |= bits[8 * rank + x];
      }
      for (int rank = y + 1; rank < 8; rank++) {
        if (x - 1 > -1) blackPassedPawnMask |= bits[8 * rank + (x - 1)];
        blackPassedPawnMask |= bits[8 * rank + x];
        if (x + 1 < 8) blackPassedPawnMask |= bits[8 * rank + (x + 1)];
        downwardDirectionMask |= bits[8 * rank + x];
      }
      passedPawnMasks[0][square] = blackPassedPawnMask;
      passedPawnMasks[1][square] = whitePassedPawnMask;
      forwardMasks[0][square] = downwardDirectionMask;
      forwardMasks[1][square] = upwardDirectionMask;
      uint64_t whitePawnBehindMask = 0ull;
      uint64_t blackPawnBehindMask = 0ull;
      for (int rank = y; rank < 8; rank++) {
        if (x - 1 > -1) whitePawnBehindMask |= bits[8 * rank + (x - 1)];
        if (x + 1 < 8) whitePawnBehindMask |= bits[8 * rank + (x + 1)];
      }
      for (int rank = y; rank > -1; rank--) {
        if (x - 1 > -1) blackPawnBehindMask |= bits[8 * rank + (x - 1)];
        if (x + 1 < 8) blackPawnBehindMask |= bits[8 * rank + (x + 1)];
      }
      pawnBehindMasks[1][square] = whitePawnBehindMask;
      pawnBehindMasks[0][square] = blackPawnBehindMask;
      square++;
    }
  }
  for (int x = 0; x < 8; x++) {
    uint64_t isolatedPawnMask = 0ull;
    for (int y = 0; y < 8; y++) {
      if (x - 1 > -1) isolatedPawnMask |= bits[8 * y + (x - 1)];
      if (x + 1 < 8) isolatedPawnMask |= bits[8 * y + (x + 1)];
    }
    isolatedPawnMasks[x] = isolatedPawnMask;
  }
  uint64_t file = 0x8080808080808080ull;
  uint64_t rank = 0xff00000000000000ull;
  for (int i = 0; i < 8; i++) {
    files[i] = file;
    ranks[i] = rank;
    file >>= 1ull;
    rank >>= 8ull;
  }
  for (int from = 0; from < 64; from++) {
    int fromX = oneDimensionalToTwoDimensional[from][0];
    int fromY = oneDimensionalToTwoDimensional[from][1];
    for (int to = 0; to < 64; to++) {
      int toX = oneDimensionalToTwoDimensional[to][0];
      int toY = oneDimensionalToTwoDimensional[to][1];
      int deltaX = toX - fromX;
      int deltaY = toY - fromY;
      kingDistanceBetween[from][to] = std::max(std::abs(deltaX), std::abs(deltaY));
      fromToToRays[from][to] = 8;
      if (deltaX > 0 && deltaY == 0) fromToToRays[from][to] = 0;
      if (deltaX == 0 && deltaY > 0) fromToToRays[from][to] = 2;
      if (deltaX < 0 && deltaY == 0) fromToToRays[from][to] = 4;
      if (deltaX == 0 && deltaY < 0) fromToToRays[from][to] = 6;
      if (std::abs(deltaX) != std::abs(deltaY)) continue;
      if (deltaX > 0 && deltaY > 0) fromToToRays[from][to] = 1;
      if (deltaX < 0 && deltaY > 0) fromToToRays[from][to] = 3;
      if (deltaX < 0 && deltaY < 0) fromToToRays[from][to] = 5;
      if (deltaX > 0 && deltaY < 0) fromToToRays[from][to] = 7;
    }
  }
}
int msbPosition(uint64_t x) { 
  return __builtin_clzll(x); 
}
int lsbPosition(uint64_t x) { 
  return 63 - __builtin_ctzll(x); 
}

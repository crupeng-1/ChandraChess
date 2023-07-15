#include <iostream>
#include <math.h>
#include <random>
#include "bits.h"
#include "hashKey.h"
#include "state.h"
uint64_t sideToPlayKeys[2];
uint64_t positionKeys[13][64];
uint64_t castlingPermissionsKeys[16];
uint64_t enPassantSquareKeys[64];
void initializeKeys() {
  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_int_distribution<long long int> dist(std::llround(0), std::llround(std::pow(2, 64) - 1));
  sideToPlayKeys[0] = dist(e2);
  sideToPlayKeys[1] = dist(e2);
  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 64; j++) {
      positionKeys[i][j] = dist(e2);
    }
  }
  for (int i = 0; i < 16; i++) {
    castlingPermissionsKeys[i] = dist(e2);
  }
  for (int i = 0; i < 64; i++) {
    enPassantSquareKeys[i] = dist(e2);
  }
}
uint64_t getKey(board& inputBoard) {
  uint64_t key = 0ull;
  for (int i = 0; i < 64; i++) {
    uint64_t bit = bits[i];
    bool isSquareEmpty = true;
    for (int j = 0; j < 12; j++) {
      if ((inputBoard.bitboards[j] & bit) == bit) {
        key ^= positionKeys[j][i];
        isSquareEmpty = false;
        break;
      }
    }
    if (isSquareEmpty) key ^= positionKeys[12][i];
  }
  key ^= sideToPlayKeys[inputBoard.sideToPlay];
  key ^= castlingPermissionsKeys[inputBoard.castlingPermission];
  key ^= enPassantSquareKeys[inputBoard.enPassantSquare];
  return key;
}

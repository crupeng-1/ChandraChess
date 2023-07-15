#include <math.h>
#include <iostream>
#include "updateState.h"
#include "bits.h"
#include "state.h"
#include "table.h"
#include "hashKey.h"
#include "board.h"
int castlingPermissionMasks[64] = {
  0b1011, 0b1111, 0b1111, 0b1111, 0b0011, 0b1111, 0b1111, 0b0111,
  0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
  0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
  0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
  0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
  0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
  0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
  0b1110, 0b1111, 0b1111, 0b1111, 0b1100, 0b1111, 0b1111, 0b1101
};
void makeMove(board& inputBoard, int move) {
  historyEntryRequiredForNullMove& newestHistoryEntryRequiredForNullMove = inputBoard.historyRequiredForNullMove[inputBoard.numberOfHistoryRequiredForNullMove];
  newestHistoryEntryRequiredForNullMove.enPassantSquare = inputBoard.enPassantSquare;
  newestHistoryEntryRequiredForNullMove.currentKey = inputBoard.currentKey;
  newestHistoryEntryRequiredForNullMove.halfMoveClock = inputBoard.halfMoveClock;
  historyEntryNotRequiredForNullMove& newestHistoryEntryNotRequiredForNullMove = inputBoard.historyNotRequiredForNullMove[inputBoard.numberOfHistoryNotRequiredForNullMove];
  newestHistoryEntryNotRequiredForNullMove.castlingPermission = inputBoard.castlingPermission;
  newestHistoryEntryNotRequiredForNullMove.pieceCapturedOnNextMove = 0;
  bool isResettingEnPassantSquare = true;
  int from = move & 0x3f;
  int to = (move & 0xfc0) >> 6;
  int promotion = (move & 0x3000) >> 12;
  int type = (move & 0xc000) >> 14;
  uint64_t fromBit = bits[from];
  uint64_t toBit = bits[to];
  int ourOffset = 6 * inputBoard.sideToPlay;
  inputBoard.currentKey ^= castlingPermissionsKeys[inputBoard.castlingPermission];
  inputBoard.castlingPermission &= castlingPermissionMasks[from];
  inputBoard.castlingPermission &= castlingPermissionMasks[to];
  inputBoard.currentKey ^= castlingPermissionsKeys[inputBoard.castlingPermission];
  bool isCapture = (inputBoard.bitboards[14] & toBit) != 0ull;
  if (isCapture) newestHistoryEntryNotRequiredForNullMove.pieceCapturedOnNextMove = getPiece(inputBoard, to);
  bool isPiecePawn = (inputBoard.bitboards[ourOffset + 5] & fromBit) != 0ull;
  if (isCapture || isPiecePawn) { inputBoard.halfMoveClock = 0; } else { inputBoard.halfMoveClock++; };
  inputBoard.currentKey ^= enPassantSquareKeys[inputBoard.enPassantSquare];
  if (type == 0) {
    int fromSquareToDoublePushToSquare = inputBoard.sideToPlay == 0 ? 16 : -16;
    if (from + fromSquareToDoublePushToSquare == to && isPiecePawn) {
      int fromSquareToEnPassantSquare = inputBoard.sideToPlay == 0 ? 8 : -8;
      inputBoard.enPassantSquare = from + fromSquareToEnPassantSquare;
      isResettingEnPassantSquare = false;
    }
    if (isCapture) removePiece(inputBoard, to, true);
    movePiece(inputBoard, from, to, true);
  } else if (type == 1) {
    if (isCapture) removePiece(inputBoard, to, true);
    int promotionPiece = ourOffset + promotion + 1;
    removePiece(inputBoard, from, true);
    addPiece(inputBoard, promotionPiece, to, true);
  } else if (type == 2) {
    int toSquareToEnPassantCapturedPieceSquare = inputBoard.sideToPlay == 0 ? -8 : 8;
    removePiece(inputBoard, to + toSquareToEnPassantCapturedPieceSquare, true);
    movePiece(inputBoard, from, to, true);
  } else {
    movePiece(inputBoard, from, to, true);
    int kingSideCastlingToSquare = inputBoard.sideToPlay == 0 ? 6 : 62;
    if (to == kingSideCastlingToSquare) {
      int initialKingSideRookSquare = inputBoard.sideToPlay == 0 ? 7 : 63;
      movePiece(inputBoard, initialKingSideRookSquare, initialKingSideRookSquare - 2, true);
    } else {
      int initialQueenSideRookSquare = inputBoard.sideToPlay == 0 ? 0 : 56;
      movePiece(inputBoard, initialQueenSideRookSquare, initialQueenSideRookSquare + 3, true);
    }
  }
  if (isResettingEnPassantSquare == true) inputBoard.enPassantSquare = 0;
  inputBoard.currentKey ^= enPassantSquareKeys[inputBoard.enPassantSquare];
  inputBoard.currentKey ^= sideToPlayKeys[inputBoard.sideToPlay];
  inputBoard.sideToPlay ^= 1;
  inputBoard.currentKey ^= sideToPlayKeys[inputBoard.sideToPlay];
  inputBoard.distanceToRoot++;
  inputBoard.halfMovesDone++;
  inputBoard.numberOfHistoryRequiredForNullMove++;
  inputBoard.numberOfHistoryNotRequiredForNullMove++;
}
void takeMove(board& inputBoard, int move) {
  inputBoard.sideToPlay ^= 1;
  historyEntryRequiredForNullMove& newestHistoryEntryRequiredForNullMove = inputBoard.historyRequiredForNullMove[inputBoard.numberOfHistoryRequiredForNullMove - 1];
  inputBoard.enPassantSquare = newestHistoryEntryRequiredForNullMove.enPassantSquare;
  inputBoard.currentKey = newestHistoryEntryRequiredForNullMove.currentKey;
  inputBoard.halfMoveClock = newestHistoryEntryRequiredForNullMove.halfMoveClock;
  historyEntryNotRequiredForNullMove& newestHistoryEntryNotRequiredForNullMove = inputBoard.historyNotRequiredForNullMove[inputBoard.numberOfHistoryNotRequiredForNullMove - 1];
  inputBoard.castlingPermission = newestHistoryEntryNotRequiredForNullMove.castlingPermission;
  int capturedPiece = newestHistoryEntryNotRequiredForNullMove.pieceCapturedOnNextMove;
  int from = move & 0x3f;
  int to = (move & 0xfc0) >> 6;
  int type = (move & 0xc000) >> 14;
  if (type == 0) {
    movePiece(inputBoard, to, from, false);
    if (capturedPiece != 0) addPiece(inputBoard, capturedPiece, to, false);
  } else if (type == 1) {
    removePiece(inputBoard, to, false);
    if (capturedPiece != 0) addPiece(inputBoard, capturedPiece, to, false);
    int ourPawn = 6 * inputBoard.sideToPlay + 5;
    addPiece(inputBoard, ourPawn, from, false);
  } else if (type == 2) {
    int toSquareToEnPassantCapturedPieceSquare = inputBoard.sideToPlay == 0 ? -8 : 8;
    int theirPawn = 6 * (inputBoard.sideToPlay ^ 1) + 5;
    addPiece(inputBoard, theirPawn, to + toSquareToEnPassantCapturedPieceSquare, false);
    movePiece(inputBoard, to, from, false);
  } else {
    movePiece(inputBoard, to, from, false); 
    int kingSideCastlingToSquare = inputBoard.sideToPlay == 0 ? 6 : 62;
    if (to == kingSideCastlingToSquare) {
      int initialKingSideRookSquare = inputBoard.sideToPlay == 0 ? 7 : 63;
      movePiece(inputBoard, initialKingSideRookSquare - 2, initialKingSideRookSquare, false);
    } else {
      int initialQueenSideRookSquare = inputBoard.sideToPlay == 0 ? 0 : 56;
      movePiece(inputBoard, initialQueenSideRookSquare + 3, initialQueenSideRookSquare, false);
    }
  }
  inputBoard.distanceToRoot--;
  inputBoard.halfMovesDone--;
  inputBoard.numberOfHistoryRequiredForNullMove--;
  inputBoard.numberOfHistoryNotRequiredForNullMove--;
}
void makeNullMove(board& inputBoard) {
  historyEntryRequiredForNullMove& newestHistoryEntryRequiredForNullMove = inputBoard.historyRequiredForNullMove[inputBoard.numberOfHistoryRequiredForNullMove];
  newestHistoryEntryRequiredForNullMove.enPassantSquare = inputBoard.enPassantSquare;
  newestHistoryEntryRequiredForNullMove.currentKey = inputBoard.currentKey;
  newestHistoryEntryRequiredForNullMove.halfMoveClock = inputBoard.halfMoveClock;
  inputBoard.currentKey ^= enPassantSquareKeys[inputBoard.enPassantSquare];
  inputBoard.currentKey ^= sideToPlayKeys[inputBoard.sideToPlay];
  inputBoard.enPassantSquare = 0;
  inputBoard.sideToPlay ^= 1;
  inputBoard.currentKey ^= enPassantSquareKeys[inputBoard.enPassantSquare];
  inputBoard.currentKey ^= sideToPlayKeys[inputBoard.sideToPlay];
  inputBoard.distanceToRoot++;
  inputBoard.halfMovesDone++;
  inputBoard.halfMoveClock = 0;
  inputBoard.numberOfHistoryRequiredForNullMove++;
}
void takeNullMove(board& inputBoard) {
  historyEntryRequiredForNullMove& newestHistoryEntryRequiredForNullMove = inputBoard.historyRequiredForNullMove[inputBoard.numberOfHistoryRequiredForNullMove - 1];
  inputBoard.enPassantSquare = newestHistoryEntryRequiredForNullMove.enPassantSquare;
  inputBoard.currentKey = newestHistoryEntryRequiredForNullMove.currentKey;
  inputBoard.halfMoveClock = newestHistoryEntryRequiredForNullMove.halfMoveClock;
  inputBoard.sideToPlay ^= 1;
  inputBoard.distanceToRoot--;
  inputBoard.halfMovesDone--;
  inputBoard.numberOfHistoryRequiredForNullMove--;
}

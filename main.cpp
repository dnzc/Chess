#include "Board.h"
#include <iostream>

int main() {

  Board b = Board();

  Board::display8x8(b.lookupRookMoves(12, 18446462598732906495));

  b.displayBoard();

  while(true) {
    // get move
    char startFile, startRank, endFile, endRank;
    std::cin >> startFile >> startRank >> endFile >> endRank;

    int start = (startRank-'1')*8 + (startFile-'a');
    int end = (endRank-'1')*8 + (endFile-'a');
    b.makeMove(start, end);
    b.displayBoard();
  }

  return 0;
};

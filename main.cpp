#include "Position.h"
#include "Bitboard.h"
#include "Movegen.h"
#include "Terminal.h"
#include <iostream>
#include <bit>

int main() {

  Position p;
  Movegen m;

  while(true) {
    Terminal::display(p);

    std::vector<Move> legalMoves = m.genMoves(p);
    for(auto i: legalMoves) {
      std::cout << i.start << "," << i.end << "," << i.piece << " ";
    }
    std::cout << "\n";

    // get move
    int start, end;
    while(true) {
      std::cout << "Enter move: ";
      char startFile, startRank, endFile, endRank;
      std::cin >> startFile >> startRank >> endFile >> endRank;

      start = (startRank-'1')*8 + (startFile-'a');
      end = (endRank-'1')*8 + (endFile-'a');

      // check if legal move
      bool legal = false;
      for(Move i : legalMoves) {
        if(start == i.start && end == i.end) {
          legal = true;
          p.makeMove(i);
          break;
        }
      }
      if(legal) break;
    }

  }

  return 0;
};

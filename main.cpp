#include "Position.h"
#include "Bitboard.h"
#include "MoveGenerator.h"
#include "Engine.h"
#include "Util.h"
#include <iostream>
#include <bit>

// GROUP A SKILL - recursion
int numPositions(int depth, Position& p, MoveGenerator& m) {
  if(depth == 0) return 1;
  int numPos = 0;
  for(auto move : m.genMoves(p)) {
    Position next = p;
    next.makeMove(move);
    numPos += numPositions(depth-1, next, m);
  }
  return numPos;
}

void movegenTest(int depth, Position p) {
  MoveGenerator m;
  int total = 0;
  for(auto move : m.genMoves(p)) {
    Position next = p;
    next.makeMove(move);
    int res = numPositions(depth-1, next, m);
    std::cout << (char)((move.start&7)+'a') << (move.start>>3)+1
      << (char)((move.end&7)+'a') << (move.end>>3)+1
      << ": " << res << "\n";
    total += res;
  }
  std::cout << "total at depth " << depth << ": " << total << "\n";
}

int main() {

  //Engine e("1k6/6b1/2Q4q/8/8/8/2K5/8 w - - 0 1");
  Engine e;
  
  //movegenTest(5, e.getPos());

  while(true) {
    Util::display(e.getPos());

    std::vector<Move> legalMoves = e.getLegalMoves();

    // get move
    while(true) {
      std::cout << "Enter move: ";
      char startFile, startRank, endFile, endRank;
      std::cin >> startFile >> startRank >> endFile >> endRank;

      int start = (startRank-'1')*8 + (startFile-'a');
      int end = (endRank-'1')*8 + (endFile-'a');

      // check if legal move
      std::vector<Move> possibleMoves;
      for(Move i : legalMoves) {
        if(start == i.start && end == i.end) {
          possibleMoves.push_back(i);
        }
      }

      if(possibleMoves.size() == 1) {
        Move move = possibleMoves[0];
        e.makeMove(move);
        break;
      }
      else if(possibleMoves.size() > 1) { // if promotion
        PieceType promote;
        while(true) {
          std::cout << "Promote to queen (q), rook (r), bishop (b) or knight(n)? ";
          char piece; std::cin >> piece;
          bool isWhite = possibleMoves[0].piece < 6;
          if(piece=='q') {
            promote = isWhite ? wq : bq;
            break;
          }
          else if(piece=='r') {
            promote = isWhite ? wr : br;
            break;
          }
          else if(piece=='b') {
            promote = isWhite ? wb : bb;
            break;
          }
          else if(piece=='n') {
            promote = isWhite ? wn : bn;
            break;
          }
        }
        Move move = possibleMoves[0];
        e.makeMove(Move(move.start, move.end, move.piece, false, promote, false));
        break;
      }

    }

    e.makeMove(e.MCTS(5000, true));

  }

  return 0;
};

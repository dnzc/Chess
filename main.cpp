#include "Position.h"
#include "Bitboard.h"
#include "MoveGenerator.h"
#include "Engine.h"
#include "Util.h"
#include <iostream>
#include <bit>
#include <string>

// GROUP A SKILL - recursion
int numPositions(int depth, Position& p, MoveGenerator& m) {
  if(depth == 0) return 1;
  int numPos = 0;
  for(auto move : m.genMoves(p, false)) {
    Position next = p;
    next.makeMove(move);
    numPos += numPositions(depth-1, next, m);
  }
  return numPos;
}

void movegenTest(Position p) {
  std::cout << "Move generation test:\nDepth? ";
  int depth; std::cin >> depth;
  MoveGenerator m;
  int total = 0;
  for(auto move : m.genMoves(p, false)) {
    Position next = p;
    next.makeMove(move);
    int res = numPositions(depth-1, next, m);
    std::cout << "  " << (char)((move.start&7)+'a') << (move.start>>3)+1
      << (char)((move.end&7)+'a') << (move.end>>3)+1
      << ": " << res << "\n";
    total += res;
  }
  std::cout << "Total at depth " << depth << ": " << total << "\n\n";
}

Move getUserMove(Engine& e) {
  std::vector<Move> legalMoves = e.getLegalMoves();
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
      return possibleMoves[0];
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
      return Move(move.start, move.end, move.piece, false, promote, false);
    }

  }
}

int main() {

  Engine e;

  // load FEN
  std::cout << "Enter FEN to load (or press enter):\n";
  std::string FEN; std::getline(std::cin, FEN);
  if(!FEN.empty()) e = Engine(FEN);
  
  movegenTest(e.getPos());

  // get players
  int whitePlayer;
  int whiteComputerTime;
  while(true) {
    std::cout << "White: human (0), MCTS (1), MCTS-AB (2) or minimax (3) ? ";
    int n; std::cin >> n;
    if(n==1 || n==2 || n==3) {
      std::cout << "How many milliseconds per move? ";
      std::cin >> whiteComputerTime;
    }
    if(n>=0 && n<4) {
      whitePlayer = n;
      break;
    }
  }
  int blackPlayer;
  int blackComputerTime;
  while(true) {
    std::cout << "\nBlack: human (0), MCTS (1), MCTS-AB (2) or minimax (3) ? ";
    int n; std::cin >> n;
    if(n==1 || n==2 || n==3) {
      std::cout << "How many milliseconds per move? ";
      std::cin >> blackComputerTime;
    }
    if(n>=0 && n<4) {
      blackPlayer = n;
      break;
    }
  }

  std::cout << "\n";
  Util::display(e.getPos());
  bool isWhite = true;
  while(!e.isGameOver()) {

    switch(isWhite ? whitePlayer : blackPlayer) {
      case 0: e.makeMove(getUserMove(e)); break;
      case 1: e.makeMove(e.MCTS(isWhite ? whiteComputerTime : blackComputerTime, false, true)); break;
      case 2: e.makeMove(e.MCTS(isWhite ? whiteComputerTime : blackComputerTime, true, true)); break;
      case 3: e.makeMove(e.minimax(isWhite ? whiteComputerTime : blackComputerTime, true)); break;
    }

    Util::display(e.getPos());
    isWhite = !isWhite;

  }
  if(e.isGameOver() == 1) std::cout << "Draw!\n";
  else std::cout << "Checkmate, " << (isWhite ? "Black" : "White") << " wins! \n";

  return 0;
};

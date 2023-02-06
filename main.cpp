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

void movegenTest(Position p, int depth) {
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

void playGame(Engine& e, bool debug) {
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
  bool isWhite = e.getPos().isWhiteToMove();
  while(!e.isGameOver()) {

    switch(isWhite ? whitePlayer : blackPlayer) {
      case 0: e.makeMove(getUserMove(e)); break;
      case 1: e.makeMove(e.MCTS(isWhite ? whiteComputerTime : blackComputerTime, false, debug)); break;
      case 2: e.makeMove(e.MCTS(isWhite ? whiteComputerTime : blackComputerTime, true, debug)); break;
      case 3: e.makeMove(e.minimax(isWhite ? whiteComputerTime : blackComputerTime, debug)); break;
    }

    Util::display(e.getPos());
    isWhite = !isWhite;

  }
  if(e.isGameOver() == 1) std::cout << "Draw!\n";
  else std::cout << "Checkmate, " << (isWhite ? "Black" : "White") << " wins! \n";
}

int main() {

  Engine e;

  // terminal interface
  std::cout << "Engine successfully initalized.\n> ";
  std::string line;
  while(std::getline(std::cin, line)) {
    int split = line.find(" ");
    std::string command = line.substr(0, split);
    if(command == "help") {
      std::cout << "\nFormat:\ncommand <argument:type(default_value)> <...> | description \n--------------------------------------------------------------- \n \nhelp | get help about the CLI\n \nperft <depth:int(3)> | calculate the number of games at a certain depth\n \nposition | set/reset the current position\n \nd | display the current position\n \nmcts <time:int(3000)> | run mcts for a set number of milliseconds\n \nmctsab <time:int(3000)> | run mcts-ab for a set number of milliseconds\n \nminimax <time:int(3000)> | run minimax for a set number of milliseconds\n \ngame <debug:bool(false)> | start a game\n \nquit | quit the program \n \n";

    } else if(command == "perft") {
      bool valid = true;
      int depth = 3;
      if(line != command) {
        try {
          depth = std::stoi(line.substr(split, line.length()));
          if(depth <= 0) {
            std::cout << "Error: depth should be at least 1.";
            valid = false;
          } else if(depth >= 7) {
            std::cout << "Are you sure? this will take a while. (y/N) ";
            std::string x; std::getline(std::cin, x);
            if(x!="y") valid = false;
          }
        } catch (...) {
          std::cout << "Error: invalid argument.\n";
          valid = false;
        }
      }
      if(valid) movegenTest(e.getPos(), depth);
    } else if(command == "position") {
      // load FEN
      std::cout << "Enter FEN to load (or press enter to load start position):\n";
      std::string FEN; std::getline(std::cin, FEN);
      if(!FEN.empty()) e = Engine(FEN);
    } else if(command == "d") {
      std::cout << (e.getPos().isWhiteToMove() ? "White" : "Black") << " to move.\n";
      e.outputZobrist();
      Util::display(e.getPos());
    } else if(command == "mcts") {
      bool valid = true;
      int time = 3000;
      if(line != command) {
        try {
          time = std::stoi(line.substr(split, line.length()));
          if(time <= 0) {
            std::cout << "Error: time should be positive.\n";
            valid = false;
          } 
        } catch (...) {
          std::cout << "Error: invalid argument.\n";
          valid = false;
        }
      }
      if(valid) e.MCTS(time, false, true);
    } else if(command == "mctsab") {
      bool valid = true;
      int time = 3000;
      if(line != command) {
        try {
          time = std::stoi(line.substr(split, line.length()));
          if(time <= 0) {
            std::cout << "Error: time should be positive.\n";
            valid = false;
          } 
        } catch (...) {
          std::cout << "Error: invalid argument.\n";
          valid = false;
        }
      }
      if(valid) e.MCTS(time, true, true);
    } else if(command == "minimax") {
      bool valid = true;
      int time = 3000;
      if(line != command) {
        try {
          time = std::stoi(line.substr(split, line.length()));
          if(time <= 0) {
            std::cout << "Error: time should be positive.\n";
            valid = false;
          } 
        } catch (...) {
          std::cout << "Error: invalid argument.\n";
          valid = false;
        }
      }
      if(valid) e.minimax(time, true);
    } else if(command == "game") {
      bool debug = false;
      if(line != command) {
        debug = line.substr(split, line.length()) == " true";
      }
      playGame(e, debug);
    } else if(command == "quit") {
      std::cout << "Goodbye.\n";
      return 0;
    } else {
      std:: cout << "Unrecognized command.\n";
    }

    std::cout << "> ";
  }



  return 0;
};

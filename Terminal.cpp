#include "Terminal.h"
#include "Bitboard.h"
#include "Position.h"
#include <iostream>
#include <string>

void Terminal::display(Position& position) {

  std::string pieceSymbols[] = {"\u2659", "\u2658", "\u2657", "\u2656", "\u2655", "\u2654", "\u265F", "\u265E", "\u265D", "\u265C", "\u265B", "\u265A"};

  std::string board[64];
  for(int i=0; i<64; ++i) board[i] = " ";

  // draw each piece type
  for(int i=0; i<12; ++i) {
    // loop through each bit
    for(int j=0; j<64; ++j) {
      // if current bit is 1, then place piece symbol at corresponding position
      if( ((position.getPieces((PieceType)i)>>j)&1) == 1 ) {
        board[j] = pieceSymbols[i];
      }
    }
  }

  // output to screen
  display8x8(board);
}

void Terminal::display8x8(Bitboard bitboard) {
  std::string stringSymbols[64];
  for(int i=0; i<64; ++i) {
    Bitboard bit = (bitboard>>i) & 1;
    stringSymbols[i] = (bit==1) ? std::to_string(bit.getBits()) : " ";
  }
  display8x8(stringSymbols);
}

void Terminal::display8x8(std::string symbols[64]) {

  // useful strings
  std::string vb = "\u2502"; // vertical bar
  std::string hb = "\u2500"; // horizontal bar
  std::string tlc = "\u250c"; // top left corner
  std::string trc = "\u2510"; // top right corner
  std::string blc = "\u2514"; // bottom left corner
  std::string brc = "\u2518"; // bottom right corner
  std::string tm = "\u252c"; // top middle
  std::string bm = "\u2534"; // bottom middle
  std::string lm = "\u251c"; // left middle
  std::string rm = "\u2524"; // right middle
  std::string p = "\u253c"; // plus
  std::string b = "\u25e6"; // bullet
  std::string hb3 = hb + hb + hb; // horizonal bar x3
  // top row
  std::string tr = b + " " + tlc + hb3;
  for(int i=0; i<7; ++i) tr += tm + hb3;
  tr += trc + " " + b;
  // middle row
  std::string mr = b + " " + lm + hb3;
  for(int i=0; i<7; ++i) mr += p + hb3;
  mr += rm + " " + b;
  // bottom row
  std::string br = b + " " + blc + hb3;
  for(int i=0; i<7; ++i) br += bm + hb3;
  br += brc + " " + b;

  // print the file names
  std::cout << "  " + b + " ";
  for(int x=0; x<8; ++x) {
    std::cout << (char)('a'+x) << " " + b + " ";
  }
  std::cout << "\n";
  // print the top row
  std::cout << tr << "\n";

  // loop through each row of pieces
  for(int y=0; y<8; ++y) {
    // print the row of pieces
    std::cout << 8-y << " " + vb + " ";
    for(int x=0; x<8; ++x) {
      std::cout << symbols[(7-y)*8+x] << " " + vb + " ";
    }
    std::cout << 8-y << "\n";
    // print an intermediate row if necessary
    if(y<7) std::cout << mr << "\n";
  }

  // print the bottom row
  std::cout << br << "\n";
  // print the file names
  std::cout << "  " + b + " ";
  for(int x=0; x<8; ++x) {
    std::cout << (char)('a'+x) << " " + b + " ";
  }

  std::cout << "\n\n";

}

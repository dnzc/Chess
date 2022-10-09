#include "Board.h"
#include <iostream>
#include <cstdint>
#include <string>

Board::Board() {
  m_pieceBBs[wp].setBits(255 << 8);
  m_symbols[wp] = "\u2659";
  m_pieceBBs[wn].setBits(66);
  m_symbols[wn] = "\u2658";

  
  m_pieceBBs[wb].setBits(36ull);
  m_symbols[wb] = "\u2657";
  m_pieceBBs[wr].setBits(129ull);
  m_symbols[wr] = "\u2656";
  m_pieceBBs[wq].setBits(16ull);
  m_symbols[wq] = "\u2655";
  m_pieceBBs[wk].setBits(8ull);
  m_symbols[wk] = "\u2654";
  m_pieceBBs[bp].setBits(255ull << 48);
  m_symbols[bp] = "\u265F";
  m_pieceBBs[bn].setBits(66ull << 56);
  m_symbols[bn] = "\u265E";
  m_pieceBBs[bb].setBits(36ull << 56);
  m_symbols[bb] = "\u265D";
  m_pieceBBs[br].setBits(129ull << 56);
  m_symbols[br] = "\u265C";
  m_pieceBBs[bq].setBits(16ull << 56);
  m_symbols[bq] = "\u265B";
  m_pieceBBs[bk].setBits(8ull << 56);
  m_symbols[bk] = "\u265A";
}

void Board::printBoard() {
  std::string board[8][8];
  // draw each piece type
  for(int i=0; i<12; ++i) {
    // loop through each bit
    for(int j=0; j<64; ++j) {
      // if current bit is 1, then place piece symbol at corresponding position
      if(m_pieceBBs[i].getBit(j)) {
        board[7 - (int)(j/8)][j%8] = m_symbols[i];
      }
    }
  }

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
  std::string hb3 = hb + hb + hb; // horizonal bar x3
  // top row
  std::string tr = tlc + hb3;
  for(int i=0; i<7; ++i) tr += tm + hb3;
  tr += trc;
  // middle row
  std::string mr = lm + hb3;
  for(int i=0; i<7; ++i) mr += p + hb3;
  mr += rm;
  // bottom row
  std::string br = blc + hb3;
  for(int i=0; i<7; ++i) br += bm + hb3;
  br += brc;

  // output the board representation
  
  // print the top row
  std::cout << tr << "\n";
  // loop through each row of pieces
  for(int y=0; y<8; ++y) {
    // print the row of pieces
    std::cout << vb + " ";
    for(int x=0; x<8; ++x) {
      std::string symbol = (board[y][x] == "" ? " " : board[y][x]);
      std::cout << symbol << " " + vb + " ";
    }
    std::cout << "\n";
    // print an intermediate row if necessary
    if(y<7) std::cout << mr << "\n";
  }
  // print the bottom row
  std::cout << br << "\n";
}

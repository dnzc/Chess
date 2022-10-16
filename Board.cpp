#include "Board.h"
#include <iostream>
#include <cstdint>
#include <string>
#include <random>

Board::Board() {

  // initialize piece bitboards and symbols
  m_pieces[wp] = 255ull << 8;
  m_symbols[wp] = "\u2659";
  m_pieces[wn] = 66ull; m_symbols[wn] = "\u2658"; m_pieces[wb] = 36ull;
  m_symbols[wb] = "\u2657";
  m_pieces[wr] = 129ull;
  m_symbols[wr] = "\u2656";
  m_pieces[wq] = 8ull;
  m_symbols[wq] = "\u2655";
  m_pieces[wk] = 16ull;
  m_symbols[wk] = "\u2654";
  m_pieces[bp] = 255ull << 48;
  m_symbols[bp] = "\u265F";
  m_pieces[bn] = 66ull << 56;
  m_symbols[bn] = "\u265E";
  m_pieces[bb] = 36ull << 56;
  m_symbols[bb] = "\u265D";
  m_pieces[br] = 129ull << 56;
  m_symbols[br] = "\u265C";
  m_pieces[bq] = 8ull << 56; m_symbols[bq] = "\u265B";
  m_pieces[bk] = 16ull << 56;
  m_symbols[bk] = "\u265A";

  initKnightMoveTable();

  initRookBishopBlockerMasks(true); // rook move masks
  initRookBishopBlockerMasks(false); // bishop move masks

  initRookBishopMoveTable(true);  // rook move table
  initRookBishopMoveTable(false); // bishop move table

  //findRookBishopMagics(true); // rook magics
  //findRookBishopMagics(false); // bishop magics

}

// generate knight move lookup table
void Board::initKnightMoveTable() {
  // knight moves
  int dx[] = {2, 2, -2, -2, 1, 1, -1, -1};
  int dy[] = {1, -1, 1, -1, 2, -2, 2, -2};
  // for each square
  for(int i=0; i<64; ++i) {
    m_knightMoves[i] = 0;
    for(int j=0; j<8; ++j) {
      // add (dx,dy) to position then check if valid
      int x = (int)(i&7) + dx[j];
      int y = (int)(i>>3) + dy[j];
      if(0<=x && 0 <= y && x < 64 && y < 64) m_knightMoves[i] |= 1ull<<(8*y+x);
    }
  }
}

// generate blocker masks, for rooks or bishops
void Board::initRookBishopBlockerMasks(bool isRook) {
  // for each square
  for(int i=0; i<64; ++i) {
    if(isRook) m_rookMasks[i] = 0;
    else m_bishopMasks[i] = 0;

    int rookXChanges[] = {1, -1, 0, 0};
    int rookYChanges[] = {0, 0, 1, -1};
    int bishopXChanges[] = {1, 1, -1, -1};
    int bishopYChanges[] = {1, -1, 1, -1};
    for(int k=0; k<4; ++k) {
      // move in direction specified by k, until arriving at the board edge
      int x = i&7;
      int y = i>>3;
      int dx = (isRook ? rookXChanges[k] : bishopXChanges[k]);
      int dy = (isRook ? rookYChanges[k] : bishopYChanges[k]);
      while(true) {
        x+=dx;
        y+=dy;
        if(isRook && ( dy==0 && (x<1||x>6) || dx==0 && (y<1||y>6) ) ) break; // if rook, then still possible to be on edge of board with valid moves
        if(!isRook  && (x < 1 || x > 6 || y < 1 || y > 6)) break; // if bishop, then no valid moves once reached edge
        // set the bit at position (x,y)
        int index = y*8 + x;
        if(isRook) m_rookMasks[i] |= (1ull<<index);
        else m_bishopMasks[i] |= (1ull<<index);
      }
    }
  }
}

// given a mask of possible blocker positions, returns the index-th configuration of blockers (sorting lexicographically)
uint64_t Board::getBlockerBoard(uint64_t mask, int index) {
  // turn index into binary then distribute its bits among where the mask indicates the bits should go
  uint64_t blockerBoard = mask;
  int bitIndex = 0;
  for(int sq = 0; sq<64; ++sq) {
    if((mask >> sq) & 1) {
      // set the sq-th bit of the board to (bitIndex-th bit of j)
      uint64_t val = (index>>bitIndex)&1; // val = bitIndex-th bit of j
      blockerBoard ^= (1ull << sq); // clear the sq-th bit
      blockerBoard |= (val<<sq); // or the sq-th bit with val, so that sq-th bit = val
      bitIndex++;
    }
  }
  return blockerBoard;
}

// given an arrangement of blockers, calculate the set of (pseudo-)legal moves
uint64_t Board::getRookBishopMoveBoard(bool isRook, uint64_t blockerBoard, int square) {
  uint64_t moveBoard = blockerBoard;
  int rookXChanges[] = {1, -1, 0, 0};
  int rookYChanges[] = {0, 0, 1, -1};
  int bishopXChanges[] = {1, 1, -1, -1};
  int bishopYChanges[] = {1, -1, 1, -1};
  for(int k=0; k<4; ++k) {
    // move in direction specified by k, until the first blocker is met
    int x = square&7;
    int y = square>>3;
    int dx = (isRook ? rookXChanges[k] : bishopXChanges[k]);
    int dy = (isRook ? rookYChanges[k] : bishopYChanges[k]);
    bool foundBlocker = false;
    while(true) {
      x+=dx;
      y+=dy;
      if(x < 0 || x > 7 || y < 0 || y > 7) break;
      int index = y*8 + x;
      if(foundBlocker) moveBoard &= ~(1ull<<index); // wipe the bit
      else moveBoard |= (1ull<<index); // set the bit
      if((blockerBoard >> index)&1) foundBlocker = true;
    }
  }
  return moveBoard;
}

int bitCount(uint64_t n) {
  int t=0;
  while(n) {
    t += n&1;
    n >>= 1;
  }
  return t;
}

void Board::findRookBishopMagics(bool isRook) {
  // number of bits needed to store all possible blocker configurations, which equals the max number of potential blockers
  int bits = (isRook ? 12 : 9); // a rook has at most 12 potential blockers, bishop has at most 9
  int size = 1<<bits; // at most 2^bits possible blocker configurations: i.e. 4096 for rook, 512 for bishop

  // generate all possible blocker configurations
  for(int i=0; i<64; ++i) {
    // try magic numbers until one works i.e. hashes with no collisions
    while(true) {
      // generate a random 64-bit trial magic number with a low density of 1s
      // note: assumes std::rand is 32-bit
      uint64_t random1 = ((long long)rand() << 32) | rand();
      uint64_t random2 = ((long long)rand() << 32) | rand();
      uint64_t random3 = ((long long)rand() << 32) | rand();
      uint64_t trialNum = random1 & random2 & random3;
      if(bitCount(trialNum)>6) continue;

      bool fail = false;
      uint64_t moveTable[size];
      bool used[size];
      for(int j=0; j<size; ++j) {
        used[j] = false;
      }
      for(int j=0; j<size; ++j) {
        uint64_t blockerBoard = getBlockerBoard(isRook ? m_rookMasks[i] : m_bishopMasks[i], j);
        uint64_t index = (trialNum * blockerBoard) >> (64 - bits); // use first 12 (if rook, 9 if bishop) bits as magic index
        uint64_t moveBoard = getRookBishopMoveBoard(isRook, blockerBoard, i);
        // if two blocker boards generate the same magic index but different move boards, then fail
        if(used[index] && moveBoard != moveTable[index]) {
          fail = true;
          break;
        } else {
          used[index] = true;
          moveTable[index] = moveBoard;
        }
      }
      if(!fail) {
        std::cout << trialNum << "\n";
        break;
      }
    }

  }  
}

void Board::initRookBishopMoveTable(bool isRook) {
  // number of bits needed to store all possible blocker configurations, which equals the max number of potential blockers
  int bits = (isRook ? 12 : 9); // a rook has at most 12 potential blockers, bishop has at most 9
  int size = 1<<bits; // at most 2^bits possible blocker configurations: i.e. 4096 for rook, 512 for bishop
  
  // for each square
  for(int i=0; i<64; ++i) {
    uint64_t magicNum = isRook ? m_rookMagics[i] : m_bishopMagics[i];
    // for every blocker configuration
    for(int j=0; j<size; ++j) {
      uint64_t blockerBoard = getBlockerBoard(isRook ? m_rookMasks[i] : m_bishopMasks[i], j);

      uint64_t index = magicNum * blockerBoard >> (64 - bits); // magic index
      uint64_t moveBoard = getRookBishopMoveBoard(isRook, blockerBoard, i);
      // write to the table
      if(isRook) m_rookMoves[i][index] = moveBoard;
      else m_bishopMoves[i][index] = moveBoard;
    }
  }
}

uint64_t Board::lookupKnightMoves(int square) {
  return m_knightMoves[square];
}

uint64_t Board::lookupRookMoves(int square, uint64_t occupancyBoard) {
  uint64_t blockerBoard = occupancyBoard & m_rookMasks[square];
  uint64_t index = m_rookMagics[square] * blockerBoard >> (64 - 12);
  return m_rookMoves[square][index];
}

uint64_t Board::lookupBishopMoves(int square, uint64_t occupancyBoard) {
  uint64_t blockerBoard = occupancyBoard & m_bishopMasks[square];
  uint64_t index = m_bishopMagics[square] * blockerBoard >> (64 - 9);
  return m_bishopMoves[square][index];
}

void Board::makeMove(int start, int end) {
  // find which bitboard (if any) contains a piece to be taken
  for(int i=0; i<12; ++i) {
    if((m_pieces[i] >> end) & 1) {
      // once found, delete it
      m_pieces[i] ^= 1ull<<end;
      break;
    }
  }
  // find which bitboard (if any) contains the piece to move
  for(int i=0; i<12; ++i) {
    if((m_pieces[i] >> start) & 1) {
      // once found, move it
      m_pieces[i] ^= 1ull<<start;
      m_pieces[i] |= 1ull<<end;
      break;
    }
  }
}

void Board::displayBoard() {
  std::string symbols[64];
  // init symbols
  for(int i=0; i<64; ++i) symbols[i] = " ";

  // draw each piece type
  for(int i=0; i<12; ++i) {
    // loop through each bit
    for(int j=0; j<64; ++j) {
      // if current bit is 1, then place piece symbol at corresponding position
      if( (m_pieces[i] >> j) & 1 ) {
        symbols[j] = m_symbols[i];
      }
    }
  }

  // output to screen
  display8x8(symbols);
}

void Board::display8x8(uint64_t bitboard) {
  std::string stringSymbols[64];
  for(int i=0; i<64; ++i) {
    uint64_t bit = (bitboard>>i) & 1;
    stringSymbols[i] = bit ? std::to_string(bit) : " ";
  }
  display8x8(stringSymbols);
}

void Board::display8x8(std::string symbols[64]) {

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

#pragma once

#include "Bitboard.h"
#include <string>

class Board {

  public:
    Board();
    void printBoard();

  private:
    
    Bitboard m_pieceBBs[12];
    std::string m_symbols[12];

    // pawns, knights, bishops, rooks, queens, king; white, black
    enum PieceType {
      wp, wn, wb, wr, wq, wk,
      bp, bn, bb, br, bq, bk,
    };

};

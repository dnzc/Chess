#pragma once

#include "Move.h"
#include "Bitboard.h"
#include <string>
#include <cstdint>
#include <vector>
enum PieceType {
  wp, wn, wb, wr, wq, wk,
  bp, bn, bb, br, bq, bk,
  empty
};

class Position {

  public:

    Position();

    void makeMove(Move move);

    Bitboard getWhiteOccupancy();
    Bitboard getBlackOccupancy();
    Bitboard getPieces(PieceType pt);
    PieceType whichPiece(int square);
    bool isWhiteToMove();
    bool canWhiteCastleKingside();
    bool canWhiteCastleQueenside();
    bool canBlackCastleKingside();
    bool canBlackCastleQueenside();
    Bitboard getEnPassant();

    void removePieces(PieceType pt, Bitboard bb);

  private:
    // array of which piece is on each square, so that "what piece is on this square?"
    // can be answered quickly (slow with bitboards)
    PieceType m_board[64];

    bool m_whiteToMove;
    // castling rights
    bool m_whiteCastleKingside;
    bool m_whiteCastleQueenside;
    bool m_blackCastleKingside;
    bool m_blackCastleQueenside;

    // bitboards for each (piece type, colour) pair
    Bitboard m_pieces[12];

    // squares that could be affected by en passant next move
    Bitboard m_enPassant;

    // bitboards for squares containing white/black pieces
    Bitboard m_whiteOccupancy;
    Bitboard m_blackOccupancy;

};

#pragma once

enum PieceType {
  wp, wn, wb, wr, wq, wk,
  bp, bn, bb, br, bq, bk,
  empty
};

// GROUP B SKILL: simple OOP
struct Move {
    Move(int start, int end, int piece, int castle, int promotion, bool enPassant)
      : start(start), end(end), piece(piece), castle(castle), promotion(promotion), enPassant(enPassant) {};
    int start;
    int end;
    int piece;
    int castle; // 0 for no, 1 for white kingside castle, 2 for white queenside, etc
    int promotion; // 0 (false) if not a pawn promotion move, else it is one of (wn,wb,wr,wq,wk, bn,bb,br,bq,bk)
    bool enPassant;
};

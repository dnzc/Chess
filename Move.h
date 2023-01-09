#pragma once

enum PieceType {
  wp, wn, wb, wr, wq, wk,
  bp, bn, bb, br, bq, bk,
  empty
};

struct Move {
    Move(int start, int end, int piece, bool castle, int promotion, bool enPassant)
      : start(start), end(end), piece(piece), castle(castle), promotion(promotion), enPassant(enPassant) {};
    int start;
    int end;
    int piece;
    bool castle;
    int promotion; // 0 (false) if not a pawn promotion move, else it is one of (wn,wb,wr,wq,wk, bn,bb,br,bq,bk)
    bool enPassant;
};
